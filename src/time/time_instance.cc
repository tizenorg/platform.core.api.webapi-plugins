// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "time/time_instance.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace time {

using namespace common;

TimeInstance::TimeInstance()  : manager_(this) {
  using std::placeholders::_1;
  using std::placeholders::_2;

  LoggerD("Entered");

#define REGISTER_SYNC(c, x) \
        RegisterSyncHandler(c, std::bind(&TimeInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
        RegisterSyncHandler(c, std::bind(&TimeInstance::x, this, _1, _2));

  REGISTER_SYNC("TimeUtil_getAvailableTimezones", TimeUtil_getAvailableTimezones);
  REGISTER_SYNC("TimeUtil_getDateFormat", TimeUtil_getDateFormat);
  REGISTER_SYNC("TimeUtil_getTimeFormat", TimeUtil_getTimeFormat);
  REGISTER_SYNC("TimeUtil_setDateTimeChangeListener", TimeUtil_setDateTimeChangeListener);
  REGISTER_SYNC("TimeUtil_unsetDateTimeChangeListener", TimeUtil_unsetDateTimeChangeListener);
  REGISTER_SYNC("TimeUtil_setTimezoneChangeListener", TimeUtil_setTimezoneChangeListener);
  REGISTER_SYNC("TimeUtil_unsetTimezoneChangeListener", TimeUtil_unsetTimezoneChangeListener);
  REGISTER_SYNC("TZDate_getLocalTimezone", TZDate_getLocalTimezone);
  REGISTER_SYNC("TZDate_getTimezoneOffset", TZDate_GetTimezoneOffset);
  REGISTER_SYNC("TZDate_toLocaleDateString", TZDate_toLocaleDateString);
  REGISTER_SYNC("TZDate_toLocaleTimeString", TZDate_toLocaleTimeString);
  REGISTER_SYNC("TZDate_toLocaleString", TZDate_toLocaleString);
  REGISTER_SYNC("TZDate_toDateString", TZDate_toDateString);
  REGISTER_SYNC("TZDate_toTimeString", TZDate_toTimeString);
  REGISTER_SYNC("TZDate_toString", TZDate_toString);
  REGISTER_SYNC("TZDate_getTimezoneAbbreviation", TZDate_getTimezoneAbbreviation);
  REGISTER_SYNC("TZDate_isDST", TZDate_isDST);
  REGISTER_SYNC("TZDate_getPreviousDSTTransition", TZDate_getPreviousDSTTransition);
  REGISTER_SYNC("TZDate_getNextDSTTransition", TZDate_getNextDSTTransition);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

TimeInstance::~TimeInstance() {
  LoggerD("Entered");
}

void TimeInstance::TimeUtil_getAvailableTimezones(const picojson::value& /*args*/,
                                                  picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  auto array = result_obj.insert(std::make_pair("availableTimezones",
                                                picojson::value(picojson::array())));
  PlatformResult res = TimeUtilTools::GetAvailableTimezones(
      &array.first->second.get<picojson::array>());
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to get available timezones"));
    return;
  }
  ReportSuccess(result, out);
}

void TimeInstance::TimeUtil_getDateFormat(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  if (!args.contains("shortformat")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out,
                      ("Required parameter \"shortformat\" is missing"));
    return;
  }

  bool shortformat = args.get("shortformat").get<bool>();
  std::string format;
  PlatformResult res = TimeUtilTools::GetDateFormat(shortformat, &format);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to get date format."));
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(std::make_pair("format", picojson::value(format)));

  ReportSuccess(result, out);
}

void TimeInstance::TimeUtil_getTimeFormat(const picojson::value& /* args */,
                                          picojson::object& out) {
  LoggerD("Entered");
  std::string format;
  PlatformResult res = TimeUtilTools::GetTimeFormat(&format);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to get time format."));
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(std::make_pair("format", picojson::value(format)));

  ReportSuccess(result, out);
}

void TimeInstance::TimeUtil_setDateTimeChangeListener(const picojson::value& /*args*/,
                                                      picojson::object& out) {
  LoggerD("Entered");
  PlatformResult res = manager_.RegisterVconfCallback(kTimeChange);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to set date-time change listener."));
  }
  ReportSuccess(out);
}

void TimeInstance::TimeUtil_unsetDateTimeChangeListener(const picojson::value& /*args*/,
                                                        picojson::object& out) {
  LoggerD("Entered");
  PlatformResult res = manager_.UnregisterVconfCallback(kTimeChange);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to remove date-time change listener."));
  }
  ReportSuccess(out);
}

void TimeInstance::TimeUtil_setTimezoneChangeListener(const picojson::value& /*args*/,
                                                      picojson::object& out) {
  LoggerD("Entered");
  PlatformResult res = manager_.RegisterVconfCallback(kTimezoneChange);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to set timezone change listener."));
  }
  ReportSuccess(out);
}

void TimeInstance::TimeUtil_unsetTimezoneChangeListener(const picojson::value& /*args*/,
                                                        picojson::object& out) {
  LoggerD("Entered");
  PlatformResult res = manager_.UnregisterVconfCallback(kTimezoneChange);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to remove timezone change listener."));
  }
  ReportSuccess(out);
}

void TimeInstance::TZDate_getLocalTimezone(const picojson::value& /*args*/, picojson::object& out) {
  LoggerD("Entered");

  std::string local_timezone = TimeManager::GetDefaultTimezone();

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(std::make_pair("timezoneId", picojson::value(local_timezone)));

  ReportSuccess(result, out);
}

void TimeInstance::TZDate_GetTimezoneOffset(const picojson::value& args,
                                            picojson::object& out) {
  LoggerD("Entered");
  if (!args.contains("timezone") || !args.contains("timestamp")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out,
                      ("Required parameters are missing: \"timezone\", \"timestamp\""));
    return;
  }
  const std::string& timezone_id = args.get("timezone").get<std::string>();
  LoggerD("Getting timezone details for id: %s ", timezone_id.c_str());

  const std::string& timestamp_str = args.get("timestamp").get<std::string>();

  std::string offset;
  std::string modifier;
  PlatformResult res = manager_.GetTimezoneOffset(timezone_id, timestamp_str,
                                                  &offset, &modifier);
  if (res.IsSuccess()) {
    picojson::value result = picojson::value(picojson::object());
    picojson::object& result_obj = result.get<picojson::object>();
    result_obj.insert(std::make_pair("offset", picojson::value(offset)));
    //this value is to correct 'missing' hour also in JS
    result_obj.insert(std::make_pair("modifier", picojson::value(modifier)));
    ReportSuccess(result, out);
  } else {
    LogAndReportError(res, &out, ("Failed to get timezone offset."));
  }
}

void TimeInstance::ToStringTemplate(const picojson::value& args,
                                    bool use_locale_fmt,
                                    TimeUtilTools::DateTimeFormatType type,
                                    picojson::object* out) {
  LoggerD("Entered");
  if (!args.contains("timezone") || !args.contains("timestamp")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), out,
                      ("Required parameters are missing: \"timezone\", \"timestamp\""));
    return;
  }
  const std::string& timezone_id = args.get("timezone").get<std::string>();
  std::shared_ptr<UnicodeString> unicode_id (new UnicodeString(timezone_id.c_str()));
  LoggerD("Getting timezone details for id: %s ", timezone_id.c_str());

  const std::string& timestamp_str = args.get("timestamp").get<std::string>();
  UDate date = std::stod(timestamp_str);

  std::string result_string;
  PlatformResult res = TimeUtilTools::ToStringHelper(date, unicode_id, use_locale_fmt,
                                                     type, &result_string);
  if (res.IsError()) {
    LogAndReportError(res, out, ("Failed to convert to string."));
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(
      std::make_pair("string", picojson::value(result_string)));

  ReportSuccess(result, *out);
}

void TimeInstance::TZDate_toLocaleDateString(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  ToStringTemplate(args, true, TimeUtilTools::DateTimeFormatType::kDateFormat, &out);
}

void TimeInstance::TZDate_toLocaleTimeString(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  ToStringTemplate(args, true, TimeUtilTools::DateTimeFormatType::kTimeFormat, &out);
}

void TimeInstance::TZDate_toLocaleString(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  ToStringTemplate(args, true, TimeUtilTools::DateTimeFormatType::kDateTimeFormat, &out);
}

void TimeInstance::TZDate_toDateString(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  ToStringTemplate(args, false, TimeUtilTools::DateTimeFormatType::kDateFormat, &out);
}

void TimeInstance::TZDate_toTimeString(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  ToStringTemplate(args, false, TimeUtilTools::DateTimeFormatType::kTimeFormat, &out);
}

void TimeInstance::TZDate_toString(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  ToStringTemplate(args, false, TimeUtilTools::DateTimeFormatType::kDateTimeFormat, &out);
}

void TimeInstance::TZDate_getTimezoneAbbreviation(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Entered");
  if (!args.contains("timezone") || !args.contains("timestamp")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out,
                      ("Required parameters are missing: \"timezone\", \"timestamp\""));
    return;
  }
  const std::string& timezone_id = args.get("timezone").get<std::string>();
  std::shared_ptr<UnicodeString> unicode_id (new UnicodeString(timezone_id.c_str()));

  const std::string& timestamp_str = args.get("timestamp").get<std::string>();
  UDate date = std::stod(timestamp_str);

  std::string result_string;
  PlatformResult res = TimeUtilTools::GetTimezoneAbbreviation(date, unicode_id, &result_string);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to get timezone abbreviation."));
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(std::make_pair("abbreviation", picojson::value(result_string)));

  ReportSuccess(result, out);
}

void TimeInstance::TZDate_isDST(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  if (!args.contains("timezone") || !args.contains("timestamp")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out,
                      ("Required parameters are missing: \"timezone\", \"timestamp\""));
    return;
  }
  const std::string& timezone_id = args.get("timezone").get<std::string>();
  std::shared_ptr<UnicodeString> unicode_id (new UnicodeString(timezone_id.c_str()));

  const std::string& timestamp_str = args.get("timestamp").get<std::string>();
  UDate date = std::stod(timestamp_str);

  bool is_dst = false;
  PlatformResult res = TimeUtilTools::IsDST(date, unicode_id, &is_dst);
  if (res.IsError()) {
    LogAndReportError(res, &out, ("Failed to check DST."));
    return;
  }
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  result_obj.insert(
      std::make_pair("isDST", picojson::value(is_dst)));
  ReportSuccess(result, out);
}

void TimeInstance::TZDate_getPreviousDSTTransition(const picojson::value& args,
                                                   picojson::object& out) {
  LoggerD("Entered");
  if (!args.contains("timezone") || !args.contains("timestamp")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out,
                      ("Required parameters are missing: \"timezone\", \"timestamp\""));
    return;
  }
  const std::string& timezone_id = args.get("timezone").get<std::string>();
  std::shared_ptr<UnicodeString> unicode_id (new UnicodeString(timezone_id.c_str()));

  const std::string& timestamp_str = args.get("timestamp").get<std::string>();
  UDate date = std::stod(timestamp_str);

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  UDate prev_dst = TimeUtilTools::GetDSTTransition(date, unicode_id,
                                                   TimeUtilTools::DSTTransition::kPreviousDST);
  result_obj.insert(std::make_pair("prevDSTDate", picojson::value(prev_dst)));

  ReportSuccess(result, out);
}

void TimeInstance::TZDate_getNextDSTTransition(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  if (!args.contains("timezone") || !args.contains("timestamp")) {
    LogAndReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid parameter passed."), &out,
                      ("Required parameters are missing: \"timezone\", \"timestamp\""));
    return;
  }
  const std::string& timezone_id = args.get("timezone").get<std::string>();
  std::shared_ptr<UnicodeString> unicode_id (new UnicodeString(timezone_id.c_str()));

  const std::string& timestamp_str = args.get("timestamp").get<std::string>();
  UDate date = std::stod(timestamp_str);

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  UDate next_dst = TimeUtilTools::GetDSTTransition(date, unicode_id,
                                                   TimeUtilTools::DSTTransition::kNextDST);
  result_obj.insert(std::make_pair("nextDSTDate", picojson::value(next_dst)));

  ReportSuccess(result, out);
}

}  // namespace time
}  // namespace extension
