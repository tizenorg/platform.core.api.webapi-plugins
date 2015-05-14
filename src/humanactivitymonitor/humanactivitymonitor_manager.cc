// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "humanactivitymonitor/humanactivitymonitor_manager.h"

#include <gesture_recognition.h>

#include "common/logger.h"

namespace extension {
namespace humanactivitymonitor {

using common::PlatformResult;
using common::ErrorCode;

HumanActivityMonitorManager::HumanActivityMonitorManager() {}

HumanActivityMonitorManager::~HumanActivityMonitorManager() {
  UnsetHrmListener();
}

PlatformResult HumanActivityMonitorManager::Init() {
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
    // TODO(r.galka) implement when available in platform
  } else if (type == kActivityTypeHrm) {
    ret = sensor_is_supported(SENSOR_HRM, &supported);
    if (ret != SENSOR_ERROR_NONE) {
      LOGGER(ERROR) << "sensor_is_supported(HRM), error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "HRM sensor check failed");
    }
  } else if (type == kActivityTypeGps) {
    // TODO(r.galka) implement when available in platform
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

  int ret;

  // PEDOMETER
  if (type == kActivityTypePedometer) {
    // TODO(r.galka) Not Supported in current implementation.
  }

  // WRIST_UP
  if (type == kActivityTypeWristUp) {
    // TODO(r.galka) implement
  }

  // HRM
  if (type == kActivityTypeHrm) {
    return SetHrmListener(callback);
  }

  // GPS
  if (type == kActivityTypeGps) {
    // TODO(r.galka) implement
  }
}

PlatformResult HumanActivityMonitorManager::UnsetListener(
    const std::string& type) {

  PlatformResult result = IsSupported(type);
  if (!result) {
    return result;
  }

  // PEDOMETER
  if (type == kActivityTypePedometer) {
    // TODO(r.galka) Not Supported in current implementation.
  }

  // WRIST_UP
  if (type == kActivityTypeWristUp) {
    // TODO(r.galka) implement
  }

  // HRM
  if (type == kActivityTypeHrm) {
    return UnsetHrmListener();
  }

  // GPS
  if (type == kActivityTypeGps) {
    // TODO(r.galka) implement
  }
}

// HRM
PlatformResult HumanActivityMonitorManager::SetHrmListener(
    JsonCallback callback) {
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
  int ret;

  if (hrm_sensor_listener_) {
    ret = sensor_listener_stop(hrm_sensor_listener_);
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

void HumanActivityMonitorManager::OnHrmSensorEvent(
    sensor_h sensor, sensor_event_s *event, void *user_data) {
  LOGGER(DEBUG) << "Sensor event:";
  LOGGER(DEBUG) << "  |- accuracy: " << event->accuracy;
  LOGGER(DEBUG) << "  |- timestamp: " << event->timestamp;
  LOGGER(DEBUG) << "  |- value_count: " << event->value_count;

  if (event->value_count < 2) {
    LOGGER(ERROR) << "To few values of HRM event";
    return;
  }

  LOGGER(DEBUG) << "  |- values[0]: " << event->values[0];
  LOGGER(DEBUG) << "  |- values[1]: " << event->values[1];

  float hr = event->values[0]; // heart beat rate 0 ~ 220 integer (bpm)

  // there are no public native api for peak to peak interval.
  // but RRI = (60 / HR) * 1000
  // or unofficially values[1] is rri (0 ~ 5000 ms)
  float rri = event->values[1];

  HumanActivityMonitorManager* manager =
      static_cast<HumanActivityMonitorManager*>(user_data);

  picojson::value hrm_data = picojson::value(picojson::object());
  picojson::object& hrm_data_o = hrm_data.get<picojson::object>();

  hrm_data_o["heartRate"] = picojson::value(static_cast<double>(hr));
  hrm_data_o["rRInterval"] = picojson::value(static_cast<double>(rri));

  manager->hrm_event_callback_(&hrm_data);
}

}  // namespace humanactivitymonitor
}  // namespace extension
