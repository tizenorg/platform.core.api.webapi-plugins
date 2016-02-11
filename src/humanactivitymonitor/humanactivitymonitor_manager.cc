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

#include <gesture_recognition.h>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/tools.h"

namespace extension {
namespace humanactivitymonitor {

using common::PlatformResult;
using common::ErrorCode;
using common::tools::ReportError;
using common::tools::ReportSuccess;

namespace {
const std::string kActivityRecognitionStationary = "STATIONARY";
const std::string kActivityRecognitionWalking = "WALKING";
const std::string kActivityRecognitionRunning = "RUNNING";
const std::string kActivityRecognitionInVehicle = "IN_VEHICLE";

const std::string kActivityAccuracyLow = "LOW";
const std::string kActivityAccuracyMedium = "MEDIUM";
const std::string kActivityAccuracyHigh = "HIGH";

long GetNextId() {
  static long id = 0;
  return ++id;
}
}

HumanActivityMonitorManager::HumanActivityMonitorManager()
    : gesture_handle_(nullptr),
      hrm_sensor_listener_(nullptr),
      location_handle_(nullptr) {
  LoggerD("Enter");
}

HumanActivityMonitorManager::~HumanActivityMonitorManager() {
  LoggerD("Enter");
  UnsetWristUpListener();
  UnsetHrmListener();
  UnsetGpsListener();

  for (const auto& it: handles_cb_) {
    activity_release(it.second->handle);
    delete it.second;
  }
  handles_cb_.erase(handles_cb_.begin(), handles_cb_.end());
}

PlatformResult HumanActivityMonitorManager::Init() {
  LoggerD("Enter");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::IsSupported(
    const std::string& type) {

  // check cache first
  if (supported_.count(type)) {
    return LogAndCreateResult(supported_[type]
        ? ErrorCode::NO_ERROR
        : ErrorCode::NOT_SUPPORTED_ERR);
  }

  int ret;
  bool supported = false;
  if (type == kActivityTypePedometer) {
    // TODO(r.galka) no native api for pedometer
    // so just pass it for not supported.
  } else if (type == kActivityTypeWristUp) {
    ret = gesture_is_supported(GESTURE_WRIST_UP, &supported);
    if (ret != SENSOR_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "WRIST_UP gesture check failed",
                            ("gesture_is_supported(GESTURE_WRIST_UP), error: %d",ret));
    }
  } else if (type == kActivityTypeHrm) {
    ret = sensor_is_supported(SENSOR_HRM, &supported);
    if (ret != SENSOR_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "HRM sensor check failed",
                            ("sensor_is_supported(HRM), error: %d",ret));
    }
  } else if (type == kActivityTypeGps) {
    supported = location_manager_is_supported_method(LOCATIONS_METHOD_GPS);
  } else if (type == kActivityRecognitionStationary) {
    ret = activity_is_supported(ACTIVITY_STATIONARY, &supported);
    if (ret == ACTIVITY_ERROR_NOT_SUPPORTED || !supported) {
      LoggerD("Type %s not supported", type.c_str());
    } else if (ret != ACTIVITY_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "activity_is_supported failed",
                                ("activity_is_supported error %d - %s",ret, get_error_message(ret)));
    }
  } else if (type == kActivityRecognitionWalking) {
    ret = activity_is_supported(ACTIVITY_WALK, &supported);
    if (ret == ACTIVITY_ERROR_NOT_SUPPORTED || !supported) {
      LoggerD("Type %s not supported", type.c_str());
    } else if (ret != ACTIVITY_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "activity_is_supported failed",
                                ("activity_is_supported error %d - %s",ret, get_error_message(ret)));
    }
  } else if (type == kActivityRecognitionRunning) {
    ret = activity_is_supported(ACTIVITY_RUN, &supported);
    if (ret == ACTIVITY_ERROR_NOT_SUPPORTED || !supported) {
      LoggerD("Type %s not supported", type.c_str());
    } else if (ret != ACTIVITY_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "activity_is_supported failed",
                                ("activity_is_supported error %d - %s",ret, get_error_message(ret)));
    }
  } else if (type == kActivityRecognitionInVehicle) {
    ret = activity_is_supported(ACTIVITY_IN_VEHICLE, &supported);
    if (ret == ACTIVITY_ERROR_NOT_SUPPORTED || !supported) {
      LoggerD("Type %s not supported", type.c_str());
    } else if (ret != ACTIVITY_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                                "activity_is_supported failed",
                                ("activity_is_supported error %d - %s",ret, get_error_message(ret)));
    }
  } else {
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR);
  }

  supported_[type] = supported;

  return LogAndCreateResult(supported_[type]
      ? ErrorCode::NO_ERROR
      : ErrorCode::NOT_SUPPORTED_ERR);
}

PlatformResult HumanActivityMonitorManager::SetListener(
    const std::string& type , JsonCallback callback, const picojson::value& args) {

  PlatformResult result = IsSupported(type);
  if (!result) {
    return result;
  }

  if (type == kActivityTypePedometer) {
    // TODO(r.galka) Not Supported in current implementation.
  }

  if (type == kActivityTypeWristUp) {
    return SetWristUpListener(callback);
  }

  if (type == kActivityTypeHrm) {
    return SetHrmListener(callback, args);
  }

  if (type == kActivityTypeGps) {
    return SetGpsListener(callback, args);
  }

  return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Undefined activity type");
}

PlatformResult HumanActivityMonitorManager::UnsetListener(
    const std::string& type) {

  PlatformResult result = IsSupported(type);
  if (!result) {
    return result;
  }

  if (type == kActivityTypePedometer) {
    // TODO(r.galka) Not Supported in current implementation.
  }

  if (type == kActivityTypeWristUp) {
    return UnsetWristUpListener();
  }

  if (type == kActivityTypeHrm) {
    return UnsetHrmListener();
  }

  if (type == kActivityTypeGps) {
    return UnsetGpsListener();
  }

  return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Undefined activity type");
}

void HumanActivityMonitorManager:: ActivityRecognitionCb(
                activity_type_e type,
                const activity_data_h data,
                double timestamp,
                activity_error_e callback_error,
                void *user_data) {
  LoggerD("enter");

  HandleCallback* handle_callback = static_cast<HandleCallback*>(user_data);
  long watch_id = handle_callback->watch_id;
  JsonCallback callback = handle_callback->callback;

  picojson::value val = picojson::value(picojson::object());
  picojson::object& obj = val.get<picojson::object>();
  obj["watchId"] = picojson::value(static_cast<double>(watch_id));

  if (callback_error != ACTIVITY_ERROR_NONE) {
    LogAndReportError(
        PlatformResult(ErrorCode::ABORT_ERR, "System operation was failed"),
        &obj,
        ("activity_recognition_cb() is failed with error code %d - %s", callback_error, get_error_message(callback_error)));
    callback(&val);
    return;
  }

  activity_accuracy_e accuracy = ACTIVITY_ACCURACY_MID;

  int ret = activity_get_accuracy(data, &accuracy);
  if (ret != ACTIVITY_ERROR_NONE) {
    LogAndReportError(
        PlatformResult(ErrorCode::ABORT_ERR, "System operation was failed"),
        &obj,
        ("activity_get_accuracy() is failed with error code %d - %s", ret, get_error_message(ret)));
    callback(&val);
    return;
  }

  const char *type_str;

  switch (type) {
    case ACTIVITY_STATIONARY:
      type_str = kActivityRecognitionStationary.c_str();
      break;

    case ACTIVITY_WALK:
      type_str = kActivityRecognitionWalking.c_str();
      break;

    case ACTIVITY_RUN:
      type_str = kActivityRecognitionRunning.c_str();
      break;

    case ACTIVITY_IN_VEHICLE:
      type_str = kActivityRecognitionInVehicle.c_str();
      break;

    default:
      LogAndReportError(
          PlatformResult(ErrorCode::ABORT_ERR, "Unknown activity recognition type"),
          &obj,
          ("Unknown activity recognition type %d", type));
      callback(&val);
      return;
  }

  LoggerD("Activity type: (%s)", type_str);

  const char *accuracy_str;

  switch (accuracy) {
    case ACTIVITY_ACCURACY_LOW:
      accuracy_str = kActivityAccuracyLow.c_str();
      break;

    case ACTIVITY_ACCURACY_MID:
      accuracy_str = kActivityAccuracyMedium.c_str();
      break;

    case ACTIVITY_ACCURACY_HIGH:
      accuracy_str = kActivityAccuracyHigh.c_str();
      break;

    default:
      LogAndReportError(
          PlatformResult(ErrorCode::ABORT_ERR, "Unknown activity accuracy type"),
          &obj,
          ("Unknown activity accuracy type %d", accuracy));
      callback(&val);
      return;
  }

  LoggerD("accuracy: (%s)", accuracy_str);
  LoggerD("##### timeStamp: (%f)", timestamp);

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  result_obj.insert(std::make_pair("type", picojson::value(type_str)));
  result_obj.insert(std::make_pair("timestamp", picojson::value(timestamp)));
  result_obj.insert(std::make_pair("accuracy", picojson::value(accuracy_str)));

  ReportSuccess(result, obj);
  callback(&val);
  return;
}

PlatformResult HumanActivityMonitorManager:: AddActivityRecognitionListener(
    const std::string& type, JsonCallback callback, const picojson::value& args, long* watch_id) {
  LoggerD("Enter");

  PlatformResult result = IsSupported(type);
  if (!result) {
    return result;
  }

  activity_type_e activity_type = ACTIVITY_STATIONARY;

  if (type == kActivityRecognitionStationary) {
    activity_type = ACTIVITY_STATIONARY;
  } else if (type == kActivityRecognitionWalking) {
    activity_type = ACTIVITY_WALK;
  } else if (type == kActivityRecognitionRunning) {
    activity_type = ACTIVITY_RUN;
  } else if (type == kActivityRecognitionInVehicle) {
    activity_type = ACTIVITY_IN_VEHICLE;
  } else {
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR,
                              "A type not supported",
                              ("The type %s is not matched with the activity recognition type", type.c_str()));
  }

  activity_h handle = nullptr;
  int ret = activity_create(&handle);
  if (ret != ACTIVITY_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                              "activity_create error",
                              ("activity_create error: %d - %s", ret, get_error_message(ret)));
  }

  long id = GetNextId();

  // Adding the handle to handles map
  HandleCallback* handle_callback = new HandleCallback(id, callback, handle);

  handles_cb_[id] = handle_callback;

  ret = activity_start_recognition(handle, activity_type, ActivityRecognitionCb, static_cast<void*>(handle_callback));
  if (ret != ACTIVITY_ERROR_NONE) {
    delete handle_callback;
    handles_cb_.erase(id);
    activity_release(handle);
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                              "activity_start_recognition error",
                              ("activity_start_recognition error: %d - %s", ret, get_error_message(ret)));
  }

  *watch_id = id;

  return result;
}

PlatformResult HumanActivityMonitorManager::RemoveActivityRecognitionListener(const long watch_id) {
  LoggerD("Enter");

  if (handles_cb_.find(watch_id) == handles_cb_.end()) {
    return LogAndCreateResult(
              ErrorCode::ABORT_ERR,
              "Listener not found",
              ("Listener with id = %ld not found", watch_id));
  }
  activity_h handle = handles_cb_[watch_id]->handle;

  int ret = activity_stop_recognition(handle);
  if (ret != ACTIVITY_ERROR_NONE) {
    return LogAndCreateResult(
              ErrorCode::ABORT_ERR,
              "System operation was failed",
              ("Activity_stop_recognition() return (%d) - %s", ret, get_error_message(ret)));
  }

  activity_release(handle);
  delete handles_cb_[watch_id];
  handles_cb_.erase(watch_id);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::GetHumanActivityData(
    const std::string& type,
    picojson::value* data) {

  LoggerD("Enter");
  if (type == kActivityTypePedometer) {
    // TODO(r.galka) Not Supported in current implementation.
  }

  if (type == kActivityTypeWristUp) {
    return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR);
  }

  if (type == kActivityTypeHrm) {
    return GetHrmData(data);
  }

  if (type == kActivityTypeGps) {
    return GetGpsData(data);
  }

  return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Undefined activity type");
}

// WRIST_UP
PlatformResult HumanActivityMonitorManager::SetWristUpListener(
    JsonCallback callback) {
  LoggerD("Enter");
  int ret;

  ret = gesture_create(&gesture_handle_);
  if (ret != GESTURE_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create WRIST_UP listener",
                          ("Failed to create WRIST_UP handle, error: %d",ret));
  }

  ret = gesture_start_recognition(gesture_handle_,
                                  GESTURE_WRIST_UP,
                                  GESTURE_OPTION_DEFAULT,
                                  OnWristUpEvent,
                                  this);
  if (ret != GESTURE_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to start WRIST_UP listener",
                          ("Failed to start WRIST_UP listener, error: %d",ret));
  }

  wrist_up_event_callback_ = callback;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::UnsetWristUpListener() {
  LoggerD("Enter");

  if (gesture_handle_) {
    int ret = gesture_stop_recognition(gesture_handle_);
    if (ret != GESTURE_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to stop WRIST_UP detection, error: " << ret;
    }

    ret = gesture_release(gesture_handle_);
    if (ret != GESTURE_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to release WRIST_UP handle, error: " << ret;
    }
  }

  wrist_up_event_callback_ = nullptr;

  return PlatformResult(ErrorCode::NO_ERROR);
}

void HumanActivityMonitorManager::OnWristUpEvent(gesture_type_e gesture,
                                                 const gesture_data_h data,
                                                 double timestamp,
                                                 gesture_error_e error,
                                                 void* user_data) {
  LoggerD("Enter");
  HumanActivityMonitorManager* manager =
      static_cast<HumanActivityMonitorManager*>(user_data);

  if (!manager->wrist_up_event_callback_) {
    LOGGER(ERROR) << "No WRIST_UP event callback registered, skipping.";
    return;
  }

  picojson::value v = picojson::value(); // null value
  manager->wrist_up_event_callback_(&v);
}

// HRM
PlatformResult HumanActivityMonitorManager::SetHrmListener(
    JsonCallback callback, const picojson::value& args) {
  LoggerD("Enter");
  sensor_h hrm_sensor;
  int ret;

  ret = sensor_get_default_sensor(SENSOR_HRM, &hrm_sensor);
  if (ret != SENSOR_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get HRM sensor",
                          ("Failed to get HRM sensor, error: %d",ret));
  }

  ret = sensor_create_listener(hrm_sensor, &hrm_sensor_listener_);
  if (ret != SENSOR_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create HRM sensor listener",
                          ("Failed to create HRM sensor listener, error: %d",ret));
  }

  int callbackInterval = static_cast<int>(args.get("callbackInterval").get<double>());
  LoggerD("callbackInterval: %d", callbackInterval);

  ret = sensor_listener_set_event_cb(hrm_sensor_listener_,
                                     callbackInterval,
                                     OnHrmSensorEvent,
                                     this);
  if (ret != SENSOR_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to set HRM sensor listener",
                          ("Failed to set HRM sensor listener, error: %d",ret));
  }

  ret = sensor_listener_start(hrm_sensor_listener_);
  if (ret != SENSOR_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to start HRM sensor listener",
                          ("Failed to start HRM sensor listener, error: %d",ret));
  }

  hrm_event_callback_ = callback;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::UnsetHrmListener() {
  LoggerD("Enter");

  if (hrm_sensor_listener_) {
    int ret = sensor_listener_stop(hrm_sensor_listener_);
    if (ret != SENSOR_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to stop HRM sensor",
                            ("Failed to stop HRM sensor, error: %d",ret));
    }

    ret = sensor_listener_unset_event_cb(hrm_sensor_listener_);
    if (ret != SENSOR_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to unset HRM sensor listener",
                            ("Failed to unset HRM sensor listener, error: %d",ret));
    }

    ret = sensor_destroy_listener(hrm_sensor_listener_);
    if (ret != SENSOR_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to destroy HRM sensor listener",
                            ("Failed to destroy HRM sensor listener, error: %d",ret));
    }
  }

  hrm_event_callback_ = nullptr;

  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult ConvertHrmEvent(sensor_event_s* event,
                                      picojson::object* data) {
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


  (*data)["heartRate"] = picojson::value(static_cast<double>(hr));
  (*data)["rRInterval"] = picojson::value(static_cast<double>(rri));

  return PlatformResult(ErrorCode::NO_ERROR);
}

void HumanActivityMonitorManager::OnHrmSensorEvent(
    sensor_h /*sensor*/, sensor_event_s *event, void *user_data) {

  LoggerD("Enter");
  HumanActivityMonitorManager* manager =
      static_cast<HumanActivityMonitorManager*>(user_data);

  if (!manager->hrm_event_callback_) {
    LOGGER(ERROR) << "No HRM event callback registered, skipping.";
    return;
  }

  picojson::value hrm_data = picojson::value(picojson::object());
  PlatformResult result = ConvertHrmEvent(event,
                                          &hrm_data.get<picojson::object>());
  if (!result) {
    LOGGER(ERROR) << "Failed to convert HRM data: " << result.message();
    return;
  }

  manager->hrm_event_callback_(&hrm_data);
}

PlatformResult HumanActivityMonitorManager::GetHrmData(picojson::value* data) {
  LoggerD("Enter");
  if (!hrm_sensor_listener_) {
    return LogAndCreateResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
  }

  int ret;

  sensor_event_s event;
  ret = sensor_listener_read_data(hrm_sensor_listener_, &event);
  if (ret != SENSOR_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get HRM sensor data",
                          ("Failed to get HRM sensor data, error: %d",ret));
  }

  *data = picojson::value(picojson::object());
  PlatformResult result = ConvertHrmEvent(&event,
                                          &data->get<picojson::object>());
  if (!result) {
    return result;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

// GPS
PlatformResult HumanActivityMonitorManager::SetGpsListener(
    JsonCallback callback, const picojson::value& args) {
  LoggerD("Enter");
  int ret;

  ret = location_manager_create(LOCATIONS_METHOD_GPS, &location_handle_);
  if (ret != LOCATIONS_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create location manager",
                          ("Failed to create location manager, error: %d",ret));
  }

  int callbackInterval = static_cast<int>(args.get("callbackInterval").get<double>()/1000);
  int sampleInterval = static_cast<int>(args.get("sampleInterval").get<double>()/1000);
  LoggerD("callbackInterval: %d, sampleInterval: %d", callbackInterval, sampleInterval);

  ret = location_manager_set_location_batch_cb(location_handle_,
                                               OnGpsEvent,
                                               sampleInterval, // batch_interval
                                               callbackInterval, // batch_period
                                               this);
  if (ret != LOCATIONS_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to set location listener",
                          ("Failed to set location listener, error: %d",ret));
  }

  ret = location_manager_start_batch(location_handle_);
  if (ret != LOCATIONS_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to start location manager",
                          ("Failed to start location manager, error: %d",ret));
  }

  gps_event_callback_ = callback;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::UnsetGpsListener() {
  LoggerD("Enter");

  if (location_handle_) {
    int ret = location_manager_stop_batch(location_handle_);
    if (ret != LOCATIONS_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to stop location manager",
                            ("Failed to stop location manager, error: %d",ret));
    }

    ret = location_manager_unset_location_batch_cb(location_handle_);
    if (ret != LOCATIONS_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to unset location listener",
                            ("Failed to unset location listener, error: %d",ret));
    }

    ret = location_manager_destroy(location_handle_);
    if (ret != LOCATIONS_ERROR_NONE) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to destroy location manager",
                            ("Failed to destroy location manager, error: %d",ret));
    }
  }

  gps_event_callback_ = nullptr;

  return PlatformResult(ErrorCode::NO_ERROR);
}

static bool ConvertGpsEvent(double latitude, double longitude, double altitude,
                            double speed, double direction, double horizontal,
                            double vertical, time_t timestamp,
                            void* user_data) {
  LoggerD("Enter");
  picojson::array* gps_info_array = static_cast<picojson::array*>(user_data);

  picojson::value gps_info = picojson::value(picojson::object());
  picojson::object& gps_info_o = gps_info.get<picojson::object>();

  gps_info_o["latitude"] = picojson::value(latitude);
  gps_info_o["longitude"] = picojson::value(longitude);
  gps_info_o["altitude"] = picojson::value(altitude);
  gps_info_o["speed"] = picojson::value(speed);
  // TODO(r.galka) errorRange not available in CAPI
  gps_info_o["errorRange"] = picojson::value(static_cast<double>(0));
  gps_info_o["timestamp"] = picojson::value(static_cast<double>(timestamp));

  gps_info_array->push_back(gps_info);

  return true;
}

void HumanActivityMonitorManager::OnGpsEvent(int num_of_location,
                                             void *user_data) {
  LoggerD("Enter");
  HumanActivityMonitorManager* manager =
      static_cast<HumanActivityMonitorManager*>(user_data);

  if (!manager->gps_event_callback_) {
    LOGGER(ERROR) << "No GPS event callback registered, skipping.";
    return;
  }

  if (0 == num_of_location) {
    LOGGER(ERROR) << "No GPS locations available, skipping.";
    return;
  }

  picojson::value gps_info = picojson::value(picojson::array());
  int ret = location_manager_foreach_location_batch(
      manager->location_handle_,
      ConvertGpsEvent,
      &gps_info.get<picojson::array>());
  if (ret != LOCATIONS_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to convert location, error: " << ret;
    return;
  }

  manager->gps_event_callback_(&gps_info);
}

PlatformResult HumanActivityMonitorManager::GetGpsData(picojson::value* data) {
  LoggerD("Enter");
  if (!location_handle_) {
    return LogAndCreateResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
  }

  int ret;
  double altitude, latitude, longitude, climb,
      direction, speed, horizontal, vertical;
  location_accuracy_level_e level;
  time_t timestamp;
  ret = location_manager_get_location(location_handle_, &altitude, &latitude,
                                      &longitude, &climb, &direction, &speed,
                                      &level, &horizontal, &vertical,
                                      &timestamp);
  if (ret != LOCATIONS_ERROR_NONE) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Failed to get location",
                              ("Failed to get location, error: %d",ret));
  }

  *data = picojson::value(picojson::array());
  ConvertGpsEvent(latitude, longitude, altitude, speed, direction, horizontal,
                  vertical, timestamp, &data->get<picojson::array>());

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace humanactivitymonitor
}  // namespace extension
