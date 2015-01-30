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
  static CalendarInstance& GetInstance();

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

};

} // namespace calendar
} // namespace extension

#endif // CALENDAR_CALENDAR_INSTANCE_H_
