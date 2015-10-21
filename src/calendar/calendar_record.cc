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

#include "calendar_record.h"

#include <calendar-service2/calendar.h>

#include "common/logger.h"
#include "common/converter.h"

namespace extension {
namespace calendar {

namespace {
const std::string kCalendarTypeEvent = "EVENT";
const std::string kCalendarTypeTask = "TASK";
}

using namespace common;

PlatformResult CalendarRecord::CheckReturn(int ret,
                                           const std::string& error_name) {
  LoggerD("Enter");
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("%s : %d", error_name.c_str(), ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_name);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void CalendarRecord::QueryDeleter(calendar_query_h handle) {
  LoggerD("Enter");
  if (handle) {
    if (CALENDAR_ERROR_NONE != calendar_query_destroy(handle)) {
      LoggerW("calendar_query_destroy failed");
    }
  }
}

void CalendarRecord::Deleter(calendar_record_h handle) {
  LoggerD("Enter");
  if (handle) {
    if (CALENDAR_ERROR_NONE != calendar_record_destroy(handle, true)) {
      LoggerW("calendar_record_destroy failed");
    }
  }
}

void CalendarRecord::ListDeleter(calendar_list_h handle) {
  LoggerD("Enter");
  if (handle) {
    if (CALENDAR_ERROR_NONE != calendar_list_destroy(handle, true)) {
      LoggerW("calendar_list_destroy failed");
    }
  }
}

PlatformResult CalendarRecord::GetString(calendar_record_h rec,
                                         unsigned int property,
                                         std::string* str,
                                         bool throw_on_error) {
  LoggerD("Enter");
  char* value = NULL;
  int ret = calendar_record_get_str(rec, property, &value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get string value form record: %d", ret);
    if (throw_on_error) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Get string from record failed.");
    }
  }

  *str = "";
  if (value) {
    *str = std::string(value);
    free(value);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::SetString(calendar_record_h record,
                                         unsigned int property,
                                         const std::string& value,
                                         bool throw_on_error) {
  LoggerD("Enter");
  int ret = calendar_record_set_str(record, property,
                                    value.empty() ? NULL : value.c_str());

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't set string value to record: %d", ret);
    if (throw_on_error) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set string to record failed.");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::GetInt(calendar_record_h rec,
                                      unsigned int property, int* value,
                                      bool throw_on_error) {
  LoggerD("Enter");
  int ret = calendar_record_get_int(rec, property, value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get int value form record: %d", ret);
    if (throw_on_error) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Get int from record failed.");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::SetInt(calendar_record_h record,
                                      unsigned int property, int value,
                                      bool throw_on_error) {
  LoggerD("Enter");
  int ret = calendar_record_set_int(record, property, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't set int value to record: %d", ret);
    if (throw_on_error) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Set int to record failed.");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

std::string CalendarRecord::TypeToString(int type) {
  LoggerD("Enter");
  if (CALENDAR_BOOK_TYPE_EVENT == type) {
    return kCalendarTypeEvent;
  }

  if (CALENDAR_BOOK_TYPE_TODO == type) {
    return kCalendarTypeTask;
  }

  return "";
}

std::string CalendarRecord::TypeToString(const char* view_uri) {
  LoggerD("Enter");
  if (0 == strcmp(view_uri, _calendar_event._uri)) {
    return kCalendarTypeEvent;
  }
  if (0 == strcmp(view_uri, _calendar_todo._uri)) {
    return kCalendarTypeTask;
  }

  return "";
}

int CalendarRecord::TypeToInt(const std::string& type) {
  LoggerD("Enter");
  if (kCalendarTypeEvent == type) {
    return CALENDAR_BOOK_TYPE_EVENT;
  }
  if (kCalendarTypeTask == type) {
    return CALENDAR_BOOK_TYPE_TODO;
  }

  return CALENDAR_BOOK_TYPE_NONE;
}

int CalendarRecord::TypeToInt(const char* view_uri) {
  LoggerD("Enter");
  if (0 == strcmp(view_uri, _calendar_event._uri)) {
    return CALENDAR_BOOK_TYPE_EVENT;
  }
  if (0 == strcmp(view_uri, _calendar_todo._uri)) {
    return CALENDAR_BOOK_TYPE_TODO;
  }

  return CALENDAR_BOOK_TYPE_NONE;
}

PlatformResult CalendarRecord::TypeToUri(const std::string& type,
                                         std::string* uri) {
  LoggerD("Enter");
  if (kCalendarTypeEvent == type) {
    *uri = _calendar_event._uri;
  } else if (kCalendarTypeTask == type) {
    *uri = _calendar_todo._uri;
  } else {
    LoggerE("Undefined record type: %s", type.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Undefined record type");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::TypeToUri(int type, std::string* uri) {
  LoggerD("Enter");
  if (CALENDAR_BOOK_TYPE_EVENT == type) {
    *uri = _calendar_event._uri;
  } else if (CALENDAR_BOOK_TYPE_TODO == type) {
    *uri = _calendar_todo._uri;
  } else {
    LoggerE("Undefined record type: %d", type);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Undefined record type");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::Create(const char* view_uri,
                                      calendar_record_h* handle) {
  LoggerD("Enter");
  int ret = calendar_record_create(view_uri, handle);
  if (CALENDAR_ERROR_NONE != ret || nullptr == handle) {
    LoggerE("Fail to create calendar record, error code: %d", ret);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Fail to create calendar record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::CreateCalendar(calendar_record_h* handle) {
  LoggerD("Enter");
  return Create(_calendar_book._uri, handle);
}

PlatformResult CalendarRecord::GetById(int id, const char* view_uri,
                                       calendar_record_h* handle) {
  LoggerD("Enter");
  int ret = calendar_db_get_record(view_uri, id, handle);
  if (CALENDAR_ERROR_NONE != ret || nullptr == handle) {
    LoggerE("Fail to get calendar record %d for view %s, error code: %d", id,
            view_uri, ret);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Fail to get record with given id");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::Insert(calendar_record_h rec, int* record_id) {
  LoggerD("Enter");
  int ret = calendar_db_insert_record(rec, record_id);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Cannot insert record, error code: %d", ret);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot insert record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::AddChildRecord(calendar_record_h rec,
                                              unsigned int property,
                                              calendar_record_h child) {
  LoggerD("Enter");
  int ret = calendar_record_add_child_record(rec, property, child);
  if (CALENDAR_ERROR_NONE != ret) {
    if (child) {
      calendar_record_destroy(child, true);
    }
    LoggerE("Cannot add child record, error code: %d", ret);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot add child record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void CalendarRecord::RemoveChildRecords(calendar_record_h rec,
                                        unsigned int property_id) {
  LoggerD("Enter");
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

PlatformResult CalendarRecord::GetChildRecordCount(calendar_record_h rec,
                                                   unsigned int property,
                                                   bool throw_on_error,
                                                   unsigned int* value) {
  LoggerD("Enter");
  int ret = calendar_record_get_child_record_count(rec, property, value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get child record count: %d", ret);
    if (throw_on_error) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Get child record count failed.");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::GetChildRecordAt(calendar_record_h rec,
                                                unsigned int property,
                                                calendar_record_h* result,
                                                int index) {
  LoggerD("Enter");
  int ret = calendar_record_get_child_record_at_p(rec, property, index, result);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get child record at: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get child record at failed.");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::CalendarToJson(calendar_record_h rec,
                                              picojson::object* out_ptr) {
  LoggerD("Enter");
  picojson::object& out = *out_ptr;

  if (NULL == rec) {
    LoggerE("Calendar record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Calendar record is null");
  }

  int id = 0;
  PlatformResult status = GetInt(rec, _calendar_book.id, &id);
  int account_id;
  GetInt(rec, _calendar_book.account_id, &account_id);

  std::string name;
  status = GetString(rec, _calendar_book.name, &name);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int value;
  status = GetInt(rec, _calendar_book.store_type, &value);
  std::string type = TypeToString(value);

  out.insert(std::make_pair("id", picojson::value(std::to_string(id))));
  out.insert(
      std::make_pair("accountId", picojson::value(std::to_string(account_id))));
  out.insert(std::make_pair("name", picojson::value(name)));
  out.insert(std::make_pair("type", picojson::value(type)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarRecord::CalendarFromJson(calendar_record_h rec,
                                                const picojson::object& in) {
  LoggerD("Enter");
  if (in.empty()) {
    LoggerE("Empty Calendar object.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Empty Calendar object.");
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
  PlatformResult status = CheckReturn(ret, "Failed to set name");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ret = calendar_record_set_int(rec, _calendar_book.account_id, account_id);
  status = CheckReturn(ret, "Failed to set account_id");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ret = calendar_record_set_int(rec, _calendar_book.store_type, store_type);
  status = CheckReturn(ret, "Failed to set store_type");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace calendar
}  // namespace webapi
