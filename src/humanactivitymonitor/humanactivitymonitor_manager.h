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

#ifndef HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H
#define HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H

#include <functional>
#include <sensor.h>
#include <gesture_recognition.h>
#include <activity_recognition.h>
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

typedef struct HandleCallbackStr {
  HandleCallbackStr(long& id, JsonCallback& cb, activity_h& h) : watch_id(id), callback(cb), handle(h) {};
  long watch_id;
  JsonCallback callback;
  activity_h handle;
} HandleCallback;

class HumanActivityMonitorManager {
 public:
  HumanActivityMonitorManager();
  virtual ~HumanActivityMonitorManager();

  common::PlatformResult Init();

  common::PlatformResult SetListener(const std::string& type,
      JsonCallback callback, const picojson::value& args);
  common::PlatformResult UnsetListener(const std::string& type);

  common::PlatformResult AddActivityRecognitionListener(const std::string& type,
      JsonCallback callback, const picojson::value& args, long* watchId);
  common::PlatformResult RemoveActivityRecognitionListener(const long watchId);

  common::PlatformResult GetHumanActivityData(const std::string& type,
                                              picojson::value* data);

 private:
  // common
  common::PlatformResult IsSupported(const std::string& type);
  // WRIST_UP
  common::PlatformResult SetWristUpListener(JsonCallback callback);
  common::PlatformResult UnsetWristUpListener();
  static void OnWristUpEvent(gesture_type_e gesture,
                             const gesture_data_h data,
                             double timestamp,
                             gesture_error_e error,
                             void* user_data);
  // HRM
  common::PlatformResult SetHrmListener(JsonCallback callback, const picojson::value& args);
  common::PlatformResult UnsetHrmListener();
  static void OnHrmSensorEvent(sensor_h sensor,
                               sensor_event_s *event,
                               void *user_data);
  common::PlatformResult GetHrmData(picojson::value* data);
  // GPS
  common::PlatformResult SetGpsListener(JsonCallback callback, const picojson::value& args);
  common::PlatformResult UnsetGpsListener();
  static void OnGpsEvent(int num_of_location, void *user_data);
  common::PlatformResult GetGpsData(picojson::value* data);
  static void ActivityRecognitionCb(activity_type_e type, const activity_data_h data, double timestamp, activity_error_e callbackError, void *userData);

  // common
  std::map<std::string, bool> supported_;
  // WRIST_UP
  gesture_h gesture_handle_;
  JsonCallback wrist_up_event_callback_;
  // HRM
  sensor_listener_h hrm_sensor_listener_;
  JsonCallback hrm_event_callback_;
  // GPS
  location_manager_h location_handle_;
  JsonCallback gps_event_callback_;

  // Activity recognition listeners handle map
  std::map<long, HandleCallback*> handles_cb_;
};

} // namespace humanactivitymonitor
} // namespace extension

#endif	// HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H
