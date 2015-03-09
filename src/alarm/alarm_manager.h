// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALARM_ALARM_MANAGER_H_
#define ALARM_ALARM_MANAGER_H_

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace alarm {

class AlarmManager {
 public:
  static AlarmManager& GetInstance();
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
  AlarmManager();
  AlarmManager(const AlarmManager&) = delete;
  AlarmManager& operator=(const AlarmManager&) = delete;
  common::PlatformResult GetAlarm(int id, picojson::object& obj);

};

} // namespace alarm
} // namespace extension

#endif // ALARM_ALARM_MANAGER_H_
