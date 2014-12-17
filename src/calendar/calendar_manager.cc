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

inline void CheckReturn(int ret, const std::string& error_name) {
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("%s : %d", error_name.c_str(), ret);
    throw UnknownException(error_name);
  }
}

CalendarManager::CalendarManager() {
  if (CALENDAR_ERROR_NONE == calendar_connect()) {
    LoggerD("Calendar DB connected");
    is_connected_ = true;
  } else {
    LoggerE("Calendar DB connection failed");
  }
}

CalendarManager::~CalendarManager() {
  if (is_connected_) {
    if (CALENDAR_ERROR_NONE == calendar_disconnect()) {
      LoggerD("Calendar DB disconnected");
    } else {
      LoggerE("Calendar DB disconnect failed");
    }
  }
}

CalendarManager& CalendarManager::GetInstance() {
  static CalendarManager instance;
  return instance;
}

bool CalendarManager::IsConnected() { return is_connected_; }

void CalendarManager::GetCalendars(const JsonObject& args,
                                   JsonObject& out) {
  LoggerD("enter");

//  NativePlugin::CheckAccess(Privilege::kCalendarRead);

  if (!is_connected_) {
    throw UnknownException("DB Connection failed.");
  }

//  int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);

  const std::string& type = FromJson<std::string>(args, "type");

  LoggerD("calendar type: %s", type.c_str());

  auto get = [type](const std::shared_ptr<JsonValue> & response)->void {

    JsonObject& response_obj = response->get<JsonObject>();
    JsonValue result = JsonValue(JsonArray());
    JsonArray& array = result.get<JsonArray>();

    calendar_list_h list = NULL;

    try {
      int ret = calendar_db_get_all_records(_calendar_book._uri, 0, 0, &list);
      CheckReturn(ret, "Failed to get list");

      int count = 0;
      ret = calendar_list_get_count(list, &count);
      CheckReturn(ret, "Failed to get list size");

      LoggerD("Calendar list count: %d", count);

      ret = calendar_list_first(list);
      CheckReturn(ret, "Failed to move list to the first position");

      int current_calendar_type = CalendarRecord::TypeToInt(type);
      calendar_record_h calendar = NULL;
      int store_type;

      while (count-- > 0) {
        ret = calendar_list_get_current_record_p(list, &calendar);
        CheckReturn(ret, "Failed to get current record");

        store_type =
            CalendarRecord::GetInt(calendar, _calendar_book.store_type);
        if (current_calendar_type != store_type) {
          LoggerD("Different store type %d, requested: %d. Skipping...",
                  store_type, current_calendar_type);
          calendar_list_next(list);
          continue;
        }

        array.push_back(JsonValue(JsonObject()));

        CalendarRecord::CalendarToJson(calendar,
                                       &array.back().get<JsonObject>());

        calendar_list_next(list);
      }

      if (list) {
        calendar_list_destroy(list, true);
      }

 //     NativePlugin::ReportSuccess(result, response_obj);
    }
    catch (...) {//const BasePlatformException& e) {
      if (list) {
        calendar_list_destroy(list, false);
      }

 //     NativePlugin::ReportError(e, response_obj);
    }
  };

//  auto get_response = [callback_handle](const std::shared_ptr<JsonValue> &
//                                        response)->void {
//    wrt::common::NativeContext::GetInstance()->InvokeCallback(
//        callback_handle, response->serialize());
//  };

//  TaskQueue::GetInstance().Queue<JsonValue>(
//      get, get_response,
//      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));

//  NativePlugin::ReportSuccess(out);
}

void CalendarManager::GetCalendar(const JsonObject& args, JsonObject& out) {
  LoggerD("enter");

 // NativePlugin::CheckAccess(Privilege::kCalendarRead);

  if (!is_connected_) {
    throw UnknownException("DB Connection failed.");
  }

  int id = common::stol(FromJson<std::string>(args, "id"));

  CalendarRecordPtr record_ptr =
      CalendarRecord::GetById(id, _calendar_book._uri);

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  int calendar_type =
      CalendarRecord::GetInt(record_ptr.get(), _calendar_book.store_type);
  if (type != calendar_type) {
    LoggerD("Calendar type doesn't match requested type");
    throw NotFoundException("Calendar not found");
  }

  JsonValue result = JsonValue(JsonObject());

  CalendarRecord::CalendarToJson(record_ptr.get(), &out);
//  NativePlugin::ReportSuccess(result, out);
}

void CalendarManager::AddCalendar(const JsonObject& args, JsonObject& out) {
  LoggerD("enter");

//  NativePlugin::CheckAccess(Privilege::kCalendarWrite);

  if (!is_connected_) {
    throw UnknownException("DB Connection failed.");
  }

  const JsonObject& calendar = FromJson<JsonObject>(args, "calendar");

  CalendarRecordPtr record_ptr = CalendarRecord::CreateCalendar();
  CalendarRecord::CalendarFromJson(record_ptr.get(), calendar);

  int ret, record_id;
  ret = calendar_db_insert_record(record_ptr.get(), &record_id);
  CheckReturn(ret, "Failed to insert calendar record into db");

//  NativePlugin::ReportSuccess(JsonValue(static_cast<double>(record_id)), out);
}

void CalendarManager::RemoveCalendar(const JsonObject& args,
                                     JsonObject& out) {
  LoggerD("enter");

//  NativePlugin::CheckAccess(Privilege::kCalendarWrite);

  if (!is_connected_) {
    throw UnknownException("DB Connection failed.");
  }

  int id = common::stol(FromJson<std::string>(args, "id"));

  if (id == kUnifiedCalendardId) {
    LoggerE("Unified calendar can not be deleted");
    throw InvalidValuesException("Unified calendar can not be deleted");
  } else if (id == DEFAULT_EVENT_CALENDAR_BOOK_ID) {
    LoggerE("Default event calendar can not be deleted");
    throw InvalidValuesException("Default event calendar can not be deleted");
  } else if (id == DEFAULT_TODO_CALENDAR_BOOK_ID) {
    LoggerE("Default todo calendar can not be deleted");
    throw InvalidValuesException("Default todo calendar can not be deleted");
  }

  int ret = calendar_db_delete_record(_calendar_book._uri, id);
  CheckReturn(ret, "Failed to delete record from db");

 // NativePlugin::ReportSuccess(out);
}
}
}
