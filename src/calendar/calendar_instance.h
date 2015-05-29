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

#ifndef CALENDAR_CALENDAR_INSTANCE_H_
#define CALENDAR_CALENDAR_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"

#include "calendar/calendar.h"

namespace extension {
namespace calendar {

class CalendarInstance : public common::ParsedInstance {
 public:
  CalendarInstance();
  virtual ~CalendarInstance();

 private:
  void CalendarGet(const picojson::value& args, picojson::object& out);
  void CalendarAdd(const picojson::value& args, picojson::object& out);
  void CalendarAddBatch(const picojson::value& args, picojson::object& out);
  void CalendarUpdate(const picojson::value& args, picojson::object& out);
  void CalendarUpdateBatch(const picojson::value& args, picojson::object& out);
  void CalendarRemove(const picojson::value& args, picojson::object& out);
  void CalendarRemoveBatch(const picojson::value& args, picojson::object& out);
  void CalendarFind(const picojson::value& args, picojson::object& out);
  void CalendarAddChangeListener(const picojson::value& args, picojson::object& out);
  void CalendarRemoveChangeListener(const picojson::value& args, picojson::object& out);

  void CalendarManagerAddCalendar(const picojson::value& args, picojson::object& out);
  void CalendarManagerGetCalendar(const picojson::value& args, picojson::object& out);
  void CalendarManagerGetCalendars(const picojson::value& args, picojson::object& out);
  void CalendarManagerRemoveCalendar(const picojson::value& args, picojson::object& out);

  Calendar calendar_;
};

} // namespace calendar
} // namespace extension

#endif // CALENDAR_CALENDAR_INSTANCE_H_
