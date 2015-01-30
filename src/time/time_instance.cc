// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "time/time_instance.h"
#include "common/platform_exception.h"
#include "common/logger.h"

#if defined(TIZEN)
#include <vconf.h>
#endif

#include <sstream>
#include <memory>
#include <cerrno>
#include <unistd.h>

#include "common/picojson.h"

#include "unicode/timezone.h"
#include "unicode/calendar.h"
#include "unicode/vtzone.h"
#include "unicode/tztrans.h"
#include "unicode/smpdtfmt.h"
#include "unicode/dtptngen.h"

namespace extension {
namespace time {

using namespace common;

enum ListenerType {
  kTimeChange,
  kTimezoneChange
};

namespace {

const int _hourInMilliseconds = 3600000;
const char kTimezoneListenerId[] = "TimezoneChangeListener";
const char kDateTimeListenerId[] = "DateTimeChangeListener";

}  // namespace

TimeInstance& TimeInstance::GetInstance() {
  static TimeInstance instance;
  return instance;
}

TimeInstance::TimeInstance() {
  using namespace std::placeholders;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&TimeInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&TimeInstance::x, this, _1, _2));

  REGISTER_SYNC("Time_getAvailableTimeZones", TimeGetAvailableTimeZones);
  REGISTER_SYNC("Time_getDSTTransition", TimeGetDSTTransition);
  REGISTER_SYNC("Time_getLocalTimeZone", TimeGetLocalTimeZone);
  REGISTER_SYNC("Time_getTimeFormat", TimeGetTimeFormat);
  REGISTER_SYNC("Time_getDateFormat", TimeGetDateFormat);
  REGISTER_SYNC("Time_getTimeZoneOffset", TimeGetTimeZoneOffset);
  REGISTER_SYNC("Time_getTimeZoneAbbreviation", TimeGetTimeZoneAbbreviation);
  REGISTER_SYNC("Time_isDST", TimeIsDST);
  REGISTER_SYNC("Time_toString", TimeToString);
  REGISTER_SYNC("Time_toDateString", TimeToDateString);
  REGISTER_SYNC("Time_toTimeString", TimeToTimeString);
  REGISTER_SYNC("Time_setDateTimeChangeListener",
                TimeSetDateTimeChangeListener);
  REGISTER_SYNC("Time_unsetDateTimeChangeListener",
                TimeUnsetDateTimeChangeListener);
  REGISTER_SYNC("Time_setTimezoneChangeListener",
                TimeSetTimezoneChangeListener);
  REGISTER_SYNC("Time_unsetTimezoneChangeListener",
                TimeUnsetTimezoneChangeListener);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

static void OnTimeChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/);
static std::string GetDefaultTimezone();

TimeInstance::~TimeInstance() {}

void TimeInstance::TimeGetLocalTimeZone(const JsonValue& /*args*/,
                                        JsonObject& out) {
  UnicodeString local_timezone;
  TimeZone::createDefault()->getID(local_timezone);

  std::string localtz;
  local_timezone.toUTF8String(localtz);

  ReportSuccess(JsonValue(localtz), out);
}

void TimeInstance::TimeGetAvailableTimeZones(const JsonValue& /*args*/,
                                             JsonObject& out) {
  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<StringEnumeration> timezones(TimeZone::createEnumeration());
  int32_t count = timezones->count(ec);
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  JsonArray a;
  const char* timezone = NULL;
  int i = 0;
  do {
    int32_t resultLen = 0;
    timezone = timezones->next(&resultLen, ec);
    if (U_SUCCESS(ec)) {
      a.push_back(JsonValue(timezone));
      i++;
    }
  } while (timezone && i < count);

  ReportSuccess(JsonValue(a), out);
}

void TimeInstance::TimeGetTimeZoneOffset(const JsonValue& args,
                                         JsonObject& out) {
  std::unique_ptr<UnicodeString> id(
      new UnicodeString(args.get("timezone").to_str().c_str()));
  UDate dateInMs = strtod(args.get("value").to_str().c_str(), NULL);

  if (errno == ERANGE) {
    ReportError(out);
    return;
  }

  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<TimeZone> timezone(TimeZone::createTimeZone(*id));
  std::unique_ptr<Calendar> cal(Calendar::createInstance(*timezone, ec));
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  cal->setTime(dateInMs, ec);
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  int32_t offset = timezone->getRawOffset();

  if (cal->inDaylightTime(ec)) offset += _hourInMilliseconds;

  std::stringstream offsetStr;
  offsetStr << offset;

  ReportSuccess(JsonValue(offsetStr.str()), out);
}

void TimeInstance::TimeGetTimeZoneAbbreviation(const JsonValue& args,
                                               JsonObject& out) {
  std::unique_ptr<UnicodeString> id(
      new UnicodeString(args.get("timezone").to_str().c_str()));
  UDate dateInMs = strtod(args.get("value").to_str().c_str(), NULL);

  if (errno == ERANGE) {
    ReportError(out);
    return;
  }

  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<Calendar> cal(
      Calendar::createInstance(TimeZone::createTimeZone(*id), ec));
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  cal->setTime(dateInMs, ec);
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  std::unique_ptr<DateFormat> fmt(
      new SimpleDateFormat(UnicodeString("z"), Locale::getEnglish(), ec));
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  UnicodeString uAbbreviation;
  fmt->setCalendar(*cal);
  fmt->format(cal->getTime(ec), uAbbreviation);
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  std::string abbreviation = "";
  uAbbreviation.toUTF8String(abbreviation);

  ReportSuccess(JsonValue(abbreviation), out);
}

void TimeInstance::TimeIsDST(const JsonValue& args, JsonObject& out) {
  std::unique_ptr<UnicodeString> id(
      new UnicodeString(args.get("timezone").to_str().c_str()));
  UDate dateInMs = strtod(args.get("value").to_str().c_str(), NULL);
  dateInMs -= _hourInMilliseconds;

  if (errno == ERANGE) {
    ReportError(out);
    return;
  }

  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<Calendar> cal(
      Calendar::createInstance(TimeZone::createTimeZone(*id), ec));
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  cal->setTime(dateInMs, ec);
  if (U_FAILURE(ec)) {
    ReportError(out);
    return;
  }

  ReportSuccess(JsonValue{static_cast<bool>(cal->inDaylightTime(ec))}, out);
}

void TimeInstance::TimeGetDSTTransition(const JsonValue& args,
                                        JsonObject& out) {
  std::unique_ptr<UnicodeString> id(
      new UnicodeString(args.get("timezone").to_str().c_str()));
  std::string trans = args.get("trans").to_str();
  UDate dateInMs = strtod(args.get("value").to_str().c_str(), NULL);

  if (errno == ERANGE) {
    ReportError(out);
    return;
  }

  std::unique_ptr<VTimeZone> vtimezone(VTimeZone::createVTimeZoneByID(*id));

  if (!vtimezone->useDaylightTime()) {
    ReportError(out);
    return;
  }

  TimeZoneTransition tzTransition;
  if (trans.compare("NEXT_TRANSITION") &&
      vtimezone->getNextTransition(dateInMs, FALSE, tzTransition))
    ReportSuccess(JsonValue{tzTransition.getTime()}, out);
  else if (vtimezone->getPreviousTransition(dateInMs, FALSE, tzTransition))
    ReportSuccess(JsonValue{tzTransition.getTime()}, out);
  else
    ReportError(out);
}

void TimeInstance::TimeToString(const JsonValue& args, JsonObject& out) {
  JsonValue val;
  if (!this->toStringByFormat(args, val, TimeInstance::DATETIME_FORMAT)) {
    ReportError(out);
    return;
  }

  ReportSuccess(val, out);
}

void TimeInstance::TimeToDateString(const JsonValue& args, JsonObject& out) {
  JsonValue val;
  if (!this->toStringByFormat(args, val, TimeInstance::DATE_FORMAT)) {
    ReportError(out);
    return;
  }

  ReportSuccess(val, out);
}

void TimeInstance::TimeToTimeString(const JsonValue& args, JsonObject& out) {
  JsonValue val;
  if (!this->toStringByFormat(args, val, TimeInstance::TIME_FORMAT)) {
    ReportError(out);
    return;
  }

  ReportSuccess(val, out);
}

bool TimeInstance::toStringByFormat(const JsonValue& args, JsonValue& out,
                                    DateTimeFormatType format) {
  std::unique_ptr<UnicodeString> id(
      new UnicodeString(args.get("timezone").to_str().c_str()));
  bool bLocale = args.get("locale").evaluate_as_boolean();

  UDate dateInMs = strtod(args.get("value").to_str().c_str(), NULL);
  if (errno == ERANGE) return false;

  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<Calendar> cal(
      Calendar::createInstance(TimeZone::createTimeZone(*id), ec));
  if (U_FAILURE(ec)) return false;

  cal->setTime(dateInMs, ec);
  if (U_FAILURE(ec)) return false;

  std::unique_ptr<DateFormat> fmt(new SimpleDateFormat(
      getDateTimeFormat(format, bLocale),
      (bLocale ? Locale::getDefault() : Locale::getEnglish()), ec));
  if (U_FAILURE(ec)) return false;

  UnicodeString uResult;
  fmt->setCalendar(*cal);
  fmt->format(cal->getTime(ec), uResult);
  if (U_FAILURE(ec)) return false;

  std::string result = "";
  uResult.toUTF8String(result);

  out = JsonValue(result);
  return true;
}

void TimeInstance::TimeGetTimeFormat(const JsonValue& /*args*/,
                                     JsonObject& out) {
  UnicodeString timeFormat = getDateTimeFormat(TimeInstance::TIME_FORMAT, true);

  timeFormat = timeFormat.findAndReplace("H", "h");
  timeFormat = timeFormat.findAndReplace("a", "ap");

  timeFormat = timeFormat.findAndReplace("hh", "h");
  timeFormat = timeFormat.findAndReplace("mm", "m");
  timeFormat = timeFormat.findAndReplace("ss", "s");

  std::string result = "";
  timeFormat.toUTF8String(result);

  ReportSuccess(JsonValue(result), out);
}

void TimeInstance::TimeGetDateFormat(const JsonValue& args, JsonObject& out) {
  bool shortformat = args.get("shortformat").evaluate_as_boolean();

  UnicodeString time_format =
      getDateTimeFormat((shortformat ? DateTimeFormatType::DATE_SHORT_FORMAT
                                     : DateTimeFormatType::DATE_FORMAT),
                        true);

  time_format = time_format.findAndReplace("E", "D");

  if (time_format.indexOf("MMM") > 0) {
    if (time_format.indexOf("MMMM") > 0) {
      time_format = time_format.findAndReplace("MMMM", "M");
    } else {
      time_format = time_format.findAndReplace("MMM", "M");
    }
  } else {
    time_format = time_format.findAndReplace("M", "m");
  }

  int i = 0;
  while (i < time_format.length() - 1) {
    if (time_format[i] == time_format[i + 1]) {
      time_format.remove(i, 1);
    } else {
      ++i;
    }
  }

  std::string result = "";
  time_format.toUTF8String(result);
  ReportSuccess(JsonValue(result), out);
}

UnicodeString TimeInstance::getDateTimeFormat(DateTimeFormatType type,
                                              bool bLocale) {
  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<DateTimePatternGenerator> dateTimepattern(
      DateTimePatternGenerator::createInstance(
          (bLocale ? Locale::getDefault() : Locale::getEnglish()), ec));

  if (U_FAILURE(ec)) return "";

  UnicodeString pattern;
  if (type == DATE_FORMAT) {
    pattern = dateTimepattern->getBestPattern(UDAT_YEAR_MONTH_WEEKDAY_DAY, ec);
  } else if (type == DATE_SHORT_FORMAT) {
    pattern = dateTimepattern->getBestPattern(UDAT_YEAR_NUM_MONTH_DAY, ec);
  } else {
    std::string skeleton;
    if (type != TIME_FORMAT) skeleton = UDAT_YEAR_MONTH_WEEKDAY_DAY;

#if defined(TIZEN)
    int ret = 0;
    int value = 0;
    ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &value);
    // if failed, set default time format
    if (-1 == ret) {
      value = VCONFKEY_TIME_FORMAT_12;
    }
    if (VCONFKEY_TIME_FORMAT_12 == value) {
      skeleton += "hhmmss";
    } else {
      skeleton += "HHmmss";
    }
#else
    skeleton += "hhmmss";
#endif

    pattern = dateTimepattern->getBestPattern(
        *(new UnicodeString(skeleton.c_str())), ec);
    if (U_FAILURE(ec)) return "";

    if (!bLocale) pattern += " 'GMT'Z v'";
  }

  return pattern;
}

/////////////////////////// TimeUtilListeners ////////////////////////////////

class TimeUtilListeners {
 public:
  TimeUtilListeners();
  ~TimeUtilListeners();

  void RegisterVconfCallback(ListenerType type);
  void UnregisterVconfCallback(ListenerType type);

  std::string GetCurrentTimezone();
  void SetCurrentTimezone(std::string& newTimezone);

 private:
  std::string current_timezone_;
  bool is_time_listener_registered_;
  bool is_timezone_listener_registered_;
};

TimeUtilListeners::TimeUtilListeners()
    : current_timezone_(GetDefaultTimezone()),
      is_time_listener_registered_(false),
      is_timezone_listener_registered_(false) {
  LoggerD("Entered");
}

TimeUtilListeners::~TimeUtilListeners() {
  LoggerD("Entered");
  if (is_time_listener_registered_ || is_timezone_listener_registered_) {
    if (0 != vconf_ignore_key_changed(VCONFKEY_SYSTEM_TIME_CHANGED,
                                      OnTimeChangedCallback)) {
      LoggerE("Failed to unregister vconf callback");
    }
  }
}

void TimeUtilListeners::RegisterVconfCallback(ListenerType type) {
  LoggerD("");
  if (!is_time_listener_registered_ && !is_timezone_listener_registered_) {
    LoggerD("registering listener on platform");
    if (0 != vconf_notify_key_changed(VCONFKEY_SYSTEM_TIME_CHANGED,
                                      OnTimeChangedCallback, nullptr)) {
      LoggerE("Failed to register vconf callback");
      throw UnknownException("Failed to register vconf callback");
    }
  } else {
    LoggerD("not registering listener on platform - already registered");
  }
  switch (type) {
    case kTimeChange:
      is_time_listener_registered_ = true;
      LoggerD("time change listener registered");
      break;
    case kTimezoneChange:
      is_timezone_listener_registered_ = true;
      LoggerD("time zone change listener registered");
      break;
    default:
      LoggerE("Unknown type of listener");
      throw UnknownException("Unknown type of listener");
  }
}

void TimeUtilListeners::UnregisterVconfCallback(ListenerType type) {
  LoggerD("");
  switch (type) {
    case kTimeChange:
      is_time_listener_registered_ = false;
      LoggerD("time change listener unregistered");
      break;
    case kTimezoneChange:
      is_timezone_listener_registered_ = false;
      LoggerD("time zone change listener unregistered");
      break;
    default:
      throw UnknownException("Unknown type of listener");
  }
  if (!is_time_listener_registered_ && !is_timezone_listener_registered_) {
    LoggerD("unregistering listener on platform");
    if (0 != vconf_ignore_key_changed(VCONFKEY_SYSTEM_TIME_CHANGED,
                                      OnTimeChangedCallback)) {
      LoggerE("Failed to unregister vconf callback");
    }
  }
}

std::string TimeUtilListeners::GetCurrentTimezone() {
  return current_timezone_;
}

void TimeUtilListeners::SetCurrentTimezone(std::string& newTimezone) {
  current_timezone_ = newTimezone;
}

static std::string GetDefaultTimezone() {
  LoggerD("");
  char buf[1024];
  std::string result;
  ssize_t len = readlink("/opt/etc/localtime", buf, sizeof(buf) - 1);
  if (len != -1) {
    buf[len] = '\0';
  } else {
    /* handle error condition */
    return result;
  }
  result = std::string(buf + strlen("/usr/share/zoneinfo/"));

  LoggerD("tzpath = %s", result.c_str());
  return result;
}

/////////////////////////// TimeUtilListeners object ////////////////////////
static TimeUtilListeners g_time_util_listeners_obj;

static void PostMessage(const char* message) {
  JsonValue result{JsonObject{}};
  JsonObject& result_obj = result.get<JsonObject>();
  result_obj.insert(std::make_pair("listenerId", picojson::value(message)));
  TimeInstance::GetInstance().PostMessage(result.serialize().c_str());
}

static void OnTimeChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/) {
  LoggerD("");
  std::string defaultTimezone = GetDefaultTimezone();

  if (g_time_util_listeners_obj.GetCurrentTimezone() != defaultTimezone) {
    g_time_util_listeners_obj.SetCurrentTimezone(defaultTimezone);
    PostMessage(kTimezoneListenerId);
  }
  PostMessage(kDateTimeListenerId);
}

void TimeInstance::TimeSetDateTimeChangeListener(const JsonValue& /*args*/,
                                                 JsonObject& out) {
  g_time_util_listeners_obj.RegisterVconfCallback(kTimeChange);
  ReportSuccess(out);
}

void TimeInstance::TimeUnsetDateTimeChangeListener(const JsonValue& /*args*/,
                                                   JsonObject& out) {
  g_time_util_listeners_obj.UnregisterVconfCallback(kTimeChange);
  ReportSuccess(out);
}

void TimeInstance::TimeSetTimezoneChangeListener(const JsonValue& /*args*/,
                                                 JsonObject& out) {
  g_time_util_listeners_obj.RegisterVconfCallback(kTimezoneChange);
  ReportSuccess(out);
}

void TimeInstance::TimeUnsetTimezoneChangeListener(const JsonValue& /*args*/,
                                                   JsonObject& out) {
  g_time_util_listeners_obj.UnregisterVconfCallback(kTimezoneChange);
  ReportSuccess(out);
}

}  // namespace time
}  // namespace extension
