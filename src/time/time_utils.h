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

#ifndef TIME_TIME_UTILS_H_
#define TIME_TIME_UTILS_H_

#include <memory>
#include <unicode/unistr.h>
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace time {

class TimeUtilTools
{
 public:
  enum class DateTimeFormatType {
    kTimeFormat,
    kDateFormat,
    kDateShortFormat,
    kDateTimeFormat
  };
  enum class DSTTransition {
    kPreviousDST,
    kNextDST
  };
  static UDate GetDSTTransition(UDate dstTransitionDate,
                                const std::shared_ptr<UnicodeString>& timezone_id,
                                DSTTransition tr_type);
  static common::PlatformResult IsDST(UDate dstTransitionDate,
                                      const std::shared_ptr<UnicodeString>& timezone_id, bool* result_bool);
  static common::PlatformResult GetTimezoneAbbreviation(UDate date,
                                                        const std::shared_ptr<UnicodeString>& timezone_id,
                                                        std::string* result_string);
  static common::PlatformResult ToStringHelper(
      UDate date, std::shared_ptr<UnicodeString>& timezone_id, bool use_locale_fmt,
      TimeUtilTools::DateTimeFormatType type,
      std::string* result_string);
  static common::PlatformResult ToUTF8String(const UnicodeString& uniStr,
                                             std::string* result_string);
  static UnicodeString* ToUnicodeString(const std::string& str);
  static common::PlatformResult GetLocalTimeZone(std::string* result_string);
  static UnicodeString GetDateTimeFormat(DateTimeFormatType type, bool bLocale);
  static Locale* GetDefaultLocale();
  static common::PlatformResult GetDateFormat(bool shortformat, std::string* result_string);
  static common::PlatformResult GetTimeFormat(std::string* result_string);
  static common::PlatformResult GetAvailableTimezones(picojson::array* available_timezones);
};

}
}

#endif // TIME_TIME_UTILS_H_
