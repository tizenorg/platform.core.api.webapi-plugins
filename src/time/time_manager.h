/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef TIME_TIME_MANAGER_H_
#define TIME_TIME_MANAGER_H_

#include <memory>
#include <vconf.h>
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace time {

enum ListenerType {
  kTimeChange,
  kTimezoneChange
};

class TimeInstance;

class TimeManager
{
 public:
  TimeManager(TimeInstance* instance);
  ~TimeManager();

  common::PlatformResult GetTimezoneOffset(const std::string& timezone_id,
                                           const std::string& timestamp_str,
                                           std::string* offset,
                                           std::string* modifier);
  common::PlatformResult RegisterVconfCallback(ListenerType type);
  common::PlatformResult UnregisterVconfCallback(ListenerType type);
  static void OnTimeChangedCallback(keynode_t* node, void* event_ptr);
  std::string GetCurrentTimezone();
  void SetCurrentTimezone(const std::string& new_timezone);
  TimeInstance* GetTimeInstance();
  static std::string GetDefaultTimezone();
 private:
  TimeInstance* instance_;
  std::string current_timezone_;
  bool is_time_listener_registered_;
  bool is_timezone_listener_registered_;
};

}
}

#endif // TIME_TIME_MANAGER_H_
