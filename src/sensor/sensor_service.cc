/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "sensor_service.h"

#include <string>
#include <memory>
#include <mutex>

#include "common/task-queue.h"
#include "common/logger.h"
#include "common/optional.h"
#include "common/platform_exception.h"
#include "sensor_instance.h"

using namespace common;
using namespace common::tools;

namespace extension {
namespace sensor {

namespace {
#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

static std::map<sensor_type_e, std::string> type_to_string_map;
static std::map<std::string, sensor_type_e> string_to_type_map;

static std::string GetAccuracyString(int accuracy) {
  LoggerD("Entered");
  switch (static_cast<sensor_data_accuracy_e>(accuracy)) {
    case SENSOR_DATA_ACCURACY_BAD:
      return "ACCURACY_BAD";
    case SENSOR_DATA_ACCURACY_NORMAL:
      return "ACCURACY_NORMAL";
    case SENSOR_DATA_ACCURACY_GOOD:
      return "ACCURACY_GOOD";
    case SENSOR_DATA_ACCURACY_VERYGOOD:
      return "ACCURACY_VERYGOOD";
    default:
      return "ACCURACY_UNDEFINED";
  }
}

static const std::string kSensorTypeTag = "sensorType";
static const std::string kListenerId = "listenerId";
static const std::string kSensorChangedListener = "SensorChangedListener";

void ReportSensorData(sensor_type_e sensor_type, sensor_event_s* sensor_event,
                      picojson::object* out) {
  LoggerD("Entered");

  switch (sensor_type) {
    case SENSOR_LIGHT: {
      (*out)["lightLevel"] = picojson::value(static_cast<double>(sensor_event->values[0]));
      break;
    }
    case SENSOR_MAGNETIC: {
      (*out)["x"] = picojson::value(static_cast<double>(sensor_event->values[0]));
      (*out)["y"] = picojson::value(static_cast<double>(sensor_event->values[1]));
      (*out)["z"] = picojson::value(static_cast<double>(sensor_event->values[2]));
      (*out)["accuracy"] = picojson::value(GetAccuracyString(sensor_event->accuracy));
      break;
    }
    case SENSOR_PRESSURE: {
      (*out)["pressure"] = picojson::value(static_cast<double>(sensor_event->values[0]));
      break;
    }
    case SENSOR_PROXIMITY: {
      const int state = static_cast<int>(sensor_event->values[0]);
      (*out)["proximityState"] = picojson::value(state ? "NEAR" : "FAR");
      break;
    }
    case SENSOR_ULTRAVIOLET: {
      (*out)["ultravioletLevel"] = picojson::value(static_cast<double>(sensor_event->values[0]));
      break;
    }
    case SENSOR_HRM_LED_IR:
    case SENSOR_HRM_LED_RED:
    case SENSOR_HRM_LED_GREEN: {
      (*out)["lightType"] = picojson::value(type_to_string_map[sensor_type]);
      (*out)["lightIntensity"] = picojson::value(static_cast<double>(sensor_event->values[0]));
      break;
    }
    default: {
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unsupported type"), out);
      return;
    }
  }
  (*out)[kListenerId] = picojson::value(kSensorChangedListener);
  (*out)[kSensorTypeTag] = picojson::value(type_to_string_map[sensor_type]);
}

std::string GetSensorErrorMessage(const int error_code) {
  LoggerD("Entered");

  switch (error_code) {
    case SENSOR_ERROR_IO_ERROR:
      return "IO error";
    case SENSOR_ERROR_INVALID_PARAMETER:
      return "Invalid parameter";
    case SENSOR_ERROR_NOT_SUPPORTED:
      return "Not supported";
    case SENSOR_ERROR_PERMISSION_DENIED:
      return "Permission denied";
    case SENSOR_ERROR_OUT_OF_MEMORY:
      return "Out of memory";
    case SENSOR_ERROR_NOT_NEED_CALIBRATION:
      return "Need calibration";
    case SENSOR_ERROR_OPERATION_FAILED:
      return "Operation failed";
    default:
      return "Unknown Error";
  }
}

PlatformResult GetSensorPlatformResult(const int error_code, const std::string &hint) {
  LoggerD("Entered");

  std::string message = hint + " : " + GetSensorErrorMessage(error_code);

  switch (error_code) {
    case SENSOR_ERROR_NOT_SUPPORTED:
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, message);
    default:
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }
}

bool MagneticEventComparator(sensor_event_s* l, sensor_event_s* r) {
  return (l->values[0] == r->values[0] &&
          l->values[1] == r->values[1] &&
          l->values[2] == r->values[2] &&
          l->accuracy == r->accuracy);
}

} // namespace

class SensorData {
 public:
  typedef bool (*EventComparator)(sensor_event_s* l, sensor_event_s* r);

  SensorData(SensorInstance& instance, sensor_type_e type_enum,
             const std::string& name, EventComparator comparator =
                 DefaultEventComparator);
  virtual ~SensorData();

  PlatformResult IsSupported(bool* supported);
  virtual PlatformResult Start();
  virtual PlatformResult Stop();
  virtual PlatformResult SetChangeListener();
  virtual PlatformResult UnsetChangeListener();
  virtual PlatformResult GetSensorData(picojson::object* data);

  sensor_type_e type() const { return type_enum_; }
  bool is_supported();

  bool UpdateEvent(sensor_event_s* event);

 protected:
  virtual PlatformResult CheckInitialization();
  virtual PlatformResult IsSupportedImpl(bool* supported) const;

 private:
  static void SensorCallback(sensor_h sensor, sensor_event_s* event, void* user_data);
  static bool DefaultEventComparator(sensor_event_s* l, sensor_event_s* r);

  sensor_type_e type_enum_;
  EventComparator comparator_;
  sensor_h handle_;
  sensor_listener_h listener_;
  sensor_event_s previous_event_;
  common::optional<bool> is_supported_;
  SensorInstance& instance_;
};

SensorData::SensorData(SensorInstance& instance, sensor_type_e type_enum,
                       const std::string& name, EventComparator comparator)
    : type_enum_(type_enum),
      comparator_(comparator),
      handle_(nullptr),
      listener_(nullptr),
      previous_event_(),
      instance_(instance) {
  type_to_string_map.insert(std::make_pair(type_enum, name));
  string_to_type_map.insert(std::make_pair(name, type_enum));

  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
}

SensorData::~SensorData() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  if (listener_) {
    sensor_destroy_listener(listener_);
  }
}

void SensorData::SensorCallback(sensor_h sensor, sensor_event_s* event, void* user_data) {
  LoggerD("Entered");

  SensorData* that = static_cast<SensorData*>(user_data);

  if (!that) {
    LoggerE("user_data is null");
    return;
  }

  LoggerD("Entered: %s", type_to_string_map[that->type()].c_str());

  if (!that->UpdateEvent(event)) {
    // value didn't change - ignore
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& object = result.get<picojson::object>();
  ReportSensorData(that->type(), event, &object);
  Instance::PostMessage(&that->instance_, result.serialize().c_str());
}

bool SensorData::DefaultEventComparator(sensor_event_s* l, sensor_event_s* r) {
  return (l->values[0] == r->values[0]);
}

PlatformResult SensorData::CheckInitialization() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  if (!handle_) {
    LoggerD("initialization of handle and listener");
    int ret = sensor_get_default_sensor(type_enum_, &handle_);
    if (SENSOR_ERROR_NONE != ret) {
      LoggerE("sensor_get_default_sensor : %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "sensor_get_default_sensor");
    }

    ret = sensor_create_listener(handle_, &listener_);
    if (SENSOR_ERROR_NONE != ret) {
      LoggerE("sensor_create_listener : %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "sensor_create_listener");
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SensorData::IsSupported(bool* supported) {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  if (!is_supported_) {
    bool is_supported = false;
    auto res = IsSupportedImpl(&is_supported);
    if (!res) {
      return res;
    } else {
      is_supported_ = is_supported;
    }
  }

  *supported = *is_supported_;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SensorData::IsSupportedImpl(bool* supported) const {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  bool is_supported = false;
  int ret = sensor_is_supported(type_enum_, &is_supported);
  if (SENSOR_ERROR_NONE != ret) {
    LoggerE("Failed to check if sensor %s is supported", type_to_string_map[type_enum_].c_str());
    return GetSensorPlatformResult(ret, "sensor_is_supported");
  } else {
    *supported = is_supported;
    LoggerD("supported: %d", is_supported);
    return PlatformResult(ErrorCode::NO_ERROR);
  }
}

bool SensorData::is_supported() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  if (!is_supported_) {
    bool is_supported = false;
    auto res = IsSupportedImpl(&is_supported);
    if (!res) {
      is_supported_ = false;
    } else {
      is_supported_ = is_supported;
    }
  }

  return *is_supported_;
}

bool SensorData::UpdateEvent(sensor_event_s* event) {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  if (comparator_(&previous_event_, event)) {
    // previous and current events are the same -> no update
    return false;
  } else {
    previous_event_ = *event;
    return true;
  }
}

PlatformResult SensorData::Start() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  auto res = CheckInitialization();

  if (!res) {
    LoggerE("Sensor initialization for sensor %s failed", type_to_string_map[type_enum_].c_str());
    return res;
  }

  sensor_listener_set_option(listener_, SENSOR_OPTION_ALWAYS_ON);
  int ret = sensor_listener_start(listener_);
  if (SENSOR_ERROR_NONE != ret) {
    LoggerE("sensor_listener_start : %d", ret);
    return GetSensorPlatformResult(ret, "sensor_listener_start");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SensorData::Stop() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  auto res = CheckInitialization();

  if (!res) {
    LoggerE("Sensor initialization for sensor %s failed", type_to_string_map[type_enum_].c_str());
    return res;
  }

  int ret = sensor_listener_stop(listener_);
  if (SENSOR_ERROR_NONE != ret) {
    LoggerE("sensor_listener_stop : %d", ret);
    return GetSensorPlatformResult(ret, "sensor_listener_stop");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SensorData::SetChangeListener() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  auto res = CheckInitialization();

  if (!res) {
    LoggerE("Sensor initialization for sensor %s failed", type_to_string_map[type_enum_].c_str());
    return res;
  }

  int ret = sensor_listener_set_event_cb(listener_, 100, SensorCallback, this);
  if (SENSOR_ERROR_NONE != ret) {
    LoggerE("sensor_listener_set_event_cb : %d", ret);
    return GetSensorPlatformResult(ret, "sensor_listener_set_event_cb");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SensorData::UnsetChangeListener() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  auto res = CheckInitialization();

  if (!res) {
    LoggerE("Sensor initialization for sensor %s failed", type_to_string_map[type_enum_].c_str());
    return res;
  }

  int ret = sensor_listener_unset_event_cb(listener_);
  if (SENSOR_ERROR_NONE != ret) {
    LoggerE("sensor_listener_unset_event_cb : %d", ret);
    return GetSensorPlatformResult(ret, "sensor_listener_unset_event_cb");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SensorData::GetSensorData(picojson::object* data) {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());

  auto res = CheckInitialization();

  if (!res) {
    LoggerE("Sensor initialization for sensor %s failed", type_to_string_map[type_enum_].c_str());
    return res;
  }

  sensor_event_s sensor_event;
  int ret = sensor_listener_read_data(listener_, &sensor_event);
  if (SENSOR_ERROR_NONE != ret) {
    LoggerE("sensor_listener_read_data : %d", ret);
    return GetSensorPlatformResult(ret, "sensor_listener_read_data");
  }

  ReportSensorData(type_enum_, &sensor_event, data);

  return PlatformResult(ErrorCode::NO_ERROR);
}

class HrmSensorData : public SensorData {
 public:
  explicit HrmSensorData(SensorInstance& instance);
  virtual ~HrmSensorData();

  virtual PlatformResult Start();
  virtual PlatformResult Stop();
  virtual PlatformResult SetChangeListener();
  virtual PlatformResult UnsetChangeListener();
  virtual PlatformResult GetSensorData(picojson::object* data);

 private:
  void AddSensor(SensorData* sensor);
  PlatformResult CallMember(PlatformResult (SensorData::*member) ());
  virtual PlatformResult IsSupportedImpl(bool* supported) const;

  std::map<sensor_type_e, std::shared_ptr<SensorData>> hrm_sensors_;
};

HrmSensorData::HrmSensorData(SensorInstance& instance)
    : SensorData(instance, SENSOR_CUSTOM, "HRM_RAW") {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  AddSensor(new SensorData(instance, SENSOR_HRM_LED_IR, "LED_IR"));
  AddSensor(new SensorData(instance, SENSOR_HRM_LED_RED, "LED_RED"));
  AddSensor(new SensorData(instance, SENSOR_HRM_LED_GREEN, "LED_GREEN"));
}

HrmSensorData::~HrmSensorData() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
}

void HrmSensorData::AddSensor(SensorData* sensor) {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  hrm_sensors_.insert(std::make_pair(sensor->type(), std::shared_ptr<SensorData>(sensor)));
}

PlatformResult HrmSensorData::CallMember(PlatformResult (SensorData::*member) ()) {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  for (const auto& sensor : hrm_sensors_) {
    if (sensor.second->is_supported()) {
      auto res = (sensor.second.get()->*member)();
      if (!res) {
        return res;
      }
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HrmSensorData::IsSupportedImpl(bool* supported) const {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  bool result = false;

  bool hrm_supported = false;
  int ret = sensor_is_supported(SENSOR_HRM, &hrm_supported);
  if (ret == SENSOR_ERROR_NONE){
    LoggerD("HRM support is: %d", hrm_supported);
    result |= hrm_supported;
  }

  for (const auto& sensor : hrm_sensors_) {
    bool is_supported = false;
    auto res = sensor.second->IsSupported(&is_supported);
    LoggerD("supported: %d", is_supported);
    if (!res) {
      return res;
    }
    result |= is_supported;
  }
  LoggerD("result supported: %d", result);
  *supported = result;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HrmSensorData::Start() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  return CallMember(&SensorData::Start);
}

PlatformResult HrmSensorData::Stop() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  return CallMember(&SensorData::Stop);
}

PlatformResult HrmSensorData::SetChangeListener() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  return CallMember(&SensorData::SetChangeListener);
}

PlatformResult HrmSensorData::UnsetChangeListener() {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  return CallMember(&SensorData::UnsetChangeListener);
}

PlatformResult HrmSensorData::GetSensorData(picojson::object* data) {
  LoggerD("Entered: %s", type_to_string_map[type()].c_str());
  for (const auto& sensor : hrm_sensors_) {
    if (sensor.second->is_supported()) {
      // HRMRawSensor.getHRMRawSensorData() can return only one value,
      // so we're returning the data from the first available sensor
      return sensor.second->GetSensorData(data);
    }
  }

  // use default values when are no available HRM sensors
  const sensor_type_e default_sensor_type = SENSOR_HRM_LED_IR;
  const double default_sensor_value = 0.0;

  LoggerD("There are no supported HRM sensors - returning default values");
  (*data)["lightType"] = picojson::value(type_to_string_map[default_sensor_type]);
  (*data)["lightIntensity"] = picojson::value(default_sensor_value);
  (*data)[kListenerId] = picojson::value(kSensorChangedListener);
  (*data)[kSensorTypeTag] = picojson::value(type_to_string_map[default_sensor_type]);
  return PlatformResult(ErrorCode::NO_ERROR);
}

SensorService::SensorService(SensorInstance& instance)
    : instance_(instance) {
  LoggerD("Entered");

  AddSensor(new SensorData(instance, SENSOR_LIGHT, "LIGHT"));
  AddSensor(new SensorData(instance, SENSOR_MAGNETIC, "MAGNETIC", MagneticEventComparator));
  AddSensor(new SensorData(instance, SENSOR_PRESSURE, "PRESSURE"));
  AddSensor(new SensorData(instance, SENSOR_PROXIMITY, "PROXIMITY"));
  AddSensor(new SensorData(instance, SENSOR_ULTRAVIOLET, "ULTRAVIOLET"));
  AddSensor(new HrmSensorData(instance));
}

SensorService::~SensorService() {
  LoggerD("Entered");
}

void SensorService::GetAvailableSensors(picojson::object& out) {
  LoggerD("Entered");

  bool is_supported = false;

  picojson::value result = picojson::value(picojson::array());
  picojson::array& result_array = result.get<picojson::array>();

  for (const auto& sensor : sensors_) {
    auto res = sensor.second->IsSupported(&is_supported);
    if (!res) {
      LoggerE("Failed to check if sensor is supported: %s", type_to_string_map[sensor.first].c_str());
      ReportError(res, &out);
      return;
    }

    if (is_supported) {
      result_array.push_back(picojson::value(type_to_string_map[sensor.first]));
    }
  }

  ReportSuccess(result, out);
}

void SensorService::SensorStart(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "callbackId", out)
  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  const std::string type_str =
      args.contains(kSensorTypeTag) ? args.get(kSensorTypeTag).get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  auto start = [this, type_enum, type_str](const std::shared_ptr<picojson::value>& result) {
    auto sensor_data = GetSensor(type_enum);
    if (!sensor_data) {
      LoggerD("Sensor data is null");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Sensor data is null"),
                         &(result->get<picojson::object>()));
      return;
    }

    PlatformResult res = sensor_data->Start();
    if (!res) {
      LoggerE("Failed to start sensor: %s", type_str.c_str());
      ReportError(res, &(result->get<picojson::object>()));
    } else {
      ReportSuccess(result->get<picojson::object>());
    }
  };
  auto start_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{static_cast<double>(callback_id)};
    Instance::PostMessage(&instance_, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      start,
      start_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});
  ReportSuccess(out);
}

void SensorService::SensorStop(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const std::string type_str =
      args.contains(kSensorTypeTag) ? args.get(kSensorTypeTag).get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  auto sensor_data = GetSensor(type_enum);
  if (!sensor_data) {
    LoggerD("Sensor data is null");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Sensor data is null"), &out);
    return;
  }

  PlatformResult res = sensor_data->Stop();
  if (!res) {
    LoggerE("Failed to stop sensor: %s", type_str.c_str());
    ReportError(res, &out);
  } else {
    ReportSuccess(out);
  }
}

void SensorService::SensorSetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const std::string type_str =
      args.contains(kSensorTypeTag) ? args.get(kSensorTypeTag).get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  auto sensor_data = GetSensor(type_enum);
  if (!sensor_data) {
    LoggerD("Sensor data is null");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Sensor data is null"), &out);
    return;
  }

  PlatformResult res = sensor_data->SetChangeListener();
  if (!res) {
    LoggerE("Failed to set change listener for sensor: %s", type_str.c_str());
    ReportError(res, &out);
  } else {
    ReportSuccess(out);
  }
}

void SensorService::SensorUnsetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const std::string type_str =
      args.contains(kSensorTypeTag) ? args.get(kSensorTypeTag).get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  auto sensor_data = GetSensor(type_enum);
  if (!sensor_data) {
    LoggerD("Sensor data is null");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Sensor data is null"), &out);
    return;
  }

  PlatformResult res = sensor_data->UnsetChangeListener();
  if (!res) {
    LoggerE("Failed to remove change listener for sensor: %s", type_str.c_str());
    ReportError(res, &out);
  } else {
    ReportSuccess(out);
  }
}

std::shared_ptr<SensorData> SensorService::GetSensor(sensor_type_e type_enum) {
  LoggerD("Entered");
  auto sensor = sensors_.find(type_enum);

  if (sensors_.end() == sensor) {
    return nullptr;
  } else {
    return sensor->second;
  }
}

void SensorService::AddSensor(SensorData* sensor) {
  LoggerD("Entered");
  sensors_.insert(std::make_pair(sensor->type(), std::shared_ptr<SensorData>(sensor)));
}

void SensorService::GetSensorData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  CHECK_EXIST(args, "callbackId", out);
  CHECK_EXIST(args, "type", out);

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  sensor_type_e sensor_type = string_to_type_map[args.get("type").get<std::string>()];

  auto get_data = [this, sensor_type](const std::shared_ptr<picojson::value>& result) {
    picojson::object& object = result->get<picojson::object>();
    auto sensor_data = this->GetSensor(sensor_type);

    if (!sensor_data) {
      LoggerD("Sensor data is null");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Sensor data is null"), &object);
      return;
    }

    PlatformResult res = sensor_data->GetSensorData(&object);
    if (!res) {
      LoggerE("Failed to read data for sensor: %s", type_to_string_map[sensor_type].c_str());
      ReportError(res, &object);
    } else {
      ReportSuccess(object);
    }
  };

  auto get_data_result = [this, callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{static_cast<double>(callback_id)};

    Instance::PostMessage(&instance_, result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      get_data,
      get_data_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});

  ReportSuccess(out);
}

} // namespace sensor
} // namespace extension
