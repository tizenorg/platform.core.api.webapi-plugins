/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "time_utils.h"

#include <unicode/dtptngen.h>
#include <unicode/smpdtfmt.h>
#include <unicode/timezone.h>
#include <unicode/vtzone.h>

#include <vconf.h>

#include "common/logger.h"

using common::PlatformResult;
using common::ErrorCode;

namespace extension {
namespace time {

UDate TimeUtilTools::GetDSTTransition(UDate timestamp,
                                      const std::shared_ptr<UnicodeString>& timezone_id,
                                      DSTTransition tr_type) {
  LoggerD("Entered");
  UBool result = false;
  UDate dst_transition_date = timestamp;
  std::unique_ptr<VTimeZone> vtz(VTimeZone::createVTimeZoneByID(*timezone_id));

  TimeZoneTransition tz_trans;
  if (vtz->useDaylightTime()) {
    switch (tr_type) {
      case DSTTransition::kNextDST:
        result = vtz->getNextTransition(dst_transition_date, FALSE, tz_trans);
        break;

      case DSTTransition::kPreviousDST:
        result = vtz->getPreviousTransition(dst_transition_date, FALSE, tz_trans);
        break;
    }

    if (result) {
      dst_transition_date = tz_trans.getTime();
    }
  }
  return dst_transition_date;
}

PlatformResult TimeUtilTools::IsDST(UDate timestamp,
                                    const std::shared_ptr<UnicodeString>& timezone_id,
                                    bool* result_bool) {
  LoggerD("Entered");
  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<TimeZone> tz (TimeZone::createTimeZone(*timezone_id));
  std::unique_ptr<icu::Calendar> calendar (Calendar::createInstance(*tz, ec));

  if (U_FAILURE(ec)){
    LoggerE("Failed to create calendar instance: %d", ec);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to create calendar instance");
  }
  calendar->setTime(timestamp, ec);
  if (U_FAILURE(ec)){
    LoggerE("Failed to set calendar date: %d", ec);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to set calendar date");
  }
  bool result = static_cast<bool>(calendar->inDaylightTime(ec));
  if (U_FAILURE(ec)){
    LoggerE("Failed to get day light boolean: %d", ec);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get day light boolean");
  }
  *result_bool = result;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult TimeUtilTools::GetTimezoneAbbreviation(UDate date,
                                                      const std::shared_ptr<UnicodeString>& timezone_id,
                                                      std::string* result_string) {
  LoggerD("Entered");
  UErrorCode ec = U_ZERO_ERROR;
  UnicodeString str;

  std::unique_ptr<DateFormat> fmt(new SimpleDateFormat(UnicodeString("z"),
                                                       Locale::getEnglish(), ec));
  if (U_SUCCESS(ec)) {
    std::unique_ptr<TimeZone> tz(TimeZone::createTimeZone(*timezone_id));
    fmt->setTimeZone(*tz);
    fmt->format(date, str);

    if ((str.length() > 3) && (str.compare(0, 3, "GMT") == 0)) {
      str.remove();
      std::unique_ptr<DateFormat> gmt(new SimpleDateFormat(UnicodeString("OOOO"),
                                                           Locale::getEnglish(), ec));
      gmt->setTimeZone(*tz);
      gmt->format(date, str);
    }

    return TimeUtilTools::ToUTF8String(str, result_string);
  }
  LOGE("can't make SimpleDateFormat or can't get time");
  return PlatformResult(ErrorCode::UNKNOWN_ERR,
                        "can't make SimpleDateFormat or can't get time");
}

PlatformResult TimeUtilTools::ToStringHelper(UDate date,
                                             std::shared_ptr<UnicodeString>& timezone_id,
                                             bool use_locale_fmt,
                                             TimeUtilTools::DateTimeFormatType type,
                                             std::string* result_string) {
  LoggerD("Entered");
  UErrorCode ec = U_ZERO_ERROR;
  UnicodeString str;

  std::unique_ptr<Locale> default_locale(TimeUtilTools::GetDefaultLocale());
  std::unique_ptr<DateFormat> fmt(
      new SimpleDateFormat(
          TimeUtilTools::GetDateTimeFormat(type, use_locale_fmt),
          ((use_locale_fmt && default_locale != nullptr) ? *default_locale : Locale::getEnglish()),
          ec));

  if (U_SUCCESS(ec)) {
    std::unique_ptr<TimeZone> tz(TimeZone::createTimeZone(*timezone_id));

    fmt->setTimeZone(*tz);
    fmt->format(date, str);

    return TimeUtilTools::ToUTF8String(str, result_string);
  }

  LOGE("can't make SimpleDateFormat or can't get time");
  return PlatformResult(ErrorCode::UNKNOWN_ERR, "can't make SimpleDateFormat or can't get time");
}

PlatformResult TimeUtilTools::ToUTF8String(const UnicodeString& uni_str,
                                           std::string* result_string) {
  LoggerD("Entered");
  int buffer_len = sizeof(UChar) * static_cast<int>(uni_str.length()) + 1;

  std::unique_ptr<char, void(*)(void*)> result_buffer(static_cast<char*>(malloc(buffer_len)),
                                                      &std::free);

  if (!result_buffer) {
    LOGE("memory allocation error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "memory allocation error");
  }

  memset(result_buffer.get(), 0, buffer_len);
  CheckedArrayByteSink sink(result_buffer.get(), buffer_len);
  uni_str.toUTF8(sink);

  if (sink.Overflowed()) {
    LOGE("Converting error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Converting error");
  }

  *result_string = result_buffer.get();
  return PlatformResult(ErrorCode::NO_ERROR);
}

UnicodeString* TimeUtilTools::ToUnicodeString(const std::string& str) {
  LoggerD("Entered");
  return new UnicodeString(str.c_str());
}

PlatformResult TimeUtilTools::GetLocalTimeZone(std::string* result_string) {
  LoggerD("Entered");
  UnicodeString id;
  std::unique_ptr<TimeZone> zone(TimeZone::createDefault());
  zone->getID(id);

  PlatformResult res = ToUTF8String(id, result_string);
  if(res.IsError()) {
    return res;
  }
  LoggerD("local timezone: %s", result_string->c_str());
  return PlatformResult(ErrorCode::NO_ERROR);
}

Locale* TimeUtilTools::GetDefaultLocale() {
  LoggerD("Entered");
  char* tempstr = vconf_get_str(VCONFKEY_REGIONFORMAT);

  if (nullptr == tempstr){
    return nullptr;
  }

  Locale* default_locale = nullptr;

  char* p = strchr(tempstr, '.');
  int len = strlen(tempstr) - (p != nullptr ? strlen(p) : 0);

  if (len > 0) {
    char* str_region = strndup(tempstr, len); //.UTF8 => 5
    default_locale = new Locale(str_region);
    free(str_region);
  }

  free(tempstr);

  if (default_locale && default_locale->isBogus()) {
    delete default_locale;
    default_locale = nullptr;
  }

  return default_locale;
}

UnicodeString TimeUtilTools::GetDateTimeFormat(DateTimeFormatType type, bool use_locale_fmt) {
  LoggerD("Entered");
  UErrorCode ec = U_ZERO_ERROR;
  Locale* default_locale = GetDefaultLocale();

  std::unique_ptr<DateTimePatternGenerator> date_time_pattern(
      DateTimePatternGenerator::createInstance(
          ((use_locale_fmt && default_locale) ? *default_locale : Locale::getEnglish()),
          ec));

  delete default_locale;
  if (U_SUCCESS(ec)) {
    UnicodeString pattern;

    switch (type) {
      case DateTimeFormatType::kDateFormat:
        pattern = date_time_pattern->getBestPattern(UDAT_YEAR_MONTH_WEEKDAY_DAY, ec);
        break;

      case DateTimeFormatType::kDateShortFormat:
        pattern = date_time_pattern->getBestPattern(UDAT_YEAR_NUM_MONTH_DAY, ec);
        break;

      default:
      {
        int ret = 0;
        int value = 0;
        ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &value);
        // if failed, set default time format
        if (-1 == ret) {
          value = VCONFKEY_TIME_FORMAT_12;
        }

        std::string skeletone;
        if (DateTimeFormatType::kTimeFormat != type) {
          skeletone = UDAT_YEAR_MONTH_WEEKDAY_DAY;
        }

        if (VCONFKEY_TIME_FORMAT_12 == value) {
          skeletone += "hhmmss";
        } else {
          skeletone += "HHmmss";
        }

        std::unique_ptr<UnicodeString> skeletone_str(ToUnicodeString(skeletone));
        pattern = date_time_pattern->getBestPattern(*skeletone_str, ec);

        if (!use_locale_fmt) {
          pattern += " 'GMT'Z v'";
        }
      }
      break;
    }

    return pattern;
  }

  return "";
}

PlatformResult TimeUtilTools::GetDateFormat(bool shortformat, std::string* result_string) {
  LoggerD("Entered");
  UnicodeString time_format =
      TimeUtilTools::GetDateTimeFormat(
          (shortformat ?
              DateTimeFormatType::kDateShortFormat:
              DateTimeFormatType::kDateFormat),
              true);
  time_format = time_format.findAndReplace("E", "D");

  if (time_format.indexOf("MMM") > 0) {
    if (time_format.indexOf("MMMM") > 0){
      time_format = time_format.findAndReplace("MMMM", "M");
    } else {
      time_format = time_format.findAndReplace("MMM", "M");
    }
  } else {
    time_format = time_format.findAndReplace("M", "m");
  }

  int32_t i = 0;

  while (i < time_format.length() - 1) {
    if (time_format[i] == time_format[i + 1]) {
      time_format.remove(i, 1);
    } else {
      ++i;
    }
  }

  PlatformResult res = ToUTF8String(time_format, result_string);
  if(res.IsError()) {
    return res;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult TimeUtilTools::GetTimeFormat(std::string* result_string) {
  LoggerD("Entered");
  UnicodeString time_format = TimeUtilTools::GetDateTimeFormat(
      DateTimeFormatType::kTimeFormat, true);
  time_format = time_format.findAndReplace("H", "h");
  time_format = time_format.findAndReplace("K", "h");
  time_format = time_format.findAndReplace("k", "h");
  time_format = time_format.findAndReplace("a", "ap");

  int32_t i = 0;

  while (i < time_format.length() - 1) {
    if (time_format[i] == time_format[i + 1]) {
      time_format.remove(i, 1);
    } else {
      ++i;
    }
  }
  PlatformResult res = ToUTF8String(time_format, result_string);
  if(res.IsError()) {
    return res;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult TimeUtilTools::GetAvailableTimezones(picojson::array* available_timezones) {
  LoggerD("Entered");
  UErrorCode ec = U_ZERO_ERROR;
  std::unique_ptr<StringEnumeration> tz_enum(TimeZone::createEnumeration());
  const char* str = nullptr;
  int32_t count = tz_enum->count(ec);

  if (U_SUCCESS(ec)) {
    int i = 0;
    do {
      int32_t resultLen = 0;
      str = tz_enum->next(&resultLen, ec);
      if (U_SUCCESS(ec)) {
        available_timezones->push_back(picojson::value(str));
        ++i;
      } else {
        LOGE("An error occurred");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "An error occurred");
      }
    } while ((str != nullptr) && (i < count));
  }
  else {
    LOGE("Can't get timezones list");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Can't get timezones list");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

} // time
}
