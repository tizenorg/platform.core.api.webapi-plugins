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

#include "calendar-record.h"

#include <calendar-service2/calendar.h>

#include "logger.h"
#include "platform-exception.h"
#include "converter.h"

namespace webapi {
namespace calendar {

namespace {
const std::string kCalendarTypeEvent = "EVENT";
const std::string kCalendarTypeTask = "TASK";
}

using namespace webapi::common;

inline void CheckReturn(int ret, const std::string& error_name) {
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("%s : %d", error_name.c_str(), ret);
    throw UnknownException(error_name);
  }
}

void CalendarRecord::QueryDeleter(calendar_query_h handle) {
  if (handle) {
    if (CALENDAR_ERROR_NONE != calendar_query_destroy(handle)) {
      LoggerW("calendar_query_destroy failed");
    }
  }
}

void CalendarRecord::Deleter(calendar_record_h handle) {
  if (handle) {
    if (CALENDAR_ERROR_NONE != calendar_record_destroy(handle, true)) {
      LoggerW("calendar_record_destroy failed");
    }
  }
}

void CalendarRecord::ListDeleter(calendar_list_h handle) {
  if (handle) {
    if (CALENDAR_ERROR_NONE != calendar_list_destroy(handle, true)) {
      LoggerW("calendar_list_destroy failed");
    }
  }
}

std::string CalendarRecord::GetString(calendar_record_h rec,
                                      unsigned int property,
                                      bool throw_on_error) {
  char* value = NULL;
  int ret = calendar_record_get_str(rec, property, &value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get string value form record: %d", ret);
    if (throw_on_error) {
      throw common::UnknownException("Get string from record failed.");
    }
  }

  std::string str = "";
  if (value) {
    str = std::string(value);
    free(value);
  }

  return str;
}

void CalendarRecord::SetString(calendar_record_h record, unsigned int property,
                               const std::string& value, bool throw_on_error) {
  LoggerD("enter");

  int ret = calendar_record_set_str(record, property,
                                    value.empty() ? NULL : value.c_str());

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set string value to record: %d", ret);

    if (throw_on_error) {
      throw common::UnknownException("Set string to record failed.");
    }
  }
}

int CalendarRecord::GetInt(calendar_record_h rec, unsigned int property,
                           bool throw_on_error) {
  int value;
  int ret = calendar_record_get_int(rec, property, &value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get int value form record: %d", ret);
    if (throw_on_error) {
      throw common::UnknownException("Get int from record failed.");
    }
  }

  return value;
}

void CalendarRecord::SetInt(calendar_record_h record, unsigned int property,
                            int value, bool throw_on_error) {
  LoggerD("enter");

  int ret = calendar_record_set_int(record, property, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set int value to record: %d", ret);

    if (throw_on_error) {
      throw common::UnknownException("Set int to record failed.");
    }
  }
}

std::string CalendarRecord::TypeToString(int type) {
  if (CALENDAR_BOOK_TYPE_EVENT == type) {
    return kCalendarTypeEvent;
  }

  if (CALENDAR_BOOK_TYPE_TODO == type) {
    return kCalendarTypeTask;
  }

  return "";
}

std::string CalendarRecord::TypeToString(const char* view_uri) {
  if (0 == strcmp(view_uri, _calendar_event._uri)) {
    return kCalendarTypeEvent;
  }
  if (0 == strcmp(view_uri, _calendar_todo._uri)) {
    return kCalendarTypeTask;
  }

  return "";
}

int CalendarRecord::TypeToInt(const std::string& type) {
  if (kCalendarTypeEvent == type) {
    return CALENDAR_BOOK_TYPE_EVENT;
  }
  if (kCalendarTypeTask == type) {
    return CALENDAR_BOOK_TYPE_TODO;
  }

  return CALENDAR_BOOK_TYPE_NONE;
}

int CalendarRecord::TypeToInt(const char* view_uri) {
  if (0 == strcmp(view_uri, _calendar_event._uri)) {
    return CALENDAR_BOOK_TYPE_EVENT;
  }
  if (0 == strcmp(view_uri, _calendar_todo._uri)) {
    return CALENDAR_BOOK_TYPE_TODO;
  }

  return CALENDAR_BOOK_TYPE_NONE;
}

const char* CalendarRecord::TypeToUri(const std::string& type) {
  if (kCalendarTypeEvent == type) {
    return _calendar_event._uri;
  }
  if (kCalendarTypeTask == type) {
    return _calendar_todo._uri;
  }

  throw common::UnknownException("Undefined record type");
}

const char* CalendarRecord::TypeToUri(int type) {
  if (CALENDAR_BOOK_TYPE_EVENT == type) {
    return _calendar_event._uri;
  }

  if (CALENDAR_BOOK_TYPE_TODO == type) {
    return _calendar_todo._uri;
  }

  throw common::UnknownException("Undefined record type");
}

CalendarRecordPtr CalendarRecord::Create(const char* view_uri) {
  LoggerD("enter");

  calendar_record_h handle = nullptr;
  int ret = calendar_record_create(view_uri, &handle);
  if (CALENDAR_ERROR_NONE != ret || nullptr == handle) {
    LoggerE("Fail to create calendar record, error code: %d", ret);
    throw NotFoundException("Fail to create calendar record");
  }

  return CalendarRecordPtr(handle, CalendarRecord::Deleter);
}

CalendarRecordPtr CalendarRecord::CreateCalendar() {
  LoggerD("enter");

  return Create(_calendar_book._uri);
}

CalendarRecordPtr CalendarRecord::GetById(int id, const char* view_uri) {
  calendar_record_h handle = nullptr;

  int ret = calendar_db_get_record(view_uri, id, &handle);
  if (CALENDAR_ERROR_NONE != ret || nullptr == handle) {
    LoggerE("Fail to get calendar record %d for view %s, error code: %d", id,
            view_uri, ret);
    throw NotFoundException("Fail to get record with given id");
  }

  return CalendarRecordPtr(handle, CalendarRecord::Deleter);
}

int CalendarRecord::Insert(calendar_record_h rec) {
  LoggerD("enter");

  int record_id;
  int ret = calendar_db_insert_record(rec, &record_id);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Cannot insert record, error code: %d", ret);
    throw NotFoundException("Cannot insert record");
  }

  return record_id;
}

void CalendarRecord::AddChildRecord(calendar_record_h rec,
                                    unsigned int property,
                                    calendar_record_h child) {
  LoggerD("enter");

  int ret = calendar_record_add_child_record(rec, property, child);
  if (CALENDAR_ERROR_NONE != ret) {
    if (child) {
      calendar_record_destroy(child, true);
    }
    LoggerE("Cannot add child record, error code: %d", ret);
    throw NotFoundException("Cannot add child record");
  }
}

void CalendarRecord::RemoveChildRecords(calendar_record_h rec,
                                        unsigned int property_id) {
  LoggerD("enter");

  unsigned int count = 0;

  if (CALENDAR_ERROR_NONE !=
      calendar_record_get_child_record_count(rec, property_id, &count)) {
    LoggerW("Can't get attendees count");
  }

  calendar_record_h attendee;
  for (unsigned int i = 0; i < count; ++i) {
    attendee = NULL;
    // Be careful about the index. We always insert 0 cause the child
    // list is updated every time we remove one=

    if (CALENDAR_ERROR_NONE !=
        calendar_record_get_child_record_at_p(rec, property_id, 0, &attendee)) {
      LoggerW("Can't get the attendee");
      continue;
    }

    if (CALENDAR_ERROR_NONE !=
        calendar_record_remove_child_record(rec, property_id, attendee)) {
      LoggerW("Can't remove the attendee");
      continue;
    }
  }
}

unsigned int CalendarRecord::GetChildRecordCount(calendar_record_h rec,
                                                 unsigned int property,
                                                 bool throw_on_error) {
  unsigned int value;
  int ret = calendar_record_get_child_record_count(rec, property, &value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get child record count: %d", ret);
    if (throw_on_error) {
      throw common::UnknownException("Get child record count failed.");
    }
  }

  return value;
}

bool CalendarRecord::GetChildRecordAt(calendar_record_h rec,
                                      unsigned int property,
                                      calendar_record_h* result, int index,
                                      bool throw_on_error) {
  int ret = calendar_record_get_child_record_at_p(rec, property, index, result);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get child record at: %d", ret);
    if (throw_on_error) {
      throw common::UnknownException("Get child record at failed.");
    }

    return false;
  }

  return true;
}

void CalendarRecord::CalendarToJson(calendar_record_h rec,
                                    json::Object* out_ptr) {
  json::Object& out = *out_ptr;

  if (NULL == rec) {
    LoggerE("Calendar record is null");
    throw UnknownException("Calendar record is null");
  }

  int id = GetInt(rec, _calendar_book.id);
  int account_id = GetInt(rec, _calendar_book.account_id);
  std::string name = GetString(rec, _calendar_book.name);
  std::string type = TypeToString(GetInt(rec, _calendar_book.store_type));

  out.insert(std::make_pair("id", json::Value(std::to_string(id))));
  out.insert(
      std::make_pair("accountId", json::Value(std::to_string(account_id))));
  out.insert(std::make_pair("name", json::Value(name)));
  out.insert(std::make_pair("type", json::Value(type)));
}

void CalendarRecord::CalendarFromJson(calendar_record_h rec,
                                      const json::Object& in) {
  if (in.empty()) {
    LoggerE("Empty Calendar object.");
    throw InvalidValuesException("Empty Calendar object.");
  }

  const std::string& name = FromJson<std::string>(in, "name");
  int account_id = static_cast<int>(FromJson<double>(in, "accountId"));
  const std::string& type = FromJson<std::string>(in, "type");

  int store_type = 0;
  if (kCalendarTypeEvent == type) {
    store_type = CALENDAR_BOOK_TYPE_EVENT;
  } else if (kCalendarTypeTask == type) {
    store_type = CALENDAR_BOOK_TYPE_TODO;
  }

  int ret = calendar_record_set_str(rec, _calendar_book.name, name.c_str());
  CheckReturn(ret, "Failed to set name");

  ret = calendar_record_set_int(rec, _calendar_book.account_id, account_id);
  CheckReturn(ret, "Failed to set account_id");

  ret = calendar_record_set_int(rec, _calendar_book.store_type, store_type);
  CheckReturn(ret, "Failed to set store_type");
}

}  // namespace calendar
}  // namespace webapi
