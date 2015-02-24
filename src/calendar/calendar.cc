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

int Calendar::current_db_version_ = 0;
std::map<std::string, std::string> Calendar::listeners_registered_;

Calendar& Calendar::GetInstance() {
  static Calendar instance;
  return instance;
}

Calendar::~Calendar() {
  int ret;

  if (listeners_registered_.find("EVENT") != listeners_registered_.end()) {
    ret = calendar_db_remove_changed_cb(_calendar_event._uri, ChangeCallback,
                                        nullptr);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Remove calendar event change callback error");
    }
  }

  if (listeners_registered_.find("TASK") != listeners_registered_.end()) {
    ret = calendar_db_remove_changed_cb(_calendar_todo._uri, ChangeCallback,
                                        nullptr);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Remove calendar todo change callback error");
    }
  }
}

void Calendar::Get(const picojson::object& args, picojson::object& out) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  int calendar_id = common::stol(FromJson<std::string>(args, "calendarId"));
  CalendarRecordPtr calendar_ptr =
      CalendarRecord::GetById(calendar_id, _calendar_book._uri);
  int type =
      CalendarRecord::GetInt(calendar_ptr.get(), _calendar_book.store_type);

  int id;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id = common::stol(FromJson<std::string>(args, "id", "uid"));
  } else {
    id = common::stol(FromJson<std::string>(args, "id"));
  }

  CalendarRecordPtr record_ptr =
      CalendarRecord::GetById(id, CalendarRecord::TypeToUri(type));
  picojson::value record_obj = picojson::value(picojson::object());
  CalendarItem::ToJson(type, record_ptr.get(), &out);
}

void Calendar::Add(const picojson::object& args, picojson::object& out) {
  LoggerD("enter");
  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  const auto& item = FromJson<picojson::object>(args, "item");
  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));

  CalendarRecordPtr item_ptr = CalendarItem::Create(type);
  CalendarItem::FromJson(type, item_ptr.get(), item);
  int record_id = CalendarRecord::Insert(item_ptr.get());
  out.insert(std::make_pair("uid", std::to_string(record_id)));

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    std::string rid = CalendarRecord::GetString(item_ptr.get(),
                                                _calendar_event.recurrence_id);
    if (!rid.empty()) {
      out["rid"] = picojson::value(rid);
    } else {
      out["rid"] = picojson::value();
    }
  }
}

void Calendar::AddBatch(const picojson::object& args, picojson::array& array) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  auto& items = FromJson<picojson::array>(args, "items");
  if (items.empty()) {
    throw InvalidValuesException("No items");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  const char* view_uri = CalendarRecord::TypeToUri(type);
  calendar_list_h list = NULL;
  if (CALENDAR_ERROR_NONE != calendar_list_create(&list)) {
    LoggerE("Could not create list for batch operation");
    throw UnknownException("Could not create list for batch operation");
  }
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

  int ret;
  calendar_record_h record;

  for (auto& item : items) {
    ret = calendar_record_create(view_uri, &record);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't create platform record %d", ret);
      throw UnknownException("Can't create platform record");
    }
    CalendarItem::FromJson(type, record, item.get<picojson::object>());
    if (CALENDAR_ERROR_NONE != calendar_list_add(list_ptr.get(), record)) {
      LoggerE("Could not add record to list events");
      throw InvalidValuesException("Could not add record to list");
    }
  }

  int* ids;
  int count;
  ret = calendar_db_insert_records(list_ptr.get(), &ids, &count);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("calendar_db_insert_records failed.");
    if (CALENDAR_ERROR_INVALID_PARAMETER == ret) {
      LoggerE("CALENDAR_ERROR_INVALID_PARAMETER.");
      throw InvalidValuesException("Parameter is invalid");
    } else {
      LoggerE("CALENDAR_ERROR_DB_FAILED");
      throw UnknownException("CALENDAR_ERROR_DB_FAILED occurred");
    }
  }

  for (int i = 0; i < count; i++) {
    picojson::value id = picojson::value(picojson::object());
    picojson::object& id_obj = id.get<picojson::object>();

    id_obj.insert(std::make_pair("uid", std::to_string(ids[i])));

    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      id_obj.insert(std::make_pair("rid", picojson::value()));
    }

    array.push_back(id);
  }
  free(ids);
}

void Calendar::Update(const picojson::object& args, picojson::object& /*out*/) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
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

  CalendarRecordPtr record_ptr =
      CalendarRecord::GetById(id, CalendarRecord::TypeToUri(type));
  CalendarItem::FromJson(type, record_ptr.get(), item);

  if (type == CALENDAR_BOOK_TYPE_TODO || update_all ||
      common::IsNull(item, "recurrenceRule")) {
    if (CALENDAR_ERROR_NONE != calendar_db_update_record(record_ptr.get())) {
      LoggerE("Can't update calendar item");
      throw UnknownException("Can't update calendar item");
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
    CalendarRecord::SetString(record_ptr.get(), _calendar_event.exdate, exdate);

    // don't set the recurrence id for the parent event
    CalendarRecord::SetString(record_ptr.get(), _calendar_event.recurrence_id,
                              "");

    if (CALENDAR_ERROR_NONE != calendar_db_update_record(record_ptr.get())) {
      LoggerE("Can't update calendar item");
      throw UnknownException("Can't update calendar item");
    }

    // now add the detached child event
    CalendarRecordPtr item_ptr = CalendarItem::Create(type);
    CalendarItem::FromJson(type, item_ptr.get(), item);
    CalendarRecord::Insert(item_ptr.get());
  }
}

void Calendar::UpdateBatch(const picojson::object& args,
                           picojson::array& array) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  auto& items = FromJson<picojson::array>(args, "items");
  if (items.empty()) {
    throw InvalidValuesException("No items");
  }

  bool update_all = true;
  if (!IsNull(args, "updateAllInstances")) {
    update_all = FromJson<bool>(args, "updateAllInstances");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  const char* view_uri = CalendarRecord::TypeToUri(type);
  calendar_list_h list = NULL;
  if (CALENDAR_ERROR_NONE != calendar_list_create(&list)) {
    LoggerE("Could not create list for batch operation");
    throw UnknownException("Could not create list for batch operation");
  }
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

  int ret, id;
  calendar_record_h record;

  for (auto& item : items) {
    const picojson::object& item_obj = item.get<picojson::object>();
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      id = common::stol(FromJson<std::string>(item_obj, "id", "uid"));
    } else {
      id = common::stol(FromJson<std::string>(item_obj, "id"));
    }

    ret = calendar_db_get_record(view_uri, id, &record);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't get platform record %d", ret);
      throw UnknownException("Can't get platform record");
    }
    CalendarItem::FromJson(type, record, item.get<picojson::object>());

    if (CALENDAR_ERROR_NONE != calendar_list_add(list_ptr.get(), record)) {
      LoggerE("Could not add record to list events");
      throw InvalidValuesException("Could not add record to list");
    }
  }

  if (type == CALENDAR_BOOK_TYPE_TODO || update_all) {
    if (CALENDAR_ERROR_NONE != calendar_db_update_records(list_ptr.get())) {
      LoggerE("Can't update calendar items");
      throw UnknownException("Can't update calendar items");
    }
  } else {
    // @todo update the exdate for a recurring parent event and add a new
    // child event
  }
}

void Calendar::Remove(const picojson::object& args, picojson::object& out) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));

  int id;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id = common::stol(FromJson<std::string>(args, "id", "uid"));
  } else {
    id = common::stol(FromJson<std::string>(args, "id"));
  }

  CalendarItem::Remove(type, id);
}

void Calendar::Find(const picojson::object& args, picojson::array& array) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }
  int calendar_id = common::stol(FromJson<std::string>(args, "calendarId"));
  int error_code = 0;
  CalendarRecordPtr calendar_ptr =
      CalendarRecord::GetById(calendar_id, _calendar_book._uri);
  int type =
      CalendarRecord::GetInt(calendar_ptr.get(), _calendar_book.store_type);
  calendar_query_h calendar_query = nullptr;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    error_code = calendar_query_create(_calendar_event._uri, &calendar_query);
    ErrorChecker(error_code);
  } else {
    error_code = calendar_query_create(_calendar_todo._uri, &calendar_query);
    ErrorChecker(error_code);
  }
  if (CALENDAR_ERROR_NONE != error_code) {
    throw UnknownException("calendar_query_create failed");
  }
  std::vector<std::vector<CalendarFilterPtr>> intermediate_filters(1);
  if (!IsNull(args, "filter")) {
    FilterVisitor visitor;
    visitor.SetOnAttributeFilter([&](const std::string& name,
                                     AttributeMatchFlag match_flag,
                                     const picojson::value& match_value) {
      int value = 0;
      calendar_filter_h calendar_filter = nullptr;
      if (type == CALENDAR_BOOK_TYPE_EVENT) {
        error_code =
            calendar_filter_create(_calendar_event._uri, &calendar_filter);
        ErrorChecker(error_code);
      } else {
        error_code =
            calendar_filter_create(_calendar_todo._uri, &calendar_filter);
        ErrorChecker(error_code);
      }
      CalendarFilterPtr calendar_filter_ptr(calendar_filter,
                                            CalendarFilterDeleter);
      unsigned int propertyId = 0;
      if (name == "startDate" || name == "endDate" || name == "dueDate")
        propertyId = CalendarItem::GetPlatformProperty(type, name + "_time");
      else
        propertyId = CalendarItem::GetPlatformProperty(type, name);
      if (name == "id" || name == "id.uid") {
        if (type == CALENDAR_BOOK_TYPE_EVENT && name == "id") {
          value = common::stol(
              FromJson<std::string>(JsonCast<JsonObject>(match_value), "uid"));
        } else {
          value = common::stol(JsonCast<std::string>(match_value));
        }
        if (value < 0) {
          throw InvalidValuesException("Match value cannot be less than 0");
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
        ErrorChecker(error_code);
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
        ErrorChecker(error_code);
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
        throw UnknownException("Reached stack size equal to 0!");
      }
      calendar_filter_h merged_filter = nullptr;

      if (type == CALENDAR_BOOK_TYPE_EVENT) {
        error_code =
            calendar_filter_create(_calendar_event._uri, &merged_filter);
        ErrorChecker(error_code);
      } else {
        error_code =
            calendar_filter_create(_calendar_todo._uri, &merged_filter);
        ErrorChecker(error_code);
      }
      CalendarFilterPtr merged_filter_ptr(merged_filter, CalendarFilterDeleter);
      for (std::size_t i = 0; i < intermediate_filters.back().size(); ++i) {
        error_code = calendar_filter_add_filter(
            merged_filter, intermediate_filters.back().at(i).get());
        ErrorChecker(error_code);
        if (CompositeFilterType::kIntersection == calType) {
          error_code = calendar_filter_add_operator(
              merged_filter, CALENDAR_FILTER_OPERATOR_AND);
          ErrorChecker(error_code);
        } else if (CompositeFilterType::kUnion == calType) {
          error_code = calendar_filter_add_operator(
              merged_filter, CALENDAR_FILTER_OPERATOR_OR);
          ErrorChecker(error_code);
        } else {
          throw InvalidValuesException("Invalid union type!");
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
        propertyId = CalendarItem::GetPlatformProperty(type, name + "_time");

      } else {
        propertyId = CalendarItem::GetPlatformProperty(type, name);
      }
      calendar_filter_h calendar_filter = nullptr;
      int error_code = 0;
      if (type == CALENDAR_BOOK_TYPE_EVENT) {
        error_code =
            calendar_filter_create(_calendar_event._uri, &calendar_filter);
        ErrorChecker(error_code);
      } else {
        error_code =
            calendar_filter_create(_calendar_todo._uri, &calendar_filter);
        ErrorChecker(error_code);
      }
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

          if (type == CALENDAR_BOOK_TYPE_EVENT) {
            error_code =
                calendar_filter_create(_calendar_event._uri, &sub_filter);
            ErrorChecker(error_code);
          } else {
            error_code =
                calendar_filter_create(_calendar_todo._uri, &sub_filter);
            ErrorChecker(error_code);
          }
          CalendarFilterPtr sub_filter_ptr(sub_filter, CalendarFilterDeleter);

          error_code = calendar_filter_add_int(
              sub_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              initial_value_date);

          error_code = calendar_filter_add_operator(
              sub_filter, CALENDAR_FILTER_OPERATOR_AND);

          error_code = calendar_filter_add_int(
              sub_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              end_value_date);

          error_code = calendar_filter_add_filter(calendar_filter, sub_filter);
          ErrorChecker(error_code);
        } else if (initial_value_exists) {
          error_code = calendar_filter_add_int(
              calendar_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              initial_value_date);
          ErrorChecker(error_code);
        } else if (end_value_exists) {
          error_code = calendar_filter_add_int(
              calendar_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              end_value_date);
          ErrorChecker(error_code);
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

        if (initial_value_exists && end_value_exists) {
          calendar_filter_h sub_filter = NULL;

          if (type == CALENDAR_BOOK_TYPE_EVENT) {
            error_code =
                calendar_filter_create(_calendar_event._uri, &sub_filter);
            ErrorChecker(error_code);
          } else {
            error_code =
                calendar_filter_create(_calendar_todo._uri, &sub_filter);
            ErrorChecker(error_code);
          }
          CalendarFilterPtr sub_filter_ptr(sub_filter, CalendarFilterDeleter);

          error_code = calendar_filter_add_caltime(
              sub_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(initial_value_date, false));
          ErrorChecker(error_code);

          error_code = calendar_filter_add_operator(
              sub_filter, CALENDAR_FILTER_OPERATOR_AND);
          ErrorChecker(error_code);

          error_code = calendar_filter_add_caltime(
              sub_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(end_value_date, false));
          ErrorChecker(error_code);

          error_code = calendar_filter_add_filter(calendar_filter, sub_filter);
          ErrorChecker(error_code);
        } else if (initial_value_exists) {
          error_code = calendar_filter_add_caltime(
              calendar_filter, propertyId, CALENDAR_MATCH_GREATER_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(initial_value_date, false));
          ErrorChecker(error_code);
        } else if (end_value_exists) {
          error_code = calendar_filter_add_caltime(
              calendar_filter, propertyId, CALENDAR_MATCH_LESS_THAN_OR_EQUAL,
              CalendarItem::DateToPlatform(end_value_date, false));
          ErrorChecker(error_code);
        }
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

          if (type == CALENDAR_BOOK_TYPE_EVENT) {
            error_code =
                calendar_filter_create(_calendar_event._uri, &sub_filter);
            ErrorChecker(error_code);
          } else {
            error_code =
                calendar_filter_create(_calendar_todo._uri, &sub_filter);
            ErrorChecker(error_code);
          }
          CalendarFilterPtr sub_filter_ptr(sub_filter, CalendarFilterDeleter);

          error_code = calendar_filter_add_str(sub_filter, propertyId,
                                               CALENDAR_MATCH_STARTSWITH,
                                               initial_value_str.c_str());
          ErrorChecker(error_code);
          error_code = calendar_filter_add_operator(
              sub_filter, CALENDAR_FILTER_OPERATOR_AND);
          ErrorChecker(error_code);
          error_code = calendar_filter_add_str(sub_filter, propertyId,
                                               CALENDAR_MATCH_ENDSWITH,
                                               end_value_str.c_str());
          ErrorChecker(error_code);
          error_code = calendar_filter_add_filter(calendar_filter, sub_filter);
          ErrorChecker(error_code);
        } else if (initial_value_exists) {
          error_code = calendar_filter_add_str(calendar_filter, propertyId,
                                               CALENDAR_MATCH_STARTSWITH,
                                               initial_value_str.c_str());
          ErrorChecker(error_code);
        } else if (end_value_exists) {
          error_code = calendar_filter_add_str(calendar_filter, propertyId,
                                               CALENDAR_MATCH_ENDSWITH,
                                               end_value_str.c_str());
          ErrorChecker(error_code);
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
      throw UnknownException("Bad filter evaluation!");
    }
    error_code = calendar_query_set_filter(calendar_query,
                                           intermediate_filters[0][0].get());
    ErrorChecker(error_code);
  }

  if (!IsNull(args, "sortMode")) {
    picojson::object sortModeObject =
        FromJson<picojson::object>(args, "sortMode");
    unsigned int propertyId = 0;
    std::string attributeName =
        FromJson<std::string>(sortModeObject, "attributeName");
    std::string order = FromJson<std::string>(sortModeObject, "order");
    if (attributeName == "startDate" || attributeName == "dueDate" ||
        attributeName == "endDate")
      propertyId =
          CalendarItem::GetPlatformProperty(type, attributeName + "_time");
    else
      propertyId = CalendarItem::GetPlatformProperty(type, attributeName);
    if (order.empty() || order == "ASC") {
      error_code = calendar_query_set_sort(calendar_query, propertyId, true);
      ErrorChecker(error_code);
    } else if (order == "DESC") {
      error_code = calendar_query_set_sort(calendar_query, propertyId, false);
      ErrorChecker(error_code);
    }
  }

  CalendarQueryPtr calendar_query_ptr(calendar_query,
                                      CalendarRecord::QueryDeleter);

  calendar_list_h record_list = nullptr;
  error_code =
      calendar_db_get_records_with_query(calendar_query, 0, 0, &record_list);
  if (CALENDAR_ERROR_NONE != error_code) {
    throw UnknownException("calendar_db_get_records_with_query failed");
  }
  CalendarListPtr record_list_ptr(record_list, CalendarRecord::ListDeleter);

  int record_count = 0;
  error_code = calendar_list_get_count(record_list, &record_count);
  if (CALENDAR_ERROR_NONE != error_code) {
    throw UnknownException("calendar_list_get_count failed");
  }
  error_code = calendar_list_first(record_list);
  if (CALENDAR_ERROR_NONE != error_code) {
    throw UnknownException("calendar_list_first failed");
  }

  array.reserve(record_count);
  for (int i = 0; i < record_count; ++i) {
    calendar_record_h current_record = NULL;
    error_code =
        calendar_list_get_current_record_p(record_list, &current_record);
    if (CALENDAR_ERROR_NONE != error_code) {
      throw UnknownException("calendar_list_get_current_record_p failed");
    }
    picojson::value record_obj = picojson::value(picojson::object());
    CalendarItem::ToJson(type, current_record,
                         &record_obj.get<picojson::object>());
    array.push_back(record_obj);

    error_code = calendar_list_next(record_list);
    if (CALENDAR_ERROR_NONE != error_code) {
      LoggerE("calendar_list_next failed (%i/%i)", i, record_count);
      break;
    }
  }
}

void Calendar::RemoveBatch(const picojson::object& args,
                           picojson::array& array) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  auto& ids = FromJson<picojson::array>(args, "ids");
  if (ids.empty()) {
    throw InvalidValuesException("No items");
  }

  int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
  const char* view_uri = CalendarRecord::TypeToUri(type);

  std::vector<int> ids_to_remove;
  int id;
  for (int i = 0, size = ids.size(); i < size; i++) {
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      id = common::stol(
          FromJson<std::string>(ids.at(i).get<picojson::object>(), "uid"));
    } else {
      id = common::stol(ids.at(i).get<std::string>());
    }

    CalendarRecordPtr record_ptr = CalendarItem::GetById(id, view_uri);

    if (type == CALENDAR_BOOK_TYPE_EVENT) {
      const std::string& rid = CalendarRecord::GetString(
          record_ptr.get(), _calendar_event.recurrence_id);
      if (rid.empty()) {
        ids_to_remove.push_back(id);
      } else {
        // @todo handle recurrence_id
      }
    } else {
      ids_to_remove.push_back(id);
    }
  }

  int ret;
  if (ids_to_remove.size() > 0) {
    ret = calendar_db_delete_records(view_uri, &ids_to_remove[0],
                                     ids_to_remove.size());

    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("calendar_db_delete_records failed.");
      if (CALENDAR_ERROR_INVALID_PARAMETER == ret) {
        LoggerE("CALENDAR_ERROR_INVALID_PARAMETER");
        throw InvalidValuesException("Parameter is invalid");
      } else {
        LoggerE("CALENDAR_ERROR_DB_FAILED");
        throw UnknownException("UnknownError");
      }
    }
  }
}

void Calendar::AddChangeListener(const picojson::object& args,
                                 picojson::object& out) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  const std::string& type = FromJson<std::string>(args, "type");
  const std::string& listener_id = FromJson<std::string>(args, "listenerId");

  int ret;
  if (listeners_registered_.find(type) == listeners_registered_.end()) {
    const char* view_uri = CalendarRecord::TypeToUri(type);
    ret = calendar_db_add_changed_cb(view_uri, ChangeCallback, nullptr);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Add calendar change callback error for type %s", type.c_str());
      throw UnknownException("Add calendar change callback error");
    }

    ret = calendar_db_get_current_version(&current_db_version_);
    if (CALENDAR_ERROR_NONE != ret) {
      current_db_version_ = 0;
      LoggerE("Can't get calendar db version");
      throw UnknownException("Can't get calendar db version");
    }

    listeners_registered_[type] = listener_id;
  }
}

void Calendar::RemoveChangeListener(const picojson::object& args,
                                    picojson::object& out) {
  LoggerD("enter");

  if (!CalendarManager::GetInstance().IsConnected()) {
    throw UnknownException("DB Connection failed.");
  }

  const std::string& type = FromJson<std::string>(args, "type");

  int ret;
  if (listeners_registered_.find(type) != listeners_registered_.end()) {
    const char* view_uri = CalendarRecord::TypeToUri(type);
    ret = calendar_db_remove_changed_cb(view_uri, ChangeCallback, nullptr);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Remove calendar change callback error for type %s",
              type.c_str());
      throw UnknownException("Remove calendar change callback error");
    }
    listeners_registered_.erase(type);
  }
}

void Calendar::ChangeCallback(const char* view_uri, void*) {
  LoggerD("enter");

  calendar_list_h list = nullptr;
  int ret, updated_version;
  ret = calendar_db_get_changes_by_version(view_uri, CALENDAR_BOOK_FILTER_ALL,
                                           current_db_version_, &list,
                                           &updated_version);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get the changed item list");
    throw UnknownException("Can't get the changed item list");
  }
  CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

  int count;
  ret = calendar_list_get_count(list_ptr.get(), &count);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerE("Can't get the changed item count");
    throw UnknownException("Can't get the changed item count");
  }
  LoggerD("Item count: %d", count);

  calendar_list_first(list_ptr.get());

  int id, calendar_id, status;
  calendar_record_h update_info = nullptr;

  int type = CalendarRecord::TypeToInt(view_uri);

  // prepare response object
  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  if (listeners_registered_.find("EVENT") != listeners_registered_.end())
    response_obj.insert(
        std::make_pair("listenerId", listeners_registered_["EVENT"]));
  else
    response_obj.insert(
        std::make_pair("listenerId", listeners_registered_["TASK"]));

  picojson::array& added =
      response_obj.insert(std::make_pair("added", picojson::array()))
          .first->second.get<picojson::array>();
  picojson::array& updated =
      response_obj.insert(std::make_pair("updated", picojson::array()))
          .first->second.get<picojson::array>();
  picojson::array& removed =
      response_obj.insert(std::make_pair("removed", picojson::array()))
          .first->second.get<picojson::array>();

  while (count-- > 0) {
    ret = calendar_list_get_current_record_p(list, &update_info);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Can't get current record");
      throw UnknownException("Can't get current record");
    }

    id = CalendarRecord::GetInt(update_info, _calendar_updated_info.id);
    status = CalendarRecord::GetInt(update_info,
                                    _calendar_updated_info.modified_status);

    if (status == CALENDAR_RECORD_MODIFIED_STATUS_DELETED) {
      calendar_id = CalendarRecord::GetInt(
          update_info, _calendar_updated_info.calendar_book_id);

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

    try {
      CalendarRecordPtr record_ptr = CalendarRecord::GetById(id, view_uri);
      picojson::value record_obj = picojson::value(picojson::object());
      CalendarItem::ToJson(type, record_ptr.get(),
                           &record_obj.get<picojson::object>());
      if (status == CALENDAR_RECORD_MODIFIED_STATUS_INSERTED) {
        added.push_back(record_obj);
      } else {
        updated.push_back(record_obj);
      }
    } catch (PlatformException& ex) {
      LoggerE("error occured");
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
    throw UnknownException("Can't get new version");
  }
  current_db_version_ = updated_version;
  CalendarInstance::GetInstance().PostMessage(response.serialize().c_str());
}

void Calendar::ErrorChecker(int errorCode) {
  if (errorCode != CALENDAR_ERROR_NONE)
    throw UnknownException("exception occured");
}

}  // namespace calendar
}  // namespace webapi
