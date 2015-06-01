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

#ifndef ALARM_ALARM_MANAGER_H_
#define ALARM_ALARM_MANAGER_H_

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace alarm {

class AlarmManager {
 public:
  AlarmManager();
  virtual ~AlarmManager();

  void Add(const picojson::value& args, picojson::object& out);
  void Remove(const picojson::value& args, picojson::object& out);
  void RemoveAll(const picojson::value& args, picojson::object& out);
  void Get(const picojson::value& args, picojson::object& out);
  void GetAll(const picojson::value& args, picojson::object& out);

  //AlarmRelative
  void GetRemainingSeconds(const picojson::value& args, picojson::object& out);
  //AlarmAbsolute
  void GetNextScheduledDate(const picojson::value& args, picojson::object& out);

 private:
  AlarmManager(const AlarmManager&) = delete;
  AlarmManager& operator=(const AlarmManager&) = delete;
  common::PlatformResult GetAlarm(int id, picojson::object& obj);

};

} // namespace alarm
} // namespace extension

#endif // ALARM_ALARM_MANAGER_H_
