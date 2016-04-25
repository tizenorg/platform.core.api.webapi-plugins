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

#include "humanactivitymonitor/humanactivitymonitor_manager.h"

#include <activity_recognition.h>
#include <gesture_recognition.h>
#include <location_batch.h>
#include <sensor.h>
#include <sensor_internal.h>
#include <system_info.h>

#include "common/logger.h"
#include "common/optional.h"
#include "common/picojson.h"
#include "common/tools.h"

namespace extension {
namespace humanactivitymonitor {

using common::PlatformResult;
using common::ErrorCode;
using common::tools::ReportError;
using common::tools::ReportSuccess;

namespace {

const std::string kActivityTypePedometer = "PEDOMETER";
const std::string kActivityTypeWristUp = "WRIST_UP";
const std::string kActivityTypeHrm = "HRM";
const std::string kActivityTypeSleepMonitor = "SLEEP_MONITOR";

const std::string kSleepStateAwake = "AWAKE";
const std::string kSleepStateAsleep = "ASLEEP";
const std::string kSleepStateUnknown = "UNKNOWN";

const std::string kCallbackInterval = "callbackInterval";
const std::string kSampleInterval = "sampleInterval";

const std::string kStatus = "status";
const std::string kTimestamp = "timestamp";

const std::string kStepStatus = "stepStatus";
const std::string kSpeed = "speed";
const std::string kWalkingFrequency = "walkingFrequency";
const std::string kCumulativeDistance = "cumulativeDistance";
const std::string kCumulativeCalorie = "cumulativeCalorie";
const std::string kCumulativeTotalStepCount = "cumulativeTotalStepCount";
const std::string kCumulativeWalkStepCount = "cumulativeWalkStepCount";
const std::string kCumulativeRunStepCount = "cumulativeRunStepCount";
const std::string kStepCountDifferences = "stepCountDifferences";
const std::string kStepCountDifference = "stepCountDifference";

const std::string kAccumulativeDistance = "accumulativeDistance";
const std::string kAccumulativeCalorie = "accumulativeCalorie";
const std::string kAccumulativeTotalStepCount = "accumulativeTotalStepCount";
const std::string kAccumulativeWalkStepCount = "accumulativeWalkStepCount";
const std::string kAccumulativeRunStepCount = "accumulativeRunStepCount";

// helper structure, allows easier access to data values
struct PedometerDataWrapper : public sensor_pedometer_data_t {
  inline float steps() const {
    return values[0];
  }

  inline float walk_steps() const {
    return values[1];
  }

  inline float run_steps() const {
    return values[2];
  }

  inline float distance() const {
    return values[3];
  }

  inline float calories() const {
    return values[4];
  }

  inline float speed() const {
    return values[5];
  }

  inline float frequency() const {
    return values[6];
  }

  inline sensor_pedometer_state_e state() const {
    return static_cast<sensor_pedometer_state_e>(values[7]);
  }
};

static int64_t getCurrentTimeStamp(unsigned long long evTime)
{
  LoggerD("Enter");
  struct timespec t;
  unsigned long long systemCurrentTime = 0;
  unsigned long long realCurrentTime = 0;
  unsigned long long timeDiff = 0;
  int64_t timeStamp = 0;

  //get current system monotonic  time
  clock_gettime(CLOCK_MONOTONIC, &t);
  systemCurrentTime = ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) /1000000; //convert millisecond
  timeDiff = (systemCurrentTime - (evTime/1000));

  //get current epoch time(millisecond)
  clock_gettime(CLOCK_REALTIME, &t);
  realCurrentTime = ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) /1000000;
  timeStamp =static_cast<int64_t>(realCurrentTime -timeDiff);

  return timeStamp;
}

void InsertStepDifference(float step_difference, float timestamp, picojson::array* out) {
  ScopeLogger();

  picojson::object d;

  d.insert(std::make_pair(kStepCountDifference, picojson::value(step_difference)));
  d.insert(std::make_pair(kTimestamp, picojson::value(timestamp)));

  out->push_back(picojson::value{d});
}

std::string FromSensorPedometerState(sensor_pedometer_state_e e) {
  switch (e) {
    case SENSOR_PEDOMETER_STATE_STOP:
      return "NOT_MOVING";

    case SENSOR_PEDOMETER_STATE_WALK:
      return "WALKING";

    case SENSOR_PEDOMETER_STATE_RUN:
      return "RUNNING";

    default:
      return "UNKNOWN";
  }
}

}  // namespace

const std::string kActivityTypeGps = "GPS";

class HumanActivityMonitorManager::Monitor {
 public:
  class GestureMonitor;
  class SensorMonitor;
  class GpsMonitor;

  explicit Monitor(const std::string& t) : type_(t) {
    ScopeLogger(type());
  }

  virtual ~Monitor() {
    ScopeLogger(type());
  }

  std::string type() const {
    return type_;
  }

  JsonCallback& event_callback() {
    ScopeLogger(type());
    return event_callback_;
  }

  PlatformResult SetListener(JsonCallback callback, const picojson::value& args) {
    ScopeLogger(type());

    auto result = IsSupported();
    if (!result) {
      return result;
    }

    result = SetListenerImpl(args);
    if (!result) {
      return result;
    }

    event_callback_ = callback;
    return result;
  }

  PlatformResult UnsetListener() {
    ScopeLogger(type());

    auto result = IsSupported();
    if (!result) {
      return result;
    }

    result = UnsetListenerImpl();
    if (!result) {
      return result;
    }

    event_callback_ = nullptr;
    return result;
  }

  PlatformResult GetData(picojson::value* data) {
    ScopeLogger(type());

    auto result = IsSupported();
    if (!result) {
      return result;
    }

    return GetDataImpl(data);
  }

 protected:
  virtual PlatformResult IsSupportedImpl(bool* supported) const {
    ScopeLogger(type());
    *supported = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult SetListenerImpl(const picojson::value& args) {
    ScopeLogger(type());
    return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR);
  }

  virtual PlatformResult UnsetListenerImpl() {
    ScopeLogger(type());
    return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR);
  }

  virtual PlatformResult GetDataImpl(picojson::value* data) const {
    ScopeLogger(type());
    return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR);
  }

 private:
  PlatformResult IsSupported() {
    ScopeLogger(type());

    if (!is_supported_) {
      bool is_supported = false;
      auto res = IsSupportedImpl(&is_supported);
      if (!res) {
        return res;
      } else {
        is_supported_ = is_supported;
      }
    }

    if (*is_supported_) {
      return PlatformResult(ErrorCode::NO_ERROR);
    } else {
      return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR);
    }
  }

  std::string type_;
  common::optional<bool> is_supported_;
  JsonCallback event_callback_;
};

class HumanActivityMonitorManager::Monitor::GestureMonitor : public HumanActivityMonitorManager::Monitor {
 public:
  explicit GestureMonitor(const std::string& t) : Monitor(t), handle_(nullptr) {
    ScopeLogger(type());
  }

  virtual ~GestureMonitor() override {
    ScopeLogger(type());
    UnsetListenerImpl();
  }

 protected:
  virtual PlatformResult IsSupportedImpl(bool* s) const override {
    ScopeLogger(type());

    bool supported = false;

    int ret = gesture_is_supported(GESTURE_WRIST_UP, &supported);
    if (ret != SENSOR_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "WRIST_UP gesture check failed",
                                ("gesture_is_supported(GESTURE_WRIST_UP), error: %d (%s)", ret, get_error_message(ret)));
    }

    *s = supported;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult SetListenerImpl(const picojson::value&) override {
    ScopeLogger(type());

    if (!handle_) {
      int ret = gesture_create(&handle_);
      if (GESTURE_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to create WRIST_UP listener",
                                  ("Failed to create WRIST_UP handle, error: %d (%s)", ret, get_error_message(ret)));
      }

      ret = gesture_start_recognition(handle_,
                                      GESTURE_WRIST_UP,
                                      GESTURE_OPTION_DEFAULT,
                                      OnWristUpEvent,
                                      this);
      if (GESTURE_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to start WRIST_UP listener",
                                  ("Failed to start WRIST_UP listener, error: %d (%s)", ret, get_error_message(ret)));
      }
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult UnsetListenerImpl() override {
    ScopeLogger(type());

    if (handle_) {
      int ret = gesture_stop_recognition(handle_);
      if (GESTURE_ERROR_NONE != ret) {
        LOGGER(ERROR) << "Failed to stop WRIST_UP detection, error: " << ret;
      }

      ret = gesture_release(handle_);
      if (GESTURE_ERROR_NONE != ret) {
        LOGGER(ERROR) << "Failed to release WRIST_UP handle, error: " << ret;
      }

      handle_ = nullptr;
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  // GetData is not supported by gesture monitor

 private:
  static void OnWristUpEvent(gesture_type_e gesture,
                             const gesture_data_h data,
                             double timestamp,
                             gesture_error_e error,
                             void* user_data) {
    ScopeLogger();

    auto monitor = static_cast<GestureMonitor*>(user_data);
    auto& callback = monitor->event_callback();

    if (!callback) {
      LOGGER(ERROR) << "No WRIST_UP event callback registered, skipping.";
      return;
    }

    picojson::value v = picojson::value();  // null value
    callback(&v);
  }

  gesture_h handle_;
};

class HumanActivityMonitorManager::Monitor::SensorMonitor : public HumanActivityMonitorManager::Monitor {
 public:
  using SensorEventConverter = std::function<PlatformResult(sensor_event_s* event, picojson::object* o)>;

  SensorMonitor(const std::string& t, sensor_type_e s, const SensorEventConverter& c) : Monitor(t), sensor_(s), handle_(nullptr), converter_(c) {
    ScopeLogger(type());
  }

  virtual ~SensorMonitor() override {
    ScopeLogger(type());
    UnsetListenerImpl();
  }

 protected:
  virtual PlatformResult IsSupportedImpl(bool* s) const override {
    ScopeLogger(type());

    bool supported = false;

    int ret = sensor_is_supported(sensor_, &supported);
    if (SENSOR_ERROR_NONE != ret) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Sensor support check failed",
                                ("sensor_is_supported(%d), error: %d (%s)", sensor_, ret, get_error_message(ret)));
    }

    *s = supported;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult SetListenerImpl(const picojson::value& args) override {
    ScopeLogger(type());

    if (!handle_) {
      sensor_h sensor_handle = nullptr;
      int ret = sensor_get_default_sensor(sensor_, &sensor_handle);

      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to get default sensor",
                                  ("Failed to get (%d) sensor, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      ret = sensor_create_listener(sensor_handle, &handle_);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to create sensor listener",
                                  ("Failed to create (%d) sensor listener, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      ret = sensor_listener_set_option(handle_, SENSOR_OPTION_ALWAYS_ON);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to set sensor listener option",
                                  ("Failed to set (%d) sensor listener option, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      int interval = 0;
      auto& js_interval = args.get(kCallbackInterval);

      if (js_interval.is<double>()) {
        interval = js_interval.get<double>();
        LoggerD("callbackInterval: %d", interval);
      }

      ret = sensor_listener_set_event_cb(handle_,
                                         interval,
                                         OnSensorEvent,
                                         this);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to set sensor listener",
                                  ("Failed to set (%d) sensor listener, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      ret = sensor_listener_start(handle_);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to start sensor listener",
                                  ("Failed to start (%d) sensor listener, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult UnsetListenerImpl() override {
    ScopeLogger(type());

    if (handle_) {
      int ret = sensor_listener_stop(handle_);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to stop sensor listener",
                                  ("Failed to stop (%d) sensor listener, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      ret = sensor_listener_unset_event_cb(handle_);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to unset sensor listener",
                                  ("Failed to unset (%d) sensor listener, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      ret = sensor_destroy_listener(handle_);
      if (SENSOR_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to destroy sensor listener",
                                  ("Failed to destroy (%d) sensor listener, error: %d (%s)", sensor_, ret, get_error_message(ret)));
      }

      handle_ = nullptr;
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult GetDataImpl(picojson::value* data) const override {
    ScopeLogger(type());

    if (!handle_) {
      return LogAndCreateResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
    }

    sensor_event_s event = {0};
    int ret = sensor_listener_read_data(handle_, &event);

    if (SENSOR_ERROR_NONE != ret) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Failed to get sensor data",
                                ("Failed to get (%d) sensor data, error: %d (%s)", sensor_, ret, get_error_message(ret)));
    }

    *data = picojson::value(picojson::object());
    auto result = converter_(&event, &data->get<picojson::object>());
    if (!result) {
      return result;
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

 private:
  static void OnSensorEvent(sensor_h, sensor_event_s* event, void* user_data) {
    ScopeLogger();

    auto monitor = static_cast<SensorMonitor*>(user_data);
    auto& callback = monitor->event_callback();

    if (!callback) {
      LOGGER(ERROR) << "No sensor event callback registered, skipping.";
      return;
    }

    picojson::value sensor_data{picojson::object{}};

    auto result = monitor->converter_(event, &sensor_data.get<picojson::object>());
    if (!result) {
      LOGGER(ERROR) << "Failed to convert sensor data: " << result.message();
      return;
    }

    callback(&sensor_data);
  }

  sensor_type_e sensor_;
  sensor_listener_h handle_;
  SensorEventConverter converter_;
};

class HumanActivityMonitorManager::Monitor::GpsMonitor : public HumanActivityMonitorManager::Monitor {
 public:
  explicit GpsMonitor(const std::string& t) : Monitor(t), handle_(nullptr) {
    ScopeLogger(type());
  }

  virtual ~GpsMonitor() override {
    ScopeLogger(type());
    UnsetListenerImpl();
  }

 protected:
  virtual PlatformResult IsSupportedImpl(bool* s) const override {
    ScopeLogger(type());

    int ret = 0;
    ret = system_info_get_platform_bool("http://tizen.org/feature/location.batch", s);

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult SetListenerImpl(const picojson::value& args) override {
    ScopeLogger(type());

    int ret = 0;

    if (!handle_) {
      int ret = location_manager_create(LOCATIONS_METHOD_GPS, &handle_);
      if (LOCATIONS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to create location manager",
                                  ("Failed to create location manager, error: %d (%s)", ret, get_error_message(ret)));
      }
    } else {
      ret = location_manager_stop_batch(handle_);
      if (LOCATIONS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to stop location manager",
                                  ("Failed to stop location manager, error: %d (%s)", ret, get_error_message(ret)));
      }


      ret = location_manager_set_setting_changed_cb(LOCATIONS_METHOD_GPS,
                                                   OnGpsSettingEvent,
                                                   this);
      if (LOCATIONS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to set setting listener",
                                  ("Failed to set setting listener, error: %d (%s)", ret, get_error_message(ret)));
      }
    }

    int callback_interval = static_cast<int>(args.get(kCallbackInterval).get<double>() / 1000);
    int sample_interval = static_cast<int>(args.get(kSampleInterval).get<double>() / 1000);
    LoggerD("callbackInterval: %d, sampleInterval: %d", callback_interval, sample_interval);

    ret = location_manager_set_location_batch_cb(handle_,
                                                 OnGpsEvent,
                                                 sample_interval, // batch_interval
                                                 callback_interval, // batch_period
                                                 this);
    if (LOCATIONS_ERROR_NONE != ret) {
      if (LOCATIONS_ERROR_INVALID_PARAMETER == ret) {
        return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR,
                                  "Failed to set location listener",
                                  ("Failed to set location listener, error: %d (%s)", ret, get_error_message(ret)));
      }
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Failed to set location listener",
                                ("Failed to set location listener, error: %d (%s)", ret, get_error_message(ret)));
    }

    ret = location_manager_start_batch(handle_);
    if (LOCATIONS_ERROR_NONE != ret) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Failed to start location manager",
                                ("Failed to start location manager, error: %d (%s)", ret, get_error_message(ret)));
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult UnsetListenerImpl() override {
    ScopeLogger(type());

    if (handle_) {
      int ret = location_manager_stop_batch(handle_);
      if (LOCATIONS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to stop location manager",
                                  ("Failed to stop location manager, error: %d (%s)", ret, get_error_message(ret)));
      }

      ret = location_manager_unset_location_batch_cb(handle_);
      if (LOCATIONS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to unset location listener",
                                  ("Failed to unset location listener, error: %d (%s)", ret, get_error_message(ret)));
      }

      ret = location_manager_destroy(handle_);
      if (LOCATIONS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Failed to destroy location manager",
                                  ("Failed to destroy location manager, error: %d (%s)", ret, get_error_message(ret)));
      }

      handle_ = nullptr;
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

  virtual PlatformResult GetDataImpl(picojson::value* data) const override {
    ScopeLogger(type());

    if (!handle_) {
      return LogAndCreateResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
    }

    double altitude = 0.0, latitude = 0.0, longitude = 0.0, climb = 0.0,
           direction = 0.0, speed = 0.0, horizontal = 0.0, vertical = 0.0;
    location_accuracy_level_e level = LOCATIONS_ACCURACY_NONE;
    time_t timestamp = 0;

    int ret = location_manager_get_location(handle_, &altitude, &latitude,
                                            &longitude, &climb, &direction, &speed,
                                            &level, &horizontal, &vertical,
                                            &timestamp);
    if (LOCATIONS_ERROR_NONE != ret) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Failed to get location",
                                ("Failed to get location, error: %d (%s)", ret, get_error_message(ret)));
    }

    *data = picojson::value(picojson::array());
    ConvertGpsEvent(latitude, longitude, altitude, speed, direction, horizontal,
                    vertical, timestamp, &data->get<picojson::array>());

    return PlatformResult(ErrorCode::NO_ERROR);
  }

 private:
  static void OnGpsSettingEvent(location_method_e method, bool enable, void *user_data) {
    ScopeLogger();

    if (LOCATIONS_METHOD_GPS != method) {
      LoggerD("Location method different from GPS");
      return;
    }

    auto monitor = static_cast<GpsMonitor*>(user_data);
    auto& callback = monitor->event_callback();

    if (!callback) {
      LoggerE("No GPS event callback registered, skipping.");
      return;
    }

    if (!enable) {
      picojson::value val{picojson::object{}};
      auto& obj = val.get<picojson::object>();

      LogAndReportError(
          PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "GPS service is not available"),
          &obj, ("GPS service is not available"));

      callback(&val);
    }
  }

  static void OnGpsEvent(int num_of_location, void* user_data) {
    ScopeLogger();

    auto monitor = static_cast<GpsMonitor*>(user_data);
    auto& callback = monitor->event_callback();

    if (!callback) {
      LOGGER(ERROR) << "No GPS event callback registered, skipping.";
      return;
    }

    if (0 == num_of_location) {
      LOGGER(ERROR) << "No GPS locations available, skipping.";
      return;
    }

    picojson::value gps_info{picojson::array{}};
    int ret = location_manager_foreach_location_batch(monitor->handle_,
                                                      ConvertGpsEvent,
                                                      &gps_info.get<picojson::array>());
    if (LOCATIONS_ERROR_NONE != ret) {
      LOGGER(ERROR) << "Failed to convert location, error: " << ret;
      return;
    }

    callback(&gps_info);
  }

  static bool ConvertGpsEvent(double latitude, double longitude, double altitude,
                              double speed, double direction, double horizontal,
                              double vertical, time_t timestamp,
                              void* user_data) {
    ScopeLogger();

    auto gps_info_array = static_cast<picojson::array*>(user_data);

    picojson::value gps_info{picojson::object{}};
    auto& gps_info_o = gps_info.get<picojson::object>();

    gps_info_o["latitude"] = picojson::value(latitude);
    gps_info_o["longitude"] = picojson::value(longitude);
    gps_info_o["altitude"] = picojson::value(altitude);
    gps_info_o["speed"] = picojson::value(speed);
    // TODO(r.galka): errorRange not available in CAPI
    gps_info_o["errorRange"] = picojson::value(static_cast<double>(0));
    gps_info_o[kTimestamp] = picojson::value(static_cast<double>(timestamp));

    gps_info_array->push_back(gps_info);

    return true;
  }

  location_manager_h handle_;
};

class HumanActivityMonitorManager::ActivityRecognition {
 public:
  PlatformResult AddListener(const std::string& type, JsonCallback callback, long* watch_id) {
    ScopeLogger();

    activity_type_e activity_type = ToActivityType(type);
    auto result = IsSupported(activity_type);

    if (!result) {
      return result;
    }

    activity_h handle = nullptr;
    int ret = activity_create(&handle);
    if (ACTIVITY_ERROR_NONE != ret) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Failed to create activity",
                                ("activity_create() error: %d - %s", ret, get_error_message(ret)));
    }

    const auto id = GetNextId();
    auto data = std::make_shared<ActivityData>(id, callback, handle);

    ret = activity_start_recognition(handle, activity_type, OnActivityRecognitionEvent, data.get());
    if (ACTIVITY_ERROR_NONE != ret) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "Failed to start activity recognition",
                                ("activity_start_recognition() error: %d - %s", ret, get_error_message(ret)));
    }

    activity_data_.insert(std::make_pair(id, data));
    *watch_id = id;

    return result;
  }

  PlatformResult RemoveListener(long watch_id) {
    ScopeLogger();

    const auto it = activity_data_.find(watch_id);

    if (activity_data_.end() != it) {
      activity_data_.erase(it);
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  }

 private:
  struct ActivityData {
    ActivityData(long id, const JsonCallback& cb, activity_h h)
        : watch_id(id),
          callback(cb),
          handle(h) {
    }

    ~ActivityData() {
      if (handle) {
        activity_stop_recognition(handle);
        activity_release(handle);
      }
    }

    long watch_id;
    JsonCallback callback;
    activity_h handle;
  };

  static long GetNextId() {
    static long id = 0;
    return ++id;
  }

  static void OnActivityRecognitionEvent(activity_type_e type,
                                         const activity_data_h data,
                                         double timestamp,
                                         activity_error_e callback_error,
                                         void* user_data) {
    ScopeLogger();

    auto activity_data = static_cast<ActivityData*>(user_data);
    JsonCallback callback = activity_data->callback;

    picojson::value val{picojson::object{}};
    auto& obj = val.get<picojson::object>();
    obj.insert(std::make_pair("watchId", picojson::value(static_cast<double>(activity_data->watch_id))));

    if (ACTIVITY_ERROR_NONE != callback_error) {
      LogAndReportError(PlatformResult(ErrorCode::ABORT_ERR, "System operation has failed"),
                        &obj,
                        ("activity_recognition_cb() has failed with error code %d - %s", callback_error, get_error_message(callback_error)));
      callback(&val);
      return;
    }

    activity_accuracy_e accuracy = ACTIVITY_ACCURACY_MID;

    int ret = activity_get_accuracy(data, &accuracy);
    if (ret != ACTIVITY_ERROR_NONE) {
      LogAndReportError(PlatformResult(ErrorCode::ABORT_ERR, "System operation has failed"),
                        &obj,
                        ("activity_get_accuracy() has failed with error code %d - %s", ret, get_error_message(ret)));
      callback(&val);
      return;
    }

    SLoggerD("Activity type: (%d)", type);
    SLoggerD("Activity accuracy: (%d)", accuracy);
    SLoggerD("Activity timestamp: (%f)", timestamp);

    picojson::value result{picojson::object{}};
    auto& result_obj = result.get<picojson::object>();

    result_obj.insert(std::make_pair("type", picojson::value(FromActivityType(type))));
    result_obj.insert(std::make_pair("accuracy", picojson::value(FromActivityAccuracy(accuracy))));
    result_obj.insert(std::make_pair(kTimestamp, picojson::value(timestamp)));

    ReportSuccess(result, obj);
    callback(&val);
  }

  PlatformResult IsSupported(activity_type_e type) {
    ScopeLogger();

    bool supported = false;
    int ret = activity_is_supported(type, &supported);

    if (ret == ACTIVITY_ERROR_NOT_SUPPORTED || !supported) {
      return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR,
                                "Activity type is not supported",
                                ("Type %d not supported", type));
    } else if (ret != ACTIVITY_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "activity_is_supported failed",
                                ("activity_is_supported error %d - %s", ret, get_error_message(ret)));
    } else {
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

#define ACTIVITY_TYPE_E \
  X(ACTIVITY_STATIONARY, "STATIONARY") \
  X(ACTIVITY_WALK, "WALKING") \
  X(ACTIVITY_RUN, "RUNNING") \
  X(ACTIVITY_IN_VEHICLE, "IN_VEHICLE") \
  XD(static_cast<activity_type_e>(-1), "unknown")

#define ACTIVITY_ACCURACY_E \
  X(ACTIVITY_ACCURACY_LOW, "LOW") \
  X(ACTIVITY_ACCURACY_MID, "MEDIUM") \
  X(ACTIVITY_ACCURACY_HIGH, "HIGH") \
  XD(static_cast<activity_accuracy_e>(-1), "unknown")

#define X(v, s) case v: return s;
#define XD(v, s) \
  default: \
    LoggerE("Unknown value: %d, returning default: %s", e, s); \
    return s;

  static std::string FromActivityType(activity_type_e e) {
    ScopeLogger();

    switch (e) {
      ACTIVITY_TYPE_E
    }
  }

  static std::string FromActivityAccuracy(activity_accuracy_e e) {
    ScopeLogger();

    switch (e) {
      ACTIVITY_ACCURACY_E
    }
  }

#undef X
#undef XD

#define X(v, s) if (e == s) return v;
#define XD(v, s) \
  LoggerE("Unknown value: %s, returning default: %d", e.c_str(), v); \
  return v;

  static activity_type_e ToActivityType(const std::string& e) {
    ScopeLogger();

    ACTIVITY_TYPE_E
  }

#undef X
#undef XD

#undef ACTIVITY_ACCURACY_E
#undef ACTIVITY_TYPE_E

  std::map<long, std::shared_ptr<ActivityData>> activity_data_;
};

HumanActivityMonitorManager::HumanActivityMonitorManager()
    : activity_recognition_(std::make_shared<ActivityRecognition>()) {
  ScopeLogger();

  auto convert_pedometer = [](sensor_event_s* event, picojson::object* data) -> PlatformResult {
    ScopeLogger("convert_pedometer");

    const auto pedometer_data = (PedometerDataWrapper*)event;

    static const auto initial_pedometer_data = *pedometer_data;  // will be initialized only once
    static float steps_so_far = 0.0;

    const auto state = pedometer_data->state();

    if (SENSOR_PEDOMETER_STATE_UNKNOWN == state) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Unknown sensor step state");
    }

    data->insert(std::make_pair(kStepStatus, picojson::value(FromSensorPedometerState(state))));
    data->insert(std::make_pair(kSpeed, picojson::value(pedometer_data->speed())));
    data->insert(std::make_pair(kWalkingFrequency, picojson::value(pedometer_data->frequency())));

    data->insert(std::make_pair(kCumulativeDistance, picojson::value(pedometer_data->distance() - initial_pedometer_data.distance())));
    data->insert(std::make_pair(kCumulativeCalorie, picojson::value(pedometer_data->calories() - initial_pedometer_data.calories())));
    data->insert(std::make_pair(kCumulativeTotalStepCount, picojson::value(pedometer_data->steps() - initial_pedometer_data.steps())));
    data->insert(std::make_pair(kCumulativeWalkStepCount, picojson::value(pedometer_data->walk_steps() - initial_pedometer_data.walk_steps())));
    data->insert(std::make_pair(kCumulativeRunStepCount, picojson::value(pedometer_data->run_steps() - initial_pedometer_data.run_steps())));

    data->insert(std::make_pair(kAccumulativeDistance, picojson::value(pedometer_data->distance())));
    data->insert(std::make_pair(kAccumulativeCalorie, picojson::value(pedometer_data->calories())));
    data->insert(std::make_pair(kAccumulativeTotalStepCount, picojson::value(pedometer_data->steps())));
    data->insert(std::make_pair(kAccumulativeWalkStepCount, picojson::value(pedometer_data->walk_steps())));
    data->insert(std::make_pair(kAccumulativeRunStepCount, picojson::value(pedometer_data->run_steps())));

    auto& diffs = data->insert(std::make_pair(kStepCountDifferences, picojson::value{picojson::array{}})).first->second.get<picojson::array>();

    if (pedometer_data->diffs_count > 0) {
      for (int i = 0; i < pedometer_data->diffs_count; ++i) {
        InsertStepDifference(pedometer_data->diffs[i].steps, getCurrentTimeStamp(pedometer_data->diffs[i].timestamp) / 1000, &diffs);
      }
    } else {
      InsertStepDifference(steps_so_far > 0.0 ? pedometer_data->steps() - steps_so_far : 0.0, getCurrentTimeStamp(pedometer_data->timestamp) / 1000, &diffs);
    }

    steps_so_far = pedometer_data->steps();

    return PlatformResult(ErrorCode::NO_ERROR);
  };

  auto convert_hrm = [](sensor_event_s* event, picojson::object* data) -> PlatformResult {
    ScopeLogger("convert_hrm");

    LOGGER(DEBUG) << "Sensor event:";
    LOGGER(DEBUG) << "  |- accuracy: " << event->accuracy;
    LOGGER(DEBUG) << "  |- timestamp: " << event->timestamp;
    LOGGER(DEBUG) << "  |- value_count: " << event->value_count;

    if (event->value_count < 2) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "To few values of HRM event");
    }

    LOGGER(DEBUG) << "  |- values[0]: " << event->values[0];
    LOGGER(DEBUG) << "  |- values[1]: " << event->values[1];

    float hr = floor( event->values[0] + 0.5); // heart beat rate 0 ~ 220 integer (bpm)

    // there are no public native api for peak to peak interval.
    // but RRI = (60 / HR) * 1000
    // or unofficially values[1] is rri (0 ~ 5000 ms)
    float rri = floor(event->values[1] + 0.5);


    data->insert(std::make_pair("heartRate", picojson::value(static_cast<double>(hr))));
    data->insert(std::make_pair("rRInterval", picojson::value(static_cast<double>(rri))));

    return PlatformResult(ErrorCode::NO_ERROR);
  };

  auto convert_sleep = [](sensor_event_s* event, picojson::object* data) -> PlatformResult {
    ScopeLogger("convert_sleep");

    if (event->value_count < 1) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "To few values of SLEEP event");
    }

    sensor_sleep_state_e state = static_cast<sensor_sleep_state_e>(event->values[0]);
    std::string sleep_state;

    switch (state) {
      case SENSOR_SLEEP_STATE_WAKE:
        sleep_state = kSleepStateAwake;
        break;

      case SENSOR_SLEEP_STATE_SLEEP:
        sleep_state = kSleepStateAsleep;
        break;

      case SENSOR_SLEEP_STATE_UNKNOWN:
        sleep_state = kSleepStateUnknown;
        break;

      default:
        return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                  "Unknown sleep state",
                                  ("Unknown sleep state: %d", state));
    }

    data->insert(std::make_pair(kStatus, picojson::value(sleep_state)));
    data->insert(std::make_pair(kTimestamp, picojson::value(static_cast<double>(event->timestamp))));

    return PlatformResult(ErrorCode::NO_ERROR);
  };

  monitors_.insert(std::make_pair(kActivityTypePedometer, std::make_shared<Monitor::SensorMonitor>(kActivityTypePedometer, SENSOR_HUMAN_PEDOMETER, convert_pedometer)));
  monitors_.insert(std::make_pair(kActivityTypeWristUp, std::make_shared<Monitor::GestureMonitor>(kActivityTypeWristUp)));
  monitors_.insert(std::make_pair(kActivityTypeHrm, std::make_shared<Monitor::SensorMonitor>(kActivityTypeHrm, SENSOR_HRM, convert_hrm)));
  monitors_.insert(std::make_pair(kActivityTypeGps, std::make_shared<Monitor::GpsMonitor>(kActivityTypeGps)));
  monitors_.insert(std::make_pair(kActivityTypeSleepMonitor, std::make_shared<Monitor::SensorMonitor>(kActivityTypeSleepMonitor, SENSOR_HUMAN_SLEEP_MONITOR, convert_sleep)));
}

HumanActivityMonitorManager::~HumanActivityMonitorManager() {
  ScopeLogger();
}

PlatformResult HumanActivityMonitorManager::Init() {
  ScopeLogger();
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::SetListener(
    const std::string& type, JsonCallback callback,
    const picojson::value& args) {
  ScopeLogger();
  return GetMonitor(type)->SetListener(callback, args);
}

PlatformResult HumanActivityMonitorManager::UnsetListener(
    const std::string& type) {
  ScopeLogger();
  return GetMonitor(type)->UnsetListener();
}

PlatformResult HumanActivityMonitorManager::GetHumanActivityData(
    const std::string& type, picojson::value* data) {
  ScopeLogger();
  return GetMonitor(type)->GetData(data);
}

PlatformResult HumanActivityMonitorManager::AddActivityRecognitionListener(
    const std::string& type, JsonCallback callback, long* watch_id) {
  ScopeLogger();
  return activity_recognition_->AddListener(type, callback, watch_id);
}

PlatformResult HumanActivityMonitorManager::RemoveActivityRecognitionListener(const long watch_id) {
  ScopeLogger();
  return activity_recognition_->RemoveListener(watch_id);
}

std::shared_ptr<HumanActivityMonitorManager::Monitor> HumanActivityMonitorManager::GetMonitor(const std::string& type) {
  ScopeLogger();

  const auto it = monitors_.find(type);

  if (monitors_.end() != it) {
    return it->second;
  } else {
    return std::make_shared<Monitor>(type);  // return default unsupported monitor
  }
}

}  // namespace humanactivitymonitor
}  // namespace extension
