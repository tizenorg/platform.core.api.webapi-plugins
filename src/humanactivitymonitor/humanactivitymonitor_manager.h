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
#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace humanactivitymonitor {

extern const std::string kActivityTypeGps;

using JsonCallback = std::function<void(picojson::value*)>;

class HumanActivityMonitorManager {
 public:
  HumanActivityMonitorManager();
  virtual ~HumanActivityMonitorManager();

  common::PlatformResult Init();

  common::PlatformResult SetListener(const std::string& type,
                                     JsonCallback callback,
                                     const picojson::value& args);
  common::PlatformResult UnsetListener(const std::string& type);
  common::PlatformResult GetHumanActivityData(const std::string& type,
                                              picojson::value* data);

  common::PlatformResult AddActivityRecognitionListener(const std::string& type,
                                                        JsonCallback callback,
                                                        long* watch_id);
  common::PlatformResult RemoveActivityRecognitionListener(const long watchId);
  common::PlatformResult StartDataRecorder(const std::string& type,
                                           int interval, int retention_period);
  common::PlatformResult StopDataRecorder(const std::string& type);
  common::PlatformResult ReadRecorderData(const std::string& type,
                                          picojson::array* data,
                                          const picojson::value& query);

 private:
  class Monitor;
  class ActivityRecognition;

  std::shared_ptr<Monitor> GetMonitor(const std::string& type);

  std::map<std::string, std::shared_ptr<Monitor>> monitors_;
  std::shared_ptr<ActivityRecognition> activity_recognition_;
};

} // namespace humanactivitymonitor
} // namespace extension

#endif	// HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_MANAGER_H
