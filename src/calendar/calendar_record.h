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

#ifndef WEBAPI_PLUGINS_CALENDAR_RECORD_H_
#define WEBAPI_PLUGINS_CALENDAR_RECORD_H_

#include <type_traits>
#include <string>
#include <memory>
#include <calendar-service2/calendar.h>

#include "common/picojson.h"
namespace extension {
namespace calendar {

typedef std::unique_ptr<std::remove_pointer<calendar_query_h>::type,
                        void (*)(calendar_query_h)> CalendarQueryPtr;
typedef std::unique_ptr<std::remove_pointer<calendar_record_h>::type,
                        void (*)(calendar_record_h)> CalendarRecordPtr;
typedef std::unique_ptr<std::remove_pointer<calendar_list_h>::type,
                        void (*)(calendar_list_h)> CalendarListPtr;

class CalendarRecord {
 public:
  static void QueryDeleter(calendar_query_h handle);
  static void Deleter(calendar_record_h handle);
  static void ListDeleter(calendar_list_h handle);

  static std::string GetString(calendar_record_h rec, unsigned int property,
                               bool throw_on_error = true);
  static void SetString(calendar_record_h rec, unsigned int property,
                        const std::string& value, bool throw_on_error = true);

  static int GetInt(calendar_record_h rec, unsigned int property,
                    bool throw_on_error = true);
  static void SetInt(calendar_record_h rec, unsigned int property, int value,
                     bool throw_on_error = true);

  static std::string TypeToString(int type);
  static std::string TypeToString(const char* view_uri);
  static int TypeToInt(const std::string& type);
  static int TypeToInt(const char* view_uri);
  static const char* TypeToUri(const std::string& type);
  static const char* TypeToUri(int type);

  static int Insert(calendar_record_h rec);

  static void AddChildRecord(calendar_record_h rec, unsigned int property,
                             calendar_record_h child);
  static void RemoveChildRecords(calendar_record_h rec,
                                 unsigned int property_id);
  static unsigned int GetChildRecordCount(calendar_record_h rec,
                                          unsigned int property,
                                          bool throw_on_error = true);
  static bool GetChildRecordAt(calendar_record_h rec, unsigned int property,
                               calendar_record_h* result, int index,
                               bool throw_on_error = true);

  static CalendarRecordPtr GetById(int id, const char* view_uri);
  static CalendarRecordPtr GetCalendar(int id);
  static CalendarRecordPtr GetItem(int id, const char* view_uri);
  static CalendarRecordPtr Create(const char* view_uri);
  static CalendarRecordPtr CreateCalendar();

  static void CalendarToJson(calendar_record_h rec,
                             picojson::object* out_ptr);
  static void CalendarFromJson(calendar_record_h rec,
                               const picojson::object& in);
};

}  // namespace calendar
}  // namespace webapi

#endif  // WEBAPI_PLUGINS_CALENDAR_RECORD_H_
