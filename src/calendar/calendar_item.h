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

#ifndef CALENDAR_CALENDAR_ITEM_H_
#define CALENDAR_CALENDAR_ITEM_H_

#include <string>

#include "calendar_record.h"
#include "common/picojson.h"

namespace extension {
namespace calendar {
struct Date {
  long long int utc_timestamp_;
  int year_;
  int month_;
  int day_;
  std::string time_zone_;
};

typedef std::map<std::string, std::map<int, unsigned int>> PlatformPropertyMap;
typedef std::map<std::string, std::map<std::string, int>> PlatformEnumMap;
typedef std::map<std::string, std::map<int, std::string>>
    PlatformEnumReverseMap;

class CalendarItem : public CalendarRecord {
 public:
  static common::PlatformResult Create(int type, calendar_record_h *handle);
  static common::PlatformResult Remove(int type, int id);

  static common::PlatformResult GetPlatformProperty(int type,
                                                    const std::string& property,
                                                    unsigned int* value);
  static common::PlatformResult StringToPlatformEnum(const std::string& type,
                                                     const std::string& value,
                                                     int* platform_enum);
  static common::PlatformResult PlatformEnumToString(const std::string& field,
                                                     int value,
                                                     std::string* platform_str);

  // string
  static common::PlatformResult SetString(int type, calendar_record_h rec,
                                          const std::string& property,
                                          const picojson::object& in,
                                          bool optional = false);
  static common::PlatformResult SetString(int type, calendar_record_h rec,
                                          const std::string& property,
                                          const std::string& value);
  static common::PlatformResult GetString(int type, calendar_record_h rec,
                                          const std::string& property,
                                          std::string* value);

  // int
  static common::PlatformResult SetInt(int type, calendar_record_h rec,
                                       const std::string& property,
                                       const picojson::object& in,
                                       bool optional = false);
  static common::PlatformResult SetInt(int type, calendar_record_h rec,
                                       const std::string& property, int value);
  static common::PlatformResult GetInt(int type, calendar_record_h rec,
                                       const std::string& property, int* value);

  // enum
  static common::PlatformResult SetEnum(int type, calendar_record_h rec,
                                        const std::string& property,
                                        const picojson::object& in,
                                        const std::string& enum_name);
  static common::PlatformResult SetEnum(calendar_record_h rec,
                                        unsigned int property,
                                        const std::string& enum_name,
                                        const std::string& value);
  static common::PlatformResult GetEnum(int type, calendar_record_h rec,
                                        const std::string& property,
                                        const std::string& enum_name,
                                        std::string* enum_str);
  static common::PlatformResult GetEnum(calendar_record_h rec,
                                        unsigned int property,
                                        const std::string& enum_name,
                                        std::string* enum_str);

  // double
  static common::PlatformResult SetDouble(int type, calendar_record_h rec,
                                          const std::string& property,
                                          double value);
  static common::PlatformResult GetDouble(int type, calendar_record_h rec,
                                          const std::string& property,
                                          double *value);

  // calendar_time_s
  static common::PlatformResult SetCaltime(int type, calendar_record_h rec,
                                           const std::string& property,
                                           calendar_time_s value,
                                           bool throw_on_error = true);
  static common::PlatformResult SetCaltime(calendar_record_h rec,
                                           unsigned int property,
                                           calendar_time_s value,
                                           bool throw_on_error = true);
  static common::PlatformResult GetCaltime(int type, calendar_record_h rec,
                                           const std::string& property,
                                           calendar_time_s * cal_time,
                                           bool throw_on_error = true);
  static common::PlatformResult GetCaltime(calendar_record_h rec,
                                           unsigned int property,
                                           calendar_time_s * cal_time,
                                           bool throw_on_error = true);

  // long long int
  static common::PlatformResult SetLli(calendar_record_h rec,
                                       unsigned int property,
                                       long long int value,
                                       bool throw_on_error = true);
  static common::PlatformResult GetLli(int type, calendar_record_h rec,
                                       const std::string& property,
                                       long long int* lli);
  static common::PlatformResult GetLli(calendar_record_h rec,
                                       unsigned int property,
                                       long long int* value,
                                       bool throw_on_error = true);

  // conversions
  static common::PlatformResult FromJson(int type, calendar_record_h record,
                                         const picojson::object& in);
  static common::PlatformResult ToJson(int type, calendar_record_h record,
                                       picojson::object* out_ptr);

  static std::string ExceptionsFromJson(const picojson::array& exceptions);
  static Date DateFromJson(const  picojson::object& in);
  static Date DateFromJson(const  picojson::object& in,
                           const char* obj_name);
   static calendar_time_s DateToPlatform(const Date& date, bool is_all_day);

 private:
  // from JSON to platform

  static common::PlatformResult CategoriesFromJson(
      int type, calendar_record_h rec, const picojson::array& value);
  static common::PlatformResult AttendeesFromJson(int type,
                                                  calendar_record_h rec,
                                                  const picojson::array& value);
  static common::PlatformResult AlarmsFromJson(int type, calendar_record_h rec,
                                               const picojson::array& alarms);
  static common::PlatformResult RecurrenceRuleFromJson(
      calendar_record_h rec, const picojson::object& rrule);

  // from platform to JSON
  static picojson::value DateToJson(Date *date);
  static common::PlatformResult CategoriesToJson(int type,
                                                 calendar_record_h rec,
                                                 picojson::array* value);
  static common::PlatformResult AttendeesToJson(int type, calendar_record_h rec,
                                                picojson::array* out);
  static common::PlatformResult AlarmsToJson(int type, calendar_record_h rec,
                                             picojson::array* out);
  static common::PlatformResult RecurrenceRuleToJson(calendar_record_h rec,
                                                     picojson::object* out_ptr);

  static common::PlatformResult DateFromPlatform(int type,
                                                 calendar_record_h rec,
                                                 const std::string& property,
                                                 Date* date_from_platform);
  static common::PlatformResult DateFromPlatform(calendar_record_h rec,
                                                 unsigned int property,
                                                 Date* date_from_platform);

  static picojson::array StringToArray(const std::string& string);

  static const PlatformPropertyMap platform_property_map_;
  static const PlatformEnumMap platform_enum_map_;
  // @todo can be replaced by Boost.Bimap
  static PlatformEnumReverseMap platform_enum_reverse_map_;
};

}  // namespace calendar
}  // namespace webapi

#endif  // CALENDAR_CALENDAR_ITEM_H_
