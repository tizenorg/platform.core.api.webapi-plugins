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
  static CalendarRecordPtr Create(int type);
  static void Remove(int type, int id);

  static unsigned int GetPlatformProperty(int type,
                                          const std::string& property);
  static int StringToPlatformEnum(const std::string& type,
                                  const std::string& value);
  static std::string PlatformEnumToString(const std::string& field, int value);

  // string
  static void SetString(int type, calendar_record_h rec,
                        const std::string& property,
                        const  picojson::object& in, bool optional = false);
  static void SetString(int type, calendar_record_h rec,
                        const std::string& property, const std::string& value);
  static std::string GetString(int type, calendar_record_h rec,
                               const std::string& property);

  // int
  static void SetInt(int type, calendar_record_h rec,
                     const std::string& property,
                     const  picojson::object& in, bool optional = false);
  static void SetInt(int type, calendar_record_h rec,
                     const std::string& property, int value);
  static int GetInt(int type, calendar_record_h rec,
                    const std::string& property);

  // enum
  static void SetEnum(int type, calendar_record_h rec,
                      const std::string& property,
                      const  picojson::object& in,
                      const std::string& enum_name);
  static void SetEnum(calendar_record_h rec, unsigned int property,
                      const std::string& enum_name, const std::string& value);
  static std::string GetEnum(int type, calendar_record_h rec,
                             const std::string& property,
                             const std::string& enum_name);
  static std::string GetEnum(calendar_record_h rec, unsigned int property,
                             const std::string& enum_name);

  // double
  static void SetDouble(int type, calendar_record_h rec,
                        const std::string& property, double value);
  static double GetDouble(int type, calendar_record_h rec,
                          const std::string& property);

  // calendar_time_s
  static void SetCaltime(int type, calendar_record_h rec,
                         const std::string& property, calendar_time_s value,
                         bool throw_on_error = true);
  static void SetCaltime(calendar_record_h rec, unsigned int property,
                         calendar_time_s value, bool throw_on_error = true);
  static calendar_time_s GetCaltime(int type, calendar_record_h rec,
                                    const std::string& property,
                                    bool throw_on_error = true);
  static calendar_time_s GetCaltime(calendar_record_h rec,
                                    unsigned int property,
                                    bool throw_on_error = true);

  // long long int
  static void SetLli(calendar_record_h rec, unsigned int property,
                     long long int value, bool throw_on_error = true);
  static long long int GetLli(int type, calendar_record_h rec,
                              const std::string& property);
  static long long int GetLli(calendar_record_h rec, unsigned int property,
                              bool throw_on_error = true);

  // conversions
  static void FromJson(int type, calendar_record_h record,
                       const  picojson::object& in);
  static void ToJson(int type, calendar_record_h record,
                      picojson::object* out_ptr);

  static std::string ExceptionsFromJson(const picojson::array& exceptions);

 private:
  // from JSON to platform
  static Date DateFromJson(const  picojson::object& in);
  static Date DateFromJson(const  picojson::object& in,
                           const char* obj_name);
  static void CategoriesFromJson(int type, calendar_record_h rec,
                                 const picojson::array& value);
  static void AttendeesFromJson(int type, calendar_record_h rec,
                                const picojson::array& value);
  static void AlarmsFromJson(int type, calendar_record_h rec,
                             const picojson::array& alarms);
  static void RecurrenceRuleFromJson(calendar_record_h rec,
                                     const  picojson::object& rrule);

  static calendar_time_s DateToPlatform(const Date& date, bool is_all_day);

  // from platform to JSON
  static picojson::value DateToJson(Date date);
  static picojson::array CategoriesToJson(int type, calendar_record_h rec);
  static picojson::array AttendeesToJson(int type, calendar_record_h rec);
  static picojson::array AlarmsToJson(int type, calendar_record_h rec);
  static  picojson::object RecurrenceRuleToJson(calendar_record_h rec);

  static Date DateFromPlatform(int type, calendar_record_h rec,
                               const std::string& property);
  static Date DateFromPlatform(calendar_record_h rec, unsigned int property);

  static picojson::array StringToArray(const std::string& string);

  static const PlatformPropertyMap platform_property_map_;
  static const PlatformEnumMap platform_enum_map_;
  // @todo can be replaced by Boost.Bimap
  static PlatformEnumReverseMap platform_enum_reverse_map_;
};

}  // namespace calendar
}  // namespace webapi

#endif  // CALENDAR_CALENDAR_ITEM_H_
