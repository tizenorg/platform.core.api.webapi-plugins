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
 
#include "calendar_manager.h"
#include "calendar_record.h"
#include "calendar_privilege.h"

#include <memory>
#include <map>
#include <calendar-service2/calendar.h>
#include "calendar_record.h"

#include "common/task-queue.h"
#include "common/converter.h"
#include "common/logger.h"

namespace extension {
namespace calendar {

namespace {
const int kUnifiedCalendardId = 0;
}

using namespace common;

CalendarManager::CalendarManager() {
  LoggerD("Enter");
  if (CALENDAR_ERROR_NONE == calendar_connect()) {
    LoggerD("Calendar DB connected");
    is_connected_ = true;
  } else {
    LoggerE("Calendar DB connection failed");
  }
}

CalendarManager::~CalendarManager() {
  LoggerD("Enter");
  if (is_connected_) {
    if (CALENDAR_ERROR_NONE == calendar_disconnect()) {
      LoggerD("Calendar DB disconnected");
    } else {
      LoggerE("Calendar DB disconnect failed");
    }
  }
}

CalendarManager& CalendarManager::GetInstance() {
  LoggerD("Enter");
  static CalendarManager instance;
  return instance;
}

bool CalendarManager::IsConnected() { return is_connected_; }

PlatformResult CalendarManager::GetCalendars(const JsonObject& args,
                                             JsonArray& array) {
  LoggerD("Enter");
  if (!is_connected_) {
    LoggerE("DB Connection failed.");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  const std::string& type = FromJson<std::string>(args, "type");

  LoggerD("calendar type: %s", type.c_str());

  calendar_list_h list = NULL;
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);
  int ret = calendar_db_get_all_records(_calendar_book._uri, 0, 0, &list);
  PlatformResult status =
      CalendarRecord::CheckReturn(ret, "Failed to get list");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int count = 0;
  ret = calendar_list_get_count(list, &count);
  status = CalendarRecord::CheckReturn(ret, "Failed to get list size");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  LoggerD("Calendar list count: %d", count);

  ret = calendar_list_first(list);
  status = CalendarRecord::CheckReturn(
      ret, "Failed to move list to the first position");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int current_calendar_type = CalendarRecord::TypeToInt(type);
  calendar_record_h calendar = NULL;
  int store_type;

  while (count-- > 0) {
    ret = calendar_list_get_current_record_p(list, &calendar);
    status = CalendarRecord::CheckReturn(ret, "Failed to get current record");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    PlatformResult status = CalendarRecord::GetInt(
        calendar, _calendar_book.store_type, &store_type);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (current_calendar_type != store_type) {
      LoggerD("Different store type %d, requested: %d. Skipping...",
              store_type, current_calendar_type);
      calendar_list_next(list);
      continue;
    }

    array.push_back(JsonValue(JsonObject()));

    status = CalendarRecord::CalendarToJson(calendar,
                                            &array.back().get<JsonObject>());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    calendar_list_next(list);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarManager::GetCalendar(const JsonObject& args,
                                            JsonObject& out) {
  LoggerD("Enter");
  if (!is_connected_) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  int id = common::stol(FromJson<std::string>(args, "id"));

  calendar_record_h handle = nullptr;
  PlatformResult status =
      CalendarRecord::GetById(id, _calendar_book._uri, &handle);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  CalendarRecordPtr record_ptr =
      CalendarRecordPtr(handle, CalendarRecord::Deleter);

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  int calendar_type;
  status = CalendarRecord::GetInt(record_ptr.get(), _calendar_book.store_type,
                                  &calendar_type);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (type != calendar_type) {
    LoggerD("Calendar type doesn't match requested type");
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Calendar not found");
  }

  status = CalendarRecord::CalendarToJson(record_ptr.get(), &out);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarManager::AddCalendar(const JsonObject& args,
                                            JsonObject& out) {
  LoggerD("Enter");
  if (!is_connected_) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  const JsonObject& calendar = FromJson<JsonObject>(args, "calendar");

  calendar_record_h handle = nullptr;
  PlatformResult status = CalendarRecord::CreateCalendar(&handle);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  CalendarRecordPtr record_ptr =
      CalendarRecordPtr(handle, CalendarRecord::Deleter);

  status = CalendarRecord::CalendarFromJson(record_ptr.get(), calendar);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int ret, record_id;
  ret = calendar_db_insert_record(record_ptr.get(), &record_id);
  status = CalendarRecord::CheckReturn(
      ret, "Failed to insert calendar record into db");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
}

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarManager::RemoveCalendar(const JsonObject& args,
                                               JsonObject& out) {
  LoggerD("Enter");
  if (!is_connected_) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  int id = common::stol(FromJson<std::string>(args, "id"));

  if (id == kUnifiedCalendardId) {
    LoggerE("Unified calendar can not be deleted");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Unified calendar can not be deleted");
  } else if (id == DEFAULT_EVENT_CALENDAR_BOOK_ID) {
    LoggerE("Default event calendar can not be deleted");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Default event calendar can not be deleted");
  } else if (id == DEFAULT_TODO_CALENDAR_BOOK_ID) {
    LoggerE("Default todo calendar can not be deleted");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Default todo calendar can not be deleted");
  }

  int ret = calendar_db_delete_record(_calendar_book._uri, id);
  PlatformResult status =
      CalendarRecord::CheckReturn(ret, "Failed to delete record from db");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}
}
}
