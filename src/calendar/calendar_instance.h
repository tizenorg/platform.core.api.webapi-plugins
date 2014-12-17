// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALENDAR_CALENDAR_INSTANCE_H_
#define CALENDAR_CALENDAR_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"

namespace extension {
namespace calendar {

class CalendarInstance : public common::ParsedInstance {
 public:
  CalendarInstance();
  virtual ~CalendarInstance();

 private:
  void Calendar_get(const picojson::value& args, picojson::object& out);
  void Calendar_add(const picojson::value& args, picojson::object& out);
  void Calendar_addBatch(const picojson::value& args, picojson::object& out);
  void Calendar_update(const picojson::value& args, picojson::object& out);
  void Calendar_updateBatch(const picojson::value& args, picojson::object& out);
  void Calendar_remove(const picojson::value& args, picojson::object& out);
  void Calendar_removeBatch(const picojson::value& args, picojson::object& out);
  void Calendar_find(const picojson::value& args, picojson::object& out);
  void Calendar_addChangeListener(const picojson::value& args, picojson::object& out);
  void Calendar_removeChangeListener(const picojson::value& args, picojson::object& out);

  void CalendarManager_addCalendar(const picojson::value& args, picojson::object& out);
  void CalendarManager_getCalendar(const picojson::value& args, picojson::object& out);
  void CalendarManager_getCalendars(const picojson::value& args, picojson::object& out);
  void CalendarManager_removeCalendar(const picojson::value& args, picojson::object& out);

};

} // namespace calendar
} // namespace extension

#endif // CALENDAR_CALENDAR_INSTANCE_H_
