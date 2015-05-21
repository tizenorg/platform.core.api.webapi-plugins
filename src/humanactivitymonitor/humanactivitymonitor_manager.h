// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H
#define HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H

#include <functional>
#include <sensor.h>
#include <gesture_recognition.h>
#include <location_batch.h>
#include <string>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace humanactivitymonitor {

namespace {
const std::string kActivityTypePedometer = "PEDOMETER";
const std::string kActivityTypeWristUp = "WRIST_UP";
const std::string kActivityTypeHrm = "HRM";
const std::string kActivityTypeGps = "GPS";
}

typedef std::function<void(picojson::value*)> JsonCallback;

class HumanActivityMonitorManager {
 public:
  HumanActivityMonitorManager();
  virtual ~HumanActivityMonitorManager();

  common::PlatformResult Init();

  common::PlatformResult SetListener(const std::string& type,
                                     JsonCallback callback);
  common::PlatformResult UnsetListener(const std::string& type);

  common::PlatformResult GetHumanActivityData(const std::string& type,
                                              picojson::value* data);

 private:
  std::map<std::string, bool> supported_;
  common::PlatformResult IsSupported(const std::string& type);

  // WRIST_UP
  gesture_h gesture_handle_;
  JsonCallback wrist_up_event_callback_;
  common::PlatformResult SetWristUpListener(JsonCallback callback);
  common::PlatformResult UnsetWristUpListener();
  static void OnWristUpEvent(gesture_type_e gesture,
                             const gesture_data_h data,
                             double timestamp,
                             gesture_error_e error,
                             void* user_data);

  // HRM
  sensor_listener_h hrm_sensor_listener_;
  JsonCallback hrm_event_callback_;
  common::PlatformResult SetHrmListener(JsonCallback callback);
  common::PlatformResult UnsetHrmListener();
  static void OnHrmSensorEvent(sensor_h sensor,
                               sensor_event_s *event,
                               void *user_data);
  common::PlatformResult GetHrmData(picojson::value* data);

  // GPS
  location_manager_h location_handle_;
  JsonCallback gps_event_callback_;
  common::PlatformResult SetGpsListener(JsonCallback callback);
  common::PlatformResult UnsetGpsListener();
  static void OnGpsEvent(int num_of_location, void *user_data);
  common::PlatformResult GetGpsData(picojson::value* data);
};

} // namespace humanactivitymonitor
} // namespace extension

#endif	// HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H
