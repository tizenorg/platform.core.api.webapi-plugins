// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TIME_TIME_INSTANCE_H_
#define TIME_TIME_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"
#include "unicode/unistr.h"
#include "time/time_manager.h"
#include "time/time_utils.h"

#include <string>

namespace extension {
namespace time {

class TimeInstance : public common::ParsedInstance {
 public:
  TimeInstance();
  virtual ~TimeInstance();

 private:
  void TimeUtil_getAvailableTimezones(const picojson::value& args, picojson::object& out);
  void TimeUtil_getDateFormat(const picojson::value& args, picojson::object& out);
  void TimeUtil_getTimeFormat(const picojson::value& args, picojson::object& out);
  void TimeUtil_setDateTimeChangeListener(const picojson::value& args, picojson::object& out);
  void TimeUtil_unsetDateTimeChangeListener(const picojson::value& args, picojson::object& out);
  void TimeUtil_setTimezoneChangeListener(const picojson::value& args, picojson::object& out);
  void TimeUtil_unsetTimezoneChangeListener(const picojson::value& args, picojson::object& out);
  void TZDate_getTimezone(const picojson::value& args, picojson::object& out);
  void TZDate_GetTimezoneOffset(const picojson::value& args, picojson::object& out);
  void ToStringTemplate(const picojson::value& args, bool use_locale_fmt,
                        TimeUtilTools::DateTimeFormatType type, picojson::object* out);
  void TZDate_toLocaleDateString(const picojson::value& args, picojson::object& out);
  void TZDate_toLocaleTimeString(const picojson::value& args, picojson::object& out);
  void TZDate_toLocaleString(const picojson::value& args, picojson::object& out);
  void TZDate_toDateString(const picojson::value& args, picojson::object& out);
  void TZDate_toTimeString(const picojson::value& args, picojson::object& out);
  void TZDate_toString(const picojson::value& args, picojson::object& out);
  void TZDate_getTimezoneAbbreviation(const picojson::value& args, picojson::object& out);
  void TZDate_isDST(const picojson::value& args, picojson::object& out);
  void TZDate_getPreviousDSTTransition(const picojson::value& args, picojson::object& out);
  void TZDate_getNextDSTTransition(const picojson::value& args, picojson::object& out);

  TimeManager manager_;
};
}  // namespace time
}  // namespace extension

#endif  // TIME_TIME_INSTANCE_H_
