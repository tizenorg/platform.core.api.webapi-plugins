// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TIME_TIME_INSTANCE_H_
#define TIME_TIME_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"
#include "unicode/unistr.h"

#include <string>

namespace extension {
namespace time {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

class TimeInstance : public common::ParsedInstance {
 public:
  TimeInstance();
  static TimeInstance& GetInstance();
  virtual ~TimeInstance();

 private:
  enum DateTimeFormatType {
    TIME_FORMAT,
    DATE_FORMAT,
    DATE_SHORT_FORMAT,
    DATETIME_FORMAT
  };

  void Time_getAvailableTimeZones(const JsonValue& args, JsonObject& out);
  void Time_getDSTTransition(const JsonValue& args, JsonObject& out);
  void Time_getLocalTimeZone(const JsonValue& args, JsonObject& out);
  void Time_getTimeFormat(const JsonValue& args, JsonObject& out);
  void Time_getDateFormat(const JsonValue& args, JsonObject& out);
  void Time_getTimeZoneOffset(const JsonValue& args, JsonObject& out);
  void Time_getTimeZoneAbbreviation(const JsonValue& args, JsonObject& out);
  void Time_isDST(const JsonValue& args, JsonObject& out);
  void Time_toString(const JsonValue& args, JsonObject& out);
  void Time_toDateString(const JsonValue& args, JsonObject& out);
  void Time_toTimeString(const JsonValue& args, JsonObject& out);
  void Time_setDateTimeChangeListener(const JsonValue& args, JsonObject& out);
  void Time_unsetDateTimeChangeListener(const JsonValue& args, JsonObject& out);
  void Time_setTimezoneChangeListener(const JsonValue& args, JsonObject& out);
  void Time_unsetTimezoneChangeListener(const JsonValue& args, JsonObject& out);

  UnicodeString getDateTimeFormat(DateTimeFormatType type, bool bLocale);
  bool toStringByFormat(const JsonValue& args, JsonValue& out,
                        DateTimeFormatType format);
};
}  // namespace time
}  // namespace extension

#endif  // TIME_TIME_INSTANCE_H_
