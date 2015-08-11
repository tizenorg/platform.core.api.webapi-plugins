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
  virtual ~TimeInstance();

 private:
  enum DateTimeFormatType {
    TIME_FORMAT,
    DATE_FORMAT,
    DATE_SHORT_FORMAT,
    DATETIME_FORMAT
  };

  void TimeGetAvailableTimeZones(const JsonValue& args, JsonObject& out);
  void TimeGetDSTTransition(const JsonValue& args, JsonObject& out);
  void TimeGetLocalTimeZone(const JsonValue& args, JsonObject& out);
  void TimeGetTimeFormat(const JsonValue& args, JsonObject& out);
  void TimeGetDateFormat(const JsonValue& args, JsonObject& out);
  void TimeGetTimeZoneOffset(const JsonValue& args, JsonObject& out);
  void TimeGetTimeZoneAbbreviation(const JsonValue& args, JsonObject& out);
  void TimeIsDST(const JsonValue& args, JsonObject& out);
  void TimeToString(const JsonValue& args, JsonObject& out);
  void TimeToDateString(const JsonValue& args, JsonObject& out);
  void TimeToTimeString(const JsonValue& args, JsonObject& out);
  void TimeSetDateTimeChangeListener(const JsonValue& args, JsonObject& out);
  void TimeUnsetDateTimeChangeListener(const JsonValue& args, JsonObject& out);
  void TimeSetTimezoneChangeListener(const JsonValue& args, JsonObject& out);
  void TimeUnsetTimezoneChangeListener(const JsonValue& args, JsonObject& out);
  void TimeGetMsUTC(const JsonValue& args, JsonObject& out);

  Locale* getDefaultLocale();
  UnicodeString getDateTimeFormat(DateTimeFormatType type, bool bLocale);
  bool toStringByFormat(const JsonValue& args, JsonValue& out,
                        DateTimeFormatType format);
};
}  // namespace time
}  // namespace extension

#endif  // TIME_TIME_INSTANCE_H_
