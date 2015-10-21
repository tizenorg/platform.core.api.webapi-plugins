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

#include "calendar.h"

#include "common/platform_exception.h"
#include "common/logger.h"
#include "common/converter.h"
#include "common/task-queue.h"
#include "common/filter-utils.h"
#include "calendar/calendar_manager.h"
#include "calendar/calendar_privilege.h"
#include "calendar/calendar_item.h"
#include "calendar/calendar_instance.h"

namespace extension {
namespace calendar {

typedef std::unique_ptr<std::remove_pointer<calendar_filter_h>::type,
                        void (*)(calendar_filter_h)> CalendarFilterPtr;

void CalendarFilterDeleter(calendar_filter_h calendar_filter) {
  if (CALENDAR_ERROR_NONE != calendar_filter_destroy(calendar_filter)) {
    LoggerE("failed to destroy contacts_filter_h");
  }
}

using namespace common;

Calendar::Calendar(CalendarInstance& instance)
    : current_db_version_(0),
      instance_(instance) {
}

Calendar::~Calendar() {
  int ret;

  if (listeners_registered_.find("EVENT") != listeners_registered_.end()) {
    ret = calendar_db_remove_changed_cb(_calendar_event._uri, ChangeCallback,
                                        this);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Remove calendar event change callback error");
    }
  }

  if (listeners_registered_.find("TASK") != listeners_registered_.end()) {
    ret = calendar_db_remove_changed_cb(_calendar_todo._uri, ChangeCallback,
                                        this);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Remove calendar todo change callback error");
    }
  }
}

PlatformResult Calendar::Get(const picojson::object& args, picojson::object& out) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  int calendar_id = common::stol(FromJson<std::string>(args, "calendarId"));

  calendar_record_h handle = nullptr;
  PlatformResult status =
      CalendarRecord::GetById(calendar_id, _calendar_book._uri, &handle);
  if (status.IsError()) return status;

  CalendarRecordPtr calendar_ptr =
      CalendarRecordPtr(handle, CalendarRecord::Deleter);

  int type;
  status = CalendarRecord::GetInt(calendar_ptr.get(), _calendar_book.store_type,
                                  &type);
  if (status.IsError()) return status;

  int id;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id = common::stol(FromJson<std::string>(args, "id", "uid"));
  } else {
    id = common::stol(FromJson<std::string>(args, "id"));
  }

  std::string uri;
  status = CalendarRecord::TypeToUri(type, &uri);
  if (status.IsError()) return status;

  calendar_record_h handle_uri = nullptr;
  status = CalendarRecord::GetById(id, uri.c_str(), &handle_uri);
  if (status.IsError()) return status;

  CalendarRecordPtr record_ptr =
      CalendarRecordPtr(handle_uri, CalendarRecord::Deleter);

  status = CalendarItem::ToJson(type, record_ptr.get(), &out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::Add(const picojson::object& args,
                             picojson::object& out) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  const auto& item = FromJson<picojson::object>(args, "item");
  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));

  calendar_record_h handle = nullptr;
  PlatformResult status = CalendarItem::Create(type, &handle);
  if (status.IsError()) return status;

  CalendarRecordPtr item_ptr =
      CalendarRecordPtr(handle, CalendarRecord::Deleter);

  status = CalendarItem::FromJson(type, item_ptr.get(), item);
  if (status.IsError()) return status;

  int record_id;
  status = CalendarRecord::Insert(item_ptr.get(), &record_id);
  if (status.IsError()) return status;

  out.insert(std::make_pair("uid", picojson::value(std::to_string(record_id))));

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    std::string rid;
    PlatformResult status = CalendarRecord::GetString(
        item_ptr.get(), _calendar_event.recurrence_id, &rid);
    if (status.IsError()) return status;

    if (!rid.empty()) {
      out["rid"] = picojson::value(rid);
    } else {
      out["rid"] = picojson::value();
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::AddBatch(const picojson::object& args,
                                  picojson::array& array) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  auto& items = FromJson<picojson::array>(args, "items");
  if (items.empty()) {
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "No items");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  std::string view_uri;
  PlatformResult status = CalendarRecord::TypeToUri(type, &view_uri);
  if (status.IsError()) return status;

  calendar_list_h list = NULL;
  if (CALENDAR_ERROR_NONE != calendar_list_create(&list)) {
    LoggerE("Could not create list for batch operation");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Could not create list for batch operation");
  }
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

  int ret;
  calendar_record_h record;

  for (auto& item : items) {
    ret = calendar_record_create(view_uri.c_str(), &record);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't create platform record %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Can't create platform record");
    }
    PlatformResult status =
        CalendarItem::FromJson(type, record, item.get<picojson::object>());
    if (status.IsError()) return status;

    if (CALENDAR_ERROR_NONE != calendar_list_add(list_ptr.get(), record)) {
      LoggerE("Could not add record to list events");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Could not add record to list");
    }
  }

  int* ids;
  int count;
  ret = calendar_db_insert_records(list_ptr.get(), &ids, &count);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("calendar_db_insert_records failed.");
    if (CALENDAR_ERROR_INVALID_PARAMETER == ret) {
      LoggerE("CALENDAR_ERROR_INVALID_PARAMETER.");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Parameter is invalid");
    } else {
      LoggerE("CALENDAR_ERROR_DB_FAILED");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "CALENDAR_ERROR_DB_FAILED occurred");
    }
  }

  for (int i = 0; i < count; i++) {
    picojson::value id = picojson::value(picojson::object());
    picojson::object& id_obj = id.get<picojson::object>();

    id_obj.insert(std::make_pair("uid", picojson::value(std::to_string(ids[i]))));

    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      id_obj.insert(std::make_pair("rid", picojson::value()));
    }

    array.push_back(id);
  }
  free(ids);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::Update(const picojson::object& args,
                                picojson::object& /*out*/) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  const auto& item = FromJson<picojson::object>(args, "item");
  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));

  bool update_all = true;

  int id;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id = common::stol(FromJson<std::string>(item, "id", "uid"));
    if (!IsNull(args, "updateAllInstances")) {
      update_all = FromJson<bool>(args, "updateAllInstances");
    }
  } else {
    id = common::stol(FromJson<std::string>(item, "id"));
  }

  std::string value_str;
  PlatformResult status = CalendarRecord::TypeToUri(type, &value_str);
  if (status.IsError()) return status;

  calendar_record_h handle = nullptr;
  status = CalendarRecord::GetById(id, value_str.c_str(), &handle);
  if (status.IsError()) return status;

  CalendarRecordPtr record_ptr =
      CalendarRecordPtr(handle, CalendarRecord::Deleter);

  status = CalendarItem::FromJson(type, record_ptr.get(), item);
  if (status.IsError()) return status;

  if (type == CALENDAR_BOOK_TYPE_TODO || update_all ||
      common::IsNull(item, "recurrenceRule")) {
    if (CALENDAR_ERROR_NONE != calendar_db_update_record(record_ptr.get())) {
      LoggerE("Can't update calendar item");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Can't update calendar item");
    }
  } else {
    // first update the parent event
    std::string exdate =
        CalendarItem::ExceptionsFromJson(common::FromJson<picojson::array>(
            item, "recurrenceRule", "exceptions"));
    if (!common::IsNull(common::FromJson<picojson::object>(item, "id"),
                        "rid")) {
      exdate.append(common::FromJson<std::string>(item, "id", "rid"));
    }
    PlatformResult status = CalendarRecord::SetString(
        record_ptr.get(), _calendar_event.exdate, exdate);
    if (status.IsError()) return status;

    // don't set the recurrence id for the parent event
    status = CalendarRecord::SetString(record_ptr.get(),
                                       _calendar_event.recurrence_id, "");
    if (status.IsError()) return status;

    if (CALENDAR_ERROR_NONE != calendar_db_update_record(record_ptr.get())) {
      LoggerE("Can't update calendar item");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Can't update calendar item");
    }

    // now add the detached child event
    calendar_record_h handle_new = nullptr;
    status = CalendarItem::Create(type, &handle_new);
    if (status.IsError()) return status;

    CalendarRecordPtr item_ptr =
        CalendarRecordPtr(handle_new, CalendarRecord::Deleter);

    status = CalendarItem::FromJson(type, item_ptr.get(), item);
    if (status.IsError()) return status;

    int record_id;
    status = CalendarRecord::Insert(item_ptr.get(), &record_id);
    if (status.IsError()) return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::UpdateBatch(const picojson::object& args,
                                     picojson::array& array) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  auto& items = FromJson<picojson::array>(args, "items");
  if (items.empty()) {
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "No items");
  }

  bool update_all = true;
  if (!IsNull(args, "updateAllInstances")) {
    update_all = FromJson<bool>(args, "updateAllInstances");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  std::string view_uri;
  PlatformResult status = CalendarRecord::TypeToUri(type, &view_uri);
  if (status.IsError()) return status;

  calendar_list_h list = NULL;
  if (CALENDAR_ERROR_NONE != calendar_list_create(&list)) {
    LoggerE("Could not create list for batch operation");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Could not create list for batch operation");
  }
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

  int id;
  calendar_record_h record;

  for (auto& item : items) {
    const picojson::object& item_obj = item.get<picojson::object>();
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      id = common::stol(FromJson<std::string>(item_obj, "id", "uid"));
    } else {
      id = common::stol(FromJson<std::string>(item_obj, "id"));
    }

    int ret = calendar_db_get_record(view_uri.c_str(), id, &record);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't get platform record %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Can't get platform record");
    }
    PlatformResult status =
        CalendarItem::FromJson(type, record, item.get<picojson::object>());
    if (status.IsError()) return status;

    if (CALENDAR_ERROR_NONE != calendar_list_add(list_ptr.get(), record)) {
      LoggerE("Could not add record to list events");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Could not add record to list");
    }
  }

  if (type == CALENDAR_BOOK_TYPE_TODO || update_all) {
    if (CALENDAR_ERROR_NONE != calendar_db_update_records(list_ptr.get())) {
      LoggerE("Can't update calendar items");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Can't update calendar items");
    }
  } else {
    // @todo update the exdate for a recurring parent event and add a new
    // child event
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::Remove(const picojson::object& args,
                                picojson::object& out) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));

  int id;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id = common::stol(FromJson<std::string>(args, "id", "uid"));
  } else {
    id = common::stol(FromJson<std::string>(args, "id"));
  }

  return CalendarItem::Remove(type, id);
}

PlatformResult Calendar::SetDefaultFilter(calendar_query_h* calendar_query, int type, int id) {
  LoggerD("Entered");

  const long UNIFIED_CALENDAR_ID = 0;
  int error_code = 0;
  PlatformResult status = PlatformResult(ErrorCode::NO_ERROR);
  calendar_filter_h calendar_filter = nullptr;
  const char* view_uri =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? _calendar_event._uri : _calendar_todo._uri;
  const int book_id =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? _calendar_event.calendar_book_id : _calendar_todo.calendar_book_id;
  const int is_deleted =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? _calendar_event.is_deleted : _calendar_todo.is_deleted;

  error_code = calendar_filter_create(view_uri, &calendar_filter);
  if ((status = ErrorChecker(error_code)).IsError()) return status;

  CalendarFilterPtr calendar_filter_ptr(calendar_filter, CalendarFilterDeleter);

  calendar_match_int_flag_e match_int_flag = CALENDAR_MATCH_EQUAL;
  if (CALENDAR_BOOK_FILTER_ALL == id || UNIFIED_CALENDAR_ID == id) {
    match_int_flag = CALENDAR_MATCH_GREATER_THAN;
  }

  error_code = calendar_filter_add_int(calendar_filter, book_id, match_int_flag, id);
  if ((status = ErrorChecker(error_code)).IsError()) return status;

  error_code = calendar_filter_add_operator(calendar_filter, CALENDAR_FILTER_OPERATOR_AND);
  if ((status = ErrorChecker(error_code)).IsError()) return status;

  error_code = calendar_filter_add_int(calendar_filter, is_deleted, CALENDAR_MATCH_EQUAL, 0);
  if ((status = ErrorChecker(error_code)).IsError()) return status;

  error_code = calendar_query_set_filter(*calendar_query, calendar_filter);
  status = ErrorChecker(error_code);

  return status;
}

PlatformResult Calendar::Find(const picojson::object& args, picojson::array& array) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }
  int calendar_id = common::stol(FromJson<std::string>(args, "calendarId"));
  int error_code = 0;
  calendar_record_h handle = nullptr;
  PlatformResult status =
      CalendarRecord::GetById(calendar_id, _calendar_book._uri, &handle);
  if (status.IsError()) return status;

  CalendarRecordPtr calendar_ptr =
      CalendarRecordPtr(handle, CalendarRecord::Deleter);

  int type;
  status = CalendarRecord::GetInt(calendar_ptr.get(), _calendar_book.store_type,
                                  &type);
  if (status.IsError()) return status;
  const char* view_uri = (type == CALENDAR_BOOK_TYPE_EVENT) ? _calendar_event._uri : _calendar_todo._uri;

  calendar_query_h calendar_query = nullptr;

  error_code = calendar_query_create(view_uri, &calendar_query);
  if ((status = ErrorChecker(error_code)).IsError()) return status;

  CalendarQueryPtr calendar_query_ptr(calendar_query,
                                      CalendarRecord::QueryDeleter);

  std::vector<std::vector<CalendarFilterPtr>> intermediate_filters(1);
  if (!IsNull(args, "filter")) {
    FilterVisitor visitor;
    visitor.SetOnAttributeFilter([&](const std::string& name,
                                     AttributeMatchFlag match_flag,
                                     const picojson::value& match_value) {
      int value = 0;
      calendar_filter_h calendar_filter = nullptr;

      error_code = calendar_filter_create(view_uri, &calendar_filter);
      if ((status = ErrorChecker(error_code)).IsError()) return status;

      CalendarFilterPtr calendar_filter_ptr(calendar_filter,
                                            CalendarFilterDeleter);

      unsigned int propertyId = 0;
      if (name == "startDate" || name == "endDate" || name == "dueDate") {
        PlatformResult status = CalendarItem::GetPlatformProperty(
            type, name + "_time", &propertyId);
        if (status.IsError()) return status;
      } else {
        PlatformResult status =
            CalendarItem::GetPlatformProperty(type, name, &propertyId);
        if (status.IsError()) return status;
      }

      if (name == "id" || name == "id.uid") {
        if (type == CALENDAR_BOOK_TYPE_EVENT && name == "id") {
          value = common::stol(
              FromJson<std::string>(JsonCast<JsonObject>(match_value), "uid"));
        } else {
          value = common::stol(JsonCast<std::string>(match_value));
        }
        if (value < 0) {
          return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                                "Match value cannot be less than 0");
        }
        calendar_match_int_flag_e flag;
        if (AttributeMatchFlag::kExists == match_flag) {
          flag = CALENDAR_MATCH_GREATER_THAN_OR_EQUAL;
          value = 0;
        } else if (AttributeMatchFlag::kStartsWith == match_flag ||
                   AttributeMatchFlag::kContains == match_flag) {
          flag = CALENDAR_MATCH_GREATER_THAN_OR_EQUAL;
        } else if (AttributeMatchFlag::kEndsWith == match_flag) {
          flag = CALENDAR_MATCH_LESS_THAN_OR_EQUAL;
        } else {
          flag = CALENDAR_MATCH_EQUAL;
        }
        error_code =
            calendar_filter_add_int(calendar_filter, propertyId, flag, value);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
      } else if (name == "startDate" || name == "endDate" ||
                 name == "dueDate") {
        calendar_match_int_flag_e flag;
        Date dateTofilter =
            CalendarItem::DateFromJson(JsonCast<picojson::object>(match_value));
        if (AttributeMatchFlag::kExists == match_flag) {
          flag = CALENDAR_MATCH_GREATER_THAN_OR_EQUAL;
          value = 0;
        } else if (AttributeMatchFlag::kStartsWith == match_flag ||
                   AttributeMatchFlag::kContains == match_flag) {
          flag = CALENDAR_MATCH_GREATER_THAN_OR_EQUAL;
        } else if (AttributeMatchFlag::kEndsWith == match_flag) {
          flag = CALENDAR_MATCH_LESS_THAN_OR_EQUAL;
        } else {
          flag = CALENDAR_MATCH_EQUAL;
        }

        error_code = calendar_filter_add_caltime(
            calendar_filter, propertyId, flag,
            CalendarItem::DateToPlatform(dateTofilter, false));
        if ((status = ErrorChecker(error_code)).IsError()) return status;
      } else  if (name == "isAllDay" || name == "isDetached") {
        calendar_match_int_flag_e flag = CALENDAR_MATCH_EQUAL;

        if (match_value.is<bool>()) {
          if(match_value.get<bool>()) {
            value = 1;
          } else {
            value = 0;
          }
        } else {
          value = 0;
        }

        error_code =
        calendar_filter_add_int(calendar_filter, propertyId, flag, value);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
      } else {
        std::string value = JsonCast<std::string>(match_value);
        calendar_match_str_flag_e flag = CALENDAR_MATCH_EXISTS;
        if (AttributeMatchFlag::kExactly == match_flag) {
          flag = CALENDAR_MATCH_EXACTLY;
        } else if (AttributeMatchFlag::kFullString == match_flag) {
          flag = CALENDAR_MATCH_FULLSTRING;
        } else if (AttributeMatchFlag::kContains == match_flag) {
          flag = CALENDAR_MATCH_CONTAINS;
        } else if (AttributeMatchFlag::kStartsWith == match_flag) {
          flag = CALENDAR_MATCH_STARTSWITH;
        } else if (AttributeMatchFlag::kEndsWith == match_flag) {
          flag = CALENDAR_MATCH_ENDSWITH;
        } else if (AttributeMatchFlag::kExists == match_flag) {
          flag = CALENDAR_MATCH_EXISTS;
          value = "";
        }
        calendar_filter_add_str(calendar_filter, propertyId, flag,
                                value.c_str());
      }
      intermediate_filters[intermediate_filters.size() - 1].push_back(
          std::move(calendar_filter_ptr));

      return PlatformResult(ErrorCode::NO_ERROR);
    });
    visitor.SetOnCompositeFilterBegin([&](CompositeFilterType type) {
      intermediate_filters.push_back(std::vector<CalendarFilterPtr>());

      return PlatformResult(ErrorCode::NO_ERROR);
    });

    visitor.SetOnCompositeFilterEnd([&](CompositeFilterType calType) {
      if (intermediate_filters.size() == 0) {
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Reached stack size equal to 0!");
      }
      calendar_filter_h merged_filter = nullptr;

      error_code = calendar_filter_create(view_uri, &merged_filter);
      if ((status = ErrorChecker(error_code)).IsError()) return status;

      CalendarFilterPtr merged_filter_ptr(merged_filter, CalendarFilterDeleter);
      for (std::size_t i = 0; i < intermediate_filters.back().size(); ++i) {
        error_code = calendar_filter_add_filter(
            merged_filter, intermediate_filters.back().at(i).get());
        if ((status = ErrorChecker(error_code)).IsError()) return status;
        if (CompositeFilterType::kIntersection == calType) {
          error_code = calendar_filter_add_operator(
              merged_filter, CALENDAR_FILTER_OPERATOR_AND);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        } else if (CompositeFilterType::kUnion == calType) {
          error_code = calendar_filter_add_operator(
              merged_filter, CALENDAR_FILTER_OPERATOR_OR);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        } else {
          return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                                "Invalid union type!");
        }
      }
      intermediate_filters.pop_back();
      intermediate_filters.back().push_back(std::move(merged_filter_ptr));

      return PlatformResult(ErrorCode::NO_ERROR);
    });

    visitor.SetOnAttributeRangeFilter([&](const std::string& name,
                                          const JsonValue& initial_value,
                                          const JsonValue& end_value) {
      unsigned int propertyId = 0;
      if (name == "startDate" || name == "endDate" || name == "dueDate") {
        PlatformResult status = CalendarItem::GetPlatformProperty(
            type, name + "_time", &propertyId);
        if (status.IsError()) return status;
      } else {
        PlatformResult status =
            CalendarItem::GetPlatformProperty(type, name, &propertyId);
        if (status.IsError()) return status;
      }

      calendar_filter_h calendar_filter = nullptr;
      int error_code = 0;

      error_code = calendar_filter_create(view_uri, &calendar_filter);
      if ((status = ErrorChecker(error_code)).IsError()) return status;

      CalendarFilterPtr calendar_filter_ptr(calendar_filter,
                                            CalendarFilterDeleter);

      bool initial_value_exists = (!IsNull(initial_value));
      bool end_value_exists = (!IsNull(end_value));
      if (name == "id") {
        int initial_value_date = 0;
        int end_value_date = 0;

        if (initial_value_exists)
          initial_value_date =
              common::stol(JsonCast<std::string>(initial_value));
        if (end_value_exists)
          end_value_date = common::stol(JsonCast<std::string>(end_value));

        if (initial_value_exists && end_value_exists) {
          calendar_filter_h sub_filter = NULL;

          error_code = calendar_filter_create(view_uri, &sub_filter);
          if ((status = ErrorChecker(error_code)).IsError()) return status;

          CalendarFilterPtr sub_filter_ptr(sub_filter, CalendarFilterDeleter);

          error_code = calendar_filter_add_int(
              sub_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              initial_value_date);
          if ((status = ErrorChecker(error_code)).IsError()) return status;

          error_code = calendar_filter_add_operator(
              sub_filter, CALENDAR_FILTER_OPERATOR_AND);
          if ((status = ErrorChecker(error_code)).IsError()) return status;

          error_code = calendar_filter_add_int(
              sub_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              end_value_date);
          if ((status = ErrorChecker(error_code)).IsError()) return status;

          error_code = calendar_filter_add_filter(calendar_filter, sub_filter);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        } else if (initial_value_exists) {
          error_code = calendar_filter_add_int(
              calendar_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              initial_value_date);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        } else if (end_value_exists) {
          error_code = calendar_filter_add_int(
              calendar_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              end_value_date);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        }
      } else if (name == "startDate" || name == "dueDate" ||
                 name == "endDate") {
        Date initial_value_date;
        Date end_value_date;

        if (initial_value_exists)
          initial_value_date = CalendarItem::DateFromJson(
              JsonCast<picojson::object>(initial_value));
        if (end_value_exists)
          end_value_date =
              CalendarItem::DateFromJson(JsonCast<picojson::object>(end_value));

        calendar_filter_h normal_filter = nullptr;
        calendar_filter_h all_day_filter = nullptr;

        error_code = calendar_filter_create(view_uri, &normal_filter);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
        CalendarFilterPtr normal_filter_ptr(normal_filter, CalendarFilterDeleter);

        error_code = calendar_filter_create(view_uri, &all_day_filter);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
        CalendarFilterPtr all_day_filter_ptr(all_day_filter, CalendarFilterDeleter);

        if (initial_value_exists) {
          error_code = calendar_filter_add_caltime(
              normal_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(initial_value_date, false));
          if ((status = ErrorChecker(error_code)).IsError()) return status;
          error_code = calendar_filter_add_caltime(
              all_day_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(initial_value_date, true));
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        }

        if (initial_value_exists && end_value_exists) {
          error_code = calendar_filter_add_operator(
              normal_filter, CALENDAR_FILTER_OPERATOR_AND);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
          error_code = calendar_filter_add_operator(
              all_day_filter, CALENDAR_FILTER_OPERATOR_AND);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        }

        if (end_value_exists) {
          error_code = calendar_filter_add_caltime(
              normal_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(end_value_date, false));
          if ((status = ErrorChecker(error_code)).IsError()) return status;
          error_code = calendar_filter_add_caltime(
              all_day_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(end_value_date, true));
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        }

        error_code = calendar_filter_add_filter(calendar_filter, normal_filter);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
        error_code = calendar_filter_add_operator(calendar_filter, CALENDAR_FILTER_OPERATOR_OR);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
        error_code = calendar_filter_add_filter(calendar_filter, all_day_filter);
        if ((status = ErrorChecker(error_code)).IsError()) return status;
      } else {
        std::string initial_value_str;
        std::string end_value_str;

        if (initial_value_exists) {
          initial_value_str = JsonCast<std::string>(initial_value);
        }

        if (end_value_exists) {
          end_value_str = JsonCast<std::string>(end_value);
        }

        if (initial_value_exists && end_value_exists) {
          calendar_filter_h sub_filter = NULL;

          error_code = calendar_filter_create(view_uri, &sub_filter);
          if ((status = ErrorChecker(error_code)).IsError()) return status;

          CalendarFilterPtr sub_filter_ptr(sub_filter, CalendarFilterDeleter);

          error_code = calendar_filter_add_str(sub_filter, propertyId,
                                               CALENDAR_MATCH_STARTSWITH,
                                               initial_value_str.c_str());
          if ((status = ErrorChecker(error_code)).IsError()) return status;
          error_code = calendar_filter_add_operator(
              sub_filter, CALENDAR_FILTER_OPERATOR_AND);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
          error_code = calendar_filter_add_str(sub_filter, propertyId,
                                               CALENDAR_MATCH_ENDSWITH,
                                               end_value_str.c_str());
          if ((status = ErrorChecker(error_code)).IsError()) return status;
          error_code = calendar_filter_add_filter(calendar_filter, sub_filter);
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        } else if (initial_value_exists) {
          error_code = calendar_filter_add_str(calendar_filter, propertyId,
                                               CALENDAR_MATCH_STARTSWITH,
                                               initial_value_str.c_str());
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        } else if (end_value_exists) {
          error_code = calendar_filter_add_str(calendar_filter, propertyId,
                                               CALENDAR_MATCH_ENDSWITH,
                                               end_value_str.c_str());
          if ((status = ErrorChecker(error_code)).IsError()) return status;
        }
      }
      intermediate_filters[intermediate_filters.size() - 1].push_back(
          std::move(calendar_filter_ptr));

      return PlatformResult(ErrorCode::NO_ERROR);
    });
    visitor.Visit(FromJson<JsonObject>(args, "filter"));
    if ((intermediate_filters.size() != 1) ||
        (intermediate_filters[0].size() != 1)) {
      LoggerE("Bad filter evaluation!");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Bad filter evaluation!");
    }
    error_code = calendar_query_set_filter(calendar_query,
                                           intermediate_filters[0][0].get());
    if ((status = ErrorChecker(error_code)).IsError()) return status;
  } else {
    //filter is not provided so default filter should be set
    status = SetDefaultFilter(&calendar_query, type, calendar_id);
    if (status.IsError()) return status;
  }

  if (!IsNull(args, "sortMode")) {
    picojson::object sortModeObject =
        FromJson<picojson::object>(args, "sortMode");
    unsigned int propertyId = 0;
    std::string attributeName =
        FromJson<std::string>(sortModeObject, "attributeName");
    std::string order = FromJson<std::string>(sortModeObject, "order");
    if (attributeName == "startDate" || attributeName == "dueDate" ||
        attributeName == "endDate") {
      PlatformResult status = CalendarItem::GetPlatformProperty(
          type, attributeName + "_time", &propertyId);
      if (status.IsError()) return status;
    } else {
      PlatformResult status =
          CalendarItem::GetPlatformProperty(type, attributeName, &propertyId);
      if (status.IsError()) return status;
    }

    if (order.empty() || order == "ASC") {
      error_code = calendar_query_set_sort(calendar_query, propertyId, true);
      if ((status = ErrorChecker(error_code)).IsError()) return status;
    } else if (order == "DESC") {
      error_code = calendar_query_set_sort(calendar_query, propertyId, false);
      if ((status = ErrorChecker(error_code)).IsError()) return status;
    }
  }

  calendar_list_h record_list = nullptr;
  error_code =
      calendar_db_get_records_with_query(calendar_query, 0, 0, &record_list);
  if (CALENDAR_ERROR_NONE != error_code) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "calendar_db_get_records_with_query failed");
  }
  CalendarListPtr record_list_ptr(record_list, CalendarRecord::ListDeleter);

  int record_count = 0;
  error_code = calendar_list_get_count(record_list, &record_count);
  if (CALENDAR_ERROR_NONE != error_code) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "calendar_list_get_count failed");
  }
  error_code = calendar_list_first(record_list);
  if (CALENDAR_ERROR_NONE != error_code) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "calendar_list_first failed");
  }

  array.reserve(record_count);
  LoggerD("Found %d records", record_count);
  for (int i = 0; i < record_count; ++i) {
    calendar_record_h current_record = NULL;
    error_code =
        calendar_list_get_current_record_p(record_list, &current_record);
    if (CALENDAR_ERROR_NONE != error_code) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "calendar_list_get_current_record_p failed");
    }
    picojson::value record_obj = picojson::value(picojson::object());
    PlatformResult status = CalendarItem::ToJson(
        type, current_record, &record_obj.get<picojson::object>());
    if (status.IsError()) return status;

    array.push_back(record_obj);

    error_code = calendar_list_next(record_list);
    if (CALENDAR_ERROR_NONE != error_code) {
      LoggerE("calendar_list_next failed (%i/%i)", i, record_count);
      break;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::RemoveBatch(const picojson::object& args,
                                     picojson::array& array) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  auto& ids = FromJson<picojson::array>(args, "ids");
  if (ids.empty()) {
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "No items");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  std::string view_uri;
  PlatformResult status = CalendarRecord::TypeToUri(type, &view_uri);
  if (status.IsError()) return status;

  std::vector<int> ids_to_remove;
  int id;
  for (int i = 0, size = ids.size(); i < size; i++) {
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      id = common::stol(
          FromJson<std::string>(ids.at(i).get<picojson::object>(), "uid"));
    } else {
      id = common::stol(ids.at(i).get<std::string>());
    }

    calendar_record_h handle = nullptr;
    PlatformResult status =
        CalendarItem::GetById(id, view_uri.c_str(), &handle);
    if (status.IsError()) return status;

    CalendarRecordPtr record_ptr =
        CalendarRecordPtr(handle, CalendarRecord::Deleter);

    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      std::string rid;
      status = CalendarRecord::GetString(record_ptr.get(),
                                         _calendar_event.recurrence_id, &rid);
      if (status.IsError()) return status;

      if (rid.empty()) {
        ids_to_remove.push_back(id);
      } else {
        // @todo handle recurrence_id
      }
    } else {
      ids_to_remove.push_back(id);
    }
  }

  if (ids_to_remove.size() > 0) {
    int ret = calendar_db_delete_records(view_uri.c_str(), &ids_to_remove[0],
                                     ids_to_remove.size());

    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("calendar_db_delete_records failed.");
      if (CALENDAR_ERROR_INVALID_PARAMETER == ret) {
        LoggerE("CALENDAR_ERROR_INVALID_PARAMETER");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                              "Parameter is invalid");
      } else {
        LoggerE("CALENDAR_ERROR_DB_FAILED");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "UnknownError");
      }
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::AddChangeListener(const picojson::object& args,
                                           picojson::object& out) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  const std::string& type = FromJson<std::string>(args, "type");
  const std::string& listener_id = FromJson<std::string>(args, "listenerId");

  if (listeners_registered_.find(type) == listeners_registered_.end()) {
    std::string view_uri;
    PlatformResult status = CalendarRecord::TypeToUri(type, &view_uri);
    if (status.IsError()) return status;

    int ret = calendar_db_add_changed_cb(view_uri.c_str(), ChangeCallback, this);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Add calendar change callback error for type %s", type.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Add calendar change callback error");
    }

    ret = calendar_db_get_current_version(&current_db_version_);
    if (CALENDAR_ERROR_NONE != ret) {
      current_db_version_ = 0;
      LoggerE("Can't get calendar db version");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Can't get calendar db version");
    }

    listeners_registered_[type] = listener_id;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Calendar::RemoveChangeListener(const picojson::object& args,
                                              picojson::object& out) {
  if (!CalendarManager::GetInstance().IsConnected()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "DB Connection failed.");
  }

  const std::string& type = FromJson<std::string>(args, "type");

  if (listeners_registered_.find(type) != listeners_registered_.end()) {
    std::string view_uri;
    PlatformResult status = CalendarRecord::TypeToUri(type, &view_uri);
    if (status.IsError()) return status;

    int ret = calendar_db_remove_changed_cb(view_uri.c_str(), ChangeCallback, this);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Remove calendar change callback error for type %s",
              type.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Remove calendar change callback error");
    }
    listeners_registered_.erase(type);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void Calendar::ChangeCallback(const char* view_uri, void* user_data) {
  LoggerD("enter");

  Calendar* c = static_cast<Calendar*>(user_data);

  calendar_list_h list = nullptr;
  int ret, updated_version;
  ret = calendar_db_get_changes_by_version(view_uri, CALENDAR_BOOK_FILTER_ALL,
                                           c->current_db_version_, &list,
                                           &updated_version);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get the changed item list");
    return;
  }
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

  int count;
  ret = calendar_list_get_count(list_ptr.get(), &count);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get the changed item count");
    return;
  }
  LoggerD("Item count: %d", count);

  calendar_list_first(list_ptr.get());

  int id, calendar_id, status;
  calendar_record_h update_info = nullptr;

  int type = CalendarRecord::TypeToInt(view_uri);

  // prepare response object
  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  if (c->listeners_registered_.find("EVENT") != c->listeners_registered_.end())
    response_obj.insert(
        std::make_pair("listenerId", picojson::value(c->listeners_registered_["EVENT"])));
  else
    response_obj.insert(
        std::make_pair("listenerId", picojson::value(c->listeners_registered_["TASK"])));

  picojson::array& added =
      response_obj.insert(std::make_pair("added", picojson::value(picojson::array())))
          .first->second.get<picojson::array>();
  picojson::array& updated =
      response_obj.insert(std::make_pair("updated", picojson::value(picojson::array())))
          .first->second.get<picojson::array>();
  picojson::array& removed =
      response_obj.insert(std::make_pair("removed", picojson::value(picojson::array())))
          .first->second.get<picojson::array>();

  while (count-- > 0) {
    ret = calendar_list_get_current_record_p(list, &update_info);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Can't get current record");
      return;
    }

    PlatformResult result =
        CalendarRecord::GetInt(update_info, _calendar_updated_info.id, &id);
    if (result.IsError()) return;

    result = CalendarRecord::GetInt(
        update_info, _calendar_updated_info.modified_status, &status);
    if (result.IsError()) return;

    if (status == CALENDAR_RECORD_MODIFIED_STATUS_DELETED) {
      result = CalendarRecord::GetInt(
          update_info, _calendar_updated_info.calendar_book_id, &calendar_id);
      if (result.IsError()) return;

      picojson::value removed_row = picojson::value(picojson::object());
      picojson::object& removed_obj = removed_row.get<picojson::object>();
      removed_obj.insert(
          std::make_pair("id", picojson::value(std::to_string(id))));
      removed_obj.insert(std::make_pair(
          "calendarId", picojson::value(std::to_string(calendar_id))));

      removed.push_back(removed_row);

      calendar_list_next(list);
      continue;
    }

    calendar_record_h handle = nullptr;
    result = CalendarRecord::GetById(id, view_uri, &handle);
    if (result.IsError()) return;

    CalendarRecordPtr record_ptr =
        CalendarRecordPtr(handle, CalendarRecord::Deleter);

    picojson::value record_obj = picojson::value(picojson::object());
    result = CalendarItem::ToJson(type, record_ptr.get(),
                                  &record_obj.get<picojson::object>());
    if (result.IsError()) {
      LoggerE("error occured: %s", result.message().c_str());
      return;
    }

    if (status == CALENDAR_RECORD_MODIFIED_STATUS_INSERTED) {
      added.push_back(record_obj);
    } else {
      updated.push_back(record_obj);
    }

    calendar_list_next(list);
  }

  if (added.empty()) {
    response_obj.erase("added");
  }
  if (updated.empty()) {
    response_obj.erase("updated");
  }
  if (removed.empty()) {
    response_obj.erase("removed");
  }

  ret = calendar_db_get_current_version(&updated_version);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get new version");
    return;
  }
  c->current_db_version_ = updated_version;
  LoggerD("-> Calendar::ChangeCallback");
  Instance::PostMessage(&c->instance_, response.serialize().c_str());
}

PlatformResult Calendar::ErrorChecker(int errorCode) {
  if (errorCode != CALENDAR_ERROR_NONE)
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "exception occured");

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace calendar
}  // namespace webapi
