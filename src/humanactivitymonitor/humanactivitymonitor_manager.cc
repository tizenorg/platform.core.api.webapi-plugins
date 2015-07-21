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

namespace extension {
namespace humanactivitymonitor {

using common::PlatformResult;
using common::ErrorCode;

HumanActivityMonitorManager::HumanActivityMonitorManager() {
  LoggerD("Enter");
}

HumanActivityMonitorManager::~HumanActivityMonitorManager() {
  LoggerD("Enter");
  UnsetWristUpListener();
  UnsetHrmListener();
  UnsetGpsListener();
}

PlatformResult HumanActivityMonitorManager::Init() {
  LoggerD("Enter");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::IsSupported(
    const std::string& type) {

  // check cache first
  if (supported_.count(type)) {
    return PlatformResult(supported_[type]
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
      LOGGER(ERROR) << "gesture_is_supported(GESTURE_WRIST_UP), error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "WRIST_UP gesture check failed");
    }
  } else if (type == kActivityTypeHrm) {
    ret = sensor_is_supported(SENSOR_HRM, &supported);
    if (ret != SENSOR_ERROR_NONE) {
      LOGGER(ERROR) << "sensor_is_supported(HRM), error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "HRM sensor check failed");
    }
  } else if (type == kActivityTypeGps) {
    supported = location_manager_is_supported_method(LOCATIONS_METHOD_GPS);
  } else {
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR);
  }

  supported_[type] = supported;

  return PlatformResult(supported_[type]
      ? ErrorCode::NO_ERROR
      : ErrorCode::NOT_SUPPORTED_ERR);
}

PlatformResult HumanActivityMonitorManager::SetListener(
    const std::string& type, JsonCallback callback) {

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
    return SetHrmListener(callback);
  }

  if (type == kActivityTypeGps) {
    return SetGpsListener(callback);
  }

  return PlatformResult(ErrorCode::UNKNOWN_ERR, "Undefined activity type");
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

  return PlatformResult(ErrorCode::UNKNOWN_ERR, "Undefined activity type");
}

PlatformResult HumanActivityMonitorManager::GetHumanActivityData(
    const std::string& type,
    picojson::value* data) {

  LoggerD("Enter");
  if (type == kActivityTypePedometer) {
    // TODO(r.galka) Not Supported in current implementation.
  }

  if (type == kActivityTypeWristUp) {
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR);
  }

  if (type == kActivityTypeHrm) {
    return GetHrmData(data);
  }

  if (type == kActivityTypeGps) {
    return GetGpsData(data);
  }

  return PlatformResult(ErrorCode::UNKNOWN_ERR, "Undefined activity type");
}

// WRIST_UP
PlatformResult HumanActivityMonitorManager::SetWristUpListener(
    JsonCallback callback) {
  LoggerD("Enter");
  int ret;

  ret = gesture_create(&gesture_handle_);
  if (ret != GESTURE_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to create WRIST_UP handle, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create WRIST_UP listener");
  }

  ret = gesture_start_recognition(gesture_handle_,
                                  GESTURE_WRIST_UP,
                                  GESTURE_OPTION_DEFAULT,
                                  OnWristUpEvent,
                                  this);
  if (ret != GESTURE_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to start WRIST_UP listener, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to start WRIST_UP listener");
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
    JsonCallback callback) {
  LoggerD("Enter");
  sensor_h hrm_sensor;
  int ret;

  ret = sensor_get_default_sensor(SENSOR_HRM, &hrm_sensor);
  if (ret != SENSOR_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to get HRM sensor, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get HRM sensor");
  }

  ret = sensor_create_listener(hrm_sensor, &hrm_sensor_listener_);
  if (ret != SENSOR_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to create HRM sensor listener, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create HRM sensor listener");
  }

  ret = sensor_listener_set_event_cb(hrm_sensor_listener_,
                                     0,
                                     OnHrmSensorEvent,
                                     this);
  if (ret != SENSOR_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to set HRM sensor listener, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to set HRM sensor listener");
  }

  ret = sensor_listener_start(hrm_sensor_listener_);
  if (ret != SENSOR_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to start HRM sensor listener, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to start HRM sensor listener");
  }

  hrm_event_callback_ = callback;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::UnsetHrmListener() {
  LoggerD("Enter");

  if (hrm_sensor_listener_) {
    int ret = sensor_listener_stop(hrm_sensor_listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to stop HRM sensor, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to stop HRM sensor");
    }

    ret = sensor_listener_unset_event_cb(hrm_sensor_listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to unset HRM sensor listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to unset HRM sensor listener");
    }

    ret = sensor_destroy_listener(hrm_sensor_listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to destroy HRM sensor listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to destroy HRM sensor listener");
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
    LOGGER(ERROR) << "To few values of HRM event";
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "To few values of HRM event");
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
    return PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
  }

  int ret;

  sensor_event_s event;
  ret = sensor_listener_read_data(hrm_sensor_listener_, &event);
  if (ret != SENSOR_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to get HRM sensor data, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get HRM sensor data");
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
    JsonCallback callback) {
  LoggerD("Enter");
  int ret;

  ret = location_manager_create(LOCATIONS_METHOD_GPS, &location_handle_);
  if (ret != LOCATIONS_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to create location manager, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create location manager");
  }

  ret = location_manager_set_location_batch_cb(location_handle_,
                                               OnGpsEvent,
                                               1, // batch_interval
                                               120, // batch_period
                                               this);
  if (ret != LOCATIONS_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to set location listener, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to set location listener");
  }

  ret = location_manager_start_batch(location_handle_);
  if (ret != LOCATIONS_ERROR_NONE) {
    LOGGER(ERROR) << "Failed to start location manager, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to start location manager");
  }

  gps_event_callback_ = callback;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult HumanActivityMonitorManager::UnsetGpsListener() {
  LoggerD("Enter");

  if (location_handle_) {
    int ret = location_manager_stop_batch(location_handle_);
    if (ret != LOCATIONS_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to stop location manager, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to stop location manager");
    }

    ret = location_manager_unset_location_batch_cb(location_handle_);
    if (ret != LOCATIONS_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to unset location listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to unset location listener");
    }

    ret = location_manager_destroy(location_handle_);
    if (ret != LOCATIONS_ERROR_NONE) {
      LOGGER(ERROR) << "Failed to destroy location manager, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to destroy location manager");
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
    return PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
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
    LOGGER(ERROR) << "Failed to get location, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get location");
  }

  *data = picojson::value(picojson::array());
  ConvertGpsEvent(latitude, longitude, altitude, speed, direction, horizontal,
                  vertical, timestamp, &data->get<picojson::array>());

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace humanactivitymonitor
}  // namespace extension
