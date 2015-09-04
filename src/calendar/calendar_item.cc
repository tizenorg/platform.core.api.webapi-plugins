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

#include "calendar_item.h"

#include <calendar_view.h>
#include <unicode/ucal.h>

#include "common/logger.h"
#include "common/converter.h"
#include <sstream>
using namespace common;
namespace extension {
namespace calendar {

namespace {
const std::string kTimeDurationUnitMilliseconds = "MSECS";
const std::string kTimeDurationUnitSeconds = "SECS";
const std::string kTimeDurationUnitMinutes = "MINS";
const std::string kTimeDurationUnitHours = "HOURS";
const std::string kTimeDurationUnitDays = "DAYS";

const std::string kDefaultEnumKey = "_DEFAULT";
const std::string kItemVisibility = "ItemVisibility";
const std::string kEventAvailability = "EventAvailability";
const std::string kEventPriority = "EventPriority";
const std::string kTaskPriority = "TaskPriority";
const std::string kEventStatus = "EventStatus";
const std::string kTaskStatus = "TaskStatus";
const std::string kAttendeeRole = "AttendeeRole";
const std::string kAttendeeStatus = "AttendeeStatus";
const std::string kAttendeeType = "AttendeeType";
const std::string kAlarmMethod = "AlarmMethod";
const std::string kRecurrenceRuleFrequency = "RecurrenceRuleFrequency";
}

const PlatformPropertyMap CalendarItem::platform_property_map_ = {
    {"id", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.id},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.id}}},
    {"id.uid", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.id},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.uid}}},
    {"calendar_id", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.calendar_book_id},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.calendar_book_id}}},
    {"description", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.description},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.description}}},
    {"summary", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.summary},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.summary}}},
    {"isAllDay", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.is_allday},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.is_allday}}},
    {"startDate_time", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.start_time},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.start_time}}},
    {"startDate_tzid", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.start_tzid},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.start_tzid}}},
    {"location", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.location},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.location}}},
    {"latitude", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.latitude},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.latitude}}},
    {"longitude", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.longitude},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.longitude}}},
    {"organizer", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.organizer_name},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.organizer_name}}},
    {"visibility", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.sensitivity},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.sensitivity}}},
    {"status", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.event_status},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.todo_status}}},
    {"priority", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.priority},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.priority}}},
    {"categories", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.categories},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.categories}}},
    {"lastModificationDate", {
        {CALENDAR_BOOK_TYPE_EVENT, _calendar_event.last_modified_time},
        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.last_modified_time}}},

    // event only
    {"endDate_time", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.end_time}}},
    {"endDate_tzid", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.end_tzid}}},
    {"recurrence_id", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.recurrence_id}}},
    {"availability", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.busy_status}}},

    // task only
    {"dueDate_time", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.due_time}}},
    {"dueDate_tzid", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.due_tzid}}},
    {"completedDate", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.completed_time}}},
    {"progress", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.progress}}}};

const PlatformEnumMap CalendarItem::platform_enum_map_ = {
    {kItemVisibility, {
        {kDefaultEnumKey, CALENDAR_SENSITIVITY_PUBLIC},
        {"PUBLIC", CALENDAR_SENSITIVITY_PUBLIC},
        {"PRIVATE", CALENDAR_SENSITIVITY_PRIVATE},
        {"CONFIDENTIAL", CALENDAR_SENSITIVITY_CONFIDENTIAL}}},
    {kEventAvailability, {
        {kDefaultEnumKey, CALENDAR_EVENT_BUSY_STATUS_BUSY},
        {"FREE", CALENDAR_EVENT_BUSY_STATUS_FREE},
        {"BUSY", CALENDAR_EVENT_BUSY_STATUS_BUSY},
        {"BUSY-UNAVAILABLE", CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE},
        {"BUSY-TENTATIVE", CALENDAR_EVENT_BUSY_STATUS_TENTATIVE}}},
    {kEventAvailability, {
        {kDefaultEnumKey, CALENDAR_EVENT_BUSY_STATUS_BUSY},
        {"FREE", CALENDAR_EVENT_BUSY_STATUS_FREE},
        {"BUSY", CALENDAR_EVENT_BUSY_STATUS_BUSY},
        {"BUSY-UNAVAILABLE", CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE},
        {"BUSY-TENTATIVE", CALENDAR_EVENT_BUSY_STATUS_TENTATIVE}}},
    {kEventPriority, {
        {kDefaultEnumKey, CALENDAR_EVENT_PRIORITY_NONE},
        {"NONE", CALENDAR_EVENT_PRIORITY_NONE},
        {"LOW", CALENDAR_EVENT_PRIORITY_LOW},
        {"MEDIUM", CALENDAR_EVENT_PRIORITY_NORMAL},
        {"HIGH", CALENDAR_EVENT_PRIORITY_HIGH}}},
    {kTaskPriority, {
        {kDefaultEnumKey, CALENDAR_TODO_PRIORITY_NONE},
        {"NONE", CALENDAR_TODO_PRIORITY_NONE},
        {"LOW", CALENDAR_TODO_PRIORITY_LOW},
        {"MEDIUM", CALENDAR_TODO_PRIORITY_NORMAL},
        {"HIGH", CALENDAR_TODO_PRIORITY_HIGH}}},
    {kEventStatus, {
        {kDefaultEnumKey, CALENDAR_EVENT_STATUS_NONE},
        {"NONE", CALENDAR_EVENT_STATUS_NONE},
        {"TENTATIVE", CALENDAR_EVENT_STATUS_TENTATIVE},
        {"CONFIRMED", CALENDAR_EVENT_STATUS_CONFIRMED},
        {"CANCELLED", CALENDAR_EVENT_STATUS_CANCELLED}}},
    {kTaskStatus, {
        {kDefaultEnumKey, CALENDAR_TODO_STATUS_NONE},
        {"NONE", CALENDAR_TODO_STATUS_NONE},
        {"NEEDS_ACTION", CALENDAR_TODO_STATUS_NEEDS_ACTION},
        {"COMPLETED", CALENDAR_TODO_STATUS_COMPLETED},
        {"IN_PROCESS", CALENDAR_TODO_STATUS_IN_PROCESS},
        {"CANCELLED", CALENDAR_TODO_STATUS_CANCELED}}},
    {kAttendeeRole, {
        {kDefaultEnumKey, CALENDAR_ATTENDEE_ROLE_CHAIR},
        {"REQ_PARTICIPANT", CALENDAR_ATTENDEE_ROLE_REQ_PARTICIPANT},
        {"OPT_PARTICIPANT", CALENDAR_ATTENDEE_ROLE_OPT_PARTICIPANT},
        {"NON_PARTICIPANT", CALENDAR_ATTENDEE_ROLE_NON_PARTICIPANT},
        {"CHAIR", CALENDAR_ATTENDEE_ROLE_CHAIR}}},
    {kAttendeeStatus, {
        {kDefaultEnumKey, CALENDAR_ATTENDEE_STATUS_PENDING},
        {"PENDING", CALENDAR_ATTENDEE_STATUS_PENDING},
        {"ACCEPTED", CALENDAR_ATTENDEE_STATUS_ACCEPTED},
        {"DECLINED", CALENDAR_ATTENDEE_STATUS_DECLINED},
        {"TENTATIVE", CALENDAR_ATTENDEE_STATUS_TENTATIVE},
        {"DELEGATED", CALENDAR_ATTENDEE_STATUS_DELEGATED},
        {"COMPLETED", CALENDAR_ATTENDEE_STATUS_COMPLETED},
        {"IN_PROCESS", CALENDAR_ATTENDEE_STATUS_IN_PROCESS}, }},
    {kAttendeeType, {
        {kDefaultEnumKey, CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL},
        {"INDIVIDUAL", CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL},
        {"GROUP", CALENDAR_ATTENDEE_CUTYPE_GROUP},
        {"RESOURCE", CALENDAR_ATTENDEE_CUTYPE_RESOURCE},
        {"ROOM", CALENDAR_ATTENDEE_CUTYPE_ROOM},
        {"UNKNOWN", CALENDAR_ATTENDEE_CUTYPE_UNKNOWN}}},
    {kAlarmMethod, {
        {kDefaultEnumKey, CALENDAR_ALARM_ACTION_AUDIO},
        {"SOUND", CALENDAR_ALARM_ACTION_AUDIO},
        {"DISPLAY", CALENDAR_ALARM_ACTION_DISPLAY}}},
    {kRecurrenceRuleFrequency, {
        {kDefaultEnumKey, CALENDAR_RECURRENCE_NONE},
        {"", CALENDAR_RECURRENCE_NONE},
        {"DAILY", CALENDAR_RECURRENCE_DAILY},
        {"WEEKLY", CALENDAR_RECURRENCE_WEEKLY},
        {"MONTHLY", CALENDAR_RECURRENCE_MONTHLY},
        {"YEARLY", CALENDAR_RECURRENCE_YEARLY}}}};
PlatformEnumReverseMap CalendarItem::platform_enum_reverse_map_ = {};

PlatformResult CalendarItem::Create(int type, calendar_record_h* handle) {
  LoggerD("Enter");
  std::string value_str;
  PlatformResult status = CalendarRecord::TypeToUri(type, &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return CalendarRecord::Create(value_str.c_str(), handle);
}

PlatformResult CalendarItem::Remove(int type, int id) {
  LoggerD("Enter");
  std::string view_uri;
  PlatformResult status = CalendarRecord::TypeToUri(type, &view_uri);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  calendar_record_h handle = nullptr;
  status = GetById(id, view_uri.c_str(), &handle);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  CalendarRecordPtr record = CalendarRecordPtr(handle, CalendarRecord::Deleter);

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    std::string rid;
    PlatformResult status =
        GetString(type, record.get(), "recurrence_id", &rid);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (rid.length() > 0) {
      // @todo remove all occurrences
      LoggerE("Error: TODO: remove all occurrences");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "TODO: remove all occurrences");
    }
  }

  if (CALENDAR_ERROR_NONE != calendar_db_delete_record(view_uri.c_str(), id)) {
    LOGE("Calendar record delete error");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Record deletion error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::GetPlatformProperty(int type,
                                                 const std::string& property,
                                                 unsigned int* value) {
  LoggerD("Enter");
  if (platform_property_map_.find(property) == platform_property_map_.end()) {
    std::string message = std::string("Undefined property ") + property;
    LoggerE("Error: %s", message.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }

  auto prop = platform_property_map_.at(property);
  if (prop.find(type) == prop.end()) {
    LoggerE("Property %s not defined for type %d", property.c_str(), type);
    return PlatformResult(
        ErrorCode::INVALID_VALUES_ERR,
        std::string("Property %s not defined for type ", property.c_str()) +
        std::to_string(type));
  }

  *value = prop.at(type);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::StringToPlatformEnum(const std::string& field,
                                                  const std::string& value,
                                                  int* platform_enum) {
  LoggerD("Enter");
  auto iter = platform_enum_map_.find(field);
  if (iter == platform_enum_map_.end()) {
    std::string message = std::string("Undefined platform enum type ") + field;
    LoggerE("Error: %s", message.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }

  auto def = platform_enum_map_.at(field);
  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    *platform_enum = def_iter->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  // default value - if any
  def_iter = def.find("_DEFAULT");
  if (def_iter != def.end()) {
    *platform_enum = def_iter->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::string message =
      "Platform enum value " + value + " not found for " + field;
  LoggerE("Error: %s", message.c_str());
  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

PlatformResult CalendarItem::PlatformEnumToString(const std::string& field,
                                                  int value,
                                                  std::string* platform_str) {
  LoggerD("Enter");
  // @todo can be replaced by Boost.Bimap
  if (platform_enum_reverse_map_.empty()) {
    for (auto& def : platform_enum_map_) {
      platform_enum_reverse_map_[def.first] = {};

      for (auto& key : def.second) {
        if (key.first != kDefaultEnumKey) {
          platform_enum_reverse_map_[def.first][key.second] = key.first;
        }
      }
    }
  }

  auto iter = platform_enum_reverse_map_.find(field);
  if (iter == platform_enum_reverse_map_.end()) {
    std::string message = std::string("Undefined platform enum type ") + field;
    LoggerE("Error: %s", message.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }

  auto def = platform_enum_reverse_map_.at(field);
  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    *platform_str = def_iter->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::string message = "Platform enum value " + std::to_string(value) +
      " not found for " + field;
  LoggerE("Error: %s", message.c_str());
  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

PlatformResult CalendarItem::SetString(int type, calendar_record_h rec,
                                       const std::string& property,
                                       const picojson::object& in,
                                       bool optional) {
  LoggerD("set: %s", property.c_str());

  if (optional && IsNull(in, property.c_str())) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  const std::string& value =
      common::FromJson<std::string>(in, property.c_str());

  return SetString(type, rec, property, value);
}

PlatformResult CalendarItem::SetString(int type, calendar_record_h rec,
                                       const std::string& property,
                                       const std::string& value) {
  LoggerD("set: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (prop != -1u) {
    PlatformResult status = CalendarRecord::SetString(rec, prop, value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::GetString(int type, calendar_record_h rec,
                                       const std::string& property,
                                       std::string* value) {
  LoggerD("get: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return CalendarRecord::GetString(rec, prop, value);
}

PlatformResult CalendarItem::SetInt(int type, calendar_record_h rec,
                                    const std::string& property,
                                    const picojson::object& in, bool optional) {
  LoggerD("set: %s", property.c_str());

  if (optional && IsNull(in, property.c_str())) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int value = common::FromJson<double>(in, property.c_str());

  return SetInt(type, rec, property, value);
}

PlatformResult CalendarItem::SetInt(int type, calendar_record_h rec,
                                    const std::string& property, int value) {
  LoggerD("set: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return CalendarRecord::SetInt(rec, prop, value);
}

PlatformResult CalendarItem::GetInt(int type, calendar_record_h rec,
                                    const std::string& property, int* value) {
  LoggerD("get: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return CalendarRecord::GetInt(rec, prop, value);
}

PlatformResult CalendarItem::SetEnum(int type, calendar_record_h rec,
                                     const std::string& property,
                                     const picojson::object& in,
                                     const std::string& enum_name) {
  LoggerD("Enter");
  std::string value = common::FromJson<std::string>(in, property.c_str());

  int value_int;
  PlatformResult status = StringToPlatformEnum(enum_name, value, &value_int);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = SetInt(type, rec, property, value_int);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::SetEnum(calendar_record_h rec,
                                     unsigned int property,
                                     const std::string& enum_name,
                                     const std::string& value) {
  LoggerD("Enter");
  int value_int;
  PlatformResult status = StringToPlatformEnum(enum_name, value, &value_int);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = CalendarRecord::SetInt(rec, property, value_int);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::GetEnum(int type, calendar_record_h rec,
                                     const std::string& property,
                                     const std::string& enum_name,
                                     std::string* enum_str) {
  int value;
  PlatformResult status = GetInt(type, rec, property, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformEnumToString(enum_name, value, enum_str);
}

PlatformResult CalendarItem::GetEnum(calendar_record_h rec,
                                     unsigned int property,
                                     const std::string& enum_name,
                                     std::string* enum_str) {
  LoggerD("Enter");
  int value;
  PlatformResult status = CalendarRecord::GetInt(rec, property, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformEnumToString(enum_name, value, enum_str);
}

PlatformResult CalendarItem::SetDouble(int type, calendar_record_h rec,
                                       const std::string& property, double value) {
  LoggerD("set: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int ret = calendar_record_set_double(rec, prop, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set double value to record: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Set double to record failed.");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::GetDouble(int type, calendar_record_h rec,
                                       const std::string& property,
                                       double* value) {
  LoggerD("get: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int ret = calendar_record_get_double(rec, prop, value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get double value form record: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get int from record failed.");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::SetCaltime(int type, calendar_record_h rec,
                                        const std::string& property,
                                        calendar_time_s value,
                                        bool throw_on_error) {
  LoggerD("Enter");
  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return SetCaltime(rec, prop, value, throw_on_error);
}

PlatformResult CalendarItem::SetCaltime(calendar_record_h rec,
                                        unsigned int property,
                                        calendar_time_s value,
                                        bool throw_on_error) {
  LoggerD("Enter");
  int ret = calendar_record_set_caltime(rec, property, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set caltime value to record: %d", ret);

    if (throw_on_error) {
      std::string message = "Set caltime to record failed.";
      LoggerE("Error: %s", message.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::GetCaltime(int type, calendar_record_h rec,
                                        const std::string& property,
                                        calendar_time_s* cal_time,
                                        bool throw_on_error) {
  LoggerD("get: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return GetCaltime(rec, prop, cal_time, throw_on_error);
}

PlatformResult CalendarItem::GetCaltime(calendar_record_h rec,
                                        unsigned int property,
                                        calendar_time_s* cal_time,
                                        bool throw_on_error) {
  LoggerD("Enter");
  if (property != -1u) {
    int ret = calendar_record_get_caltime(rec, property, cal_time);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't get calendar_time value form record: %d", ret);
      if (throw_on_error) {
        std::string message = "Can't get calendar_time value form record";
        LoggerE("Error: %s", message.c_str());
        return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
      }
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::SetLli(calendar_record_h rec,
                                    unsigned int property, long long int value,
                                    bool throw_on_error) {
  LoggerD("Enter");
  int ret = calendar_record_set_lli(rec, property, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set long long int value to record: %d", ret);

    if (throw_on_error) {
      std::string message = "Set long long int to record failed.";
      LoggerE("Error: %s", message.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::GetLli(int type, calendar_record_h rec,
                                    const std::string& property,
                                    long long int* lli) {
  LoggerD("get: %s", property.c_str());

  unsigned int prop;
  PlatformResult status = GetPlatformProperty(type, property, &prop);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return GetLli(rec, prop, lli);
}

PlatformResult CalendarItem::GetLli(calendar_record_h rec,
                                    unsigned int property, long long int* value,
                                    bool throw_on_error) {
  LoggerD("Enter");
  int ret = calendar_record_get_lli(rec, property, value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get lli value form record: %d", ret);
    if (throw_on_error) {
      std::string message = "Get lli from record failed.";
      LoggerE("Error: %s", message.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

Date CalendarItem::DateFromJson(const picojson::object& in) {
  LoggerD("json date %s", picojson::value(in).serialize().c_str());

  Date date = {(long long int)common::FromJson<double>(in, "UTCTimestamp"),
               (int)common::FromJson<double>(in, "year"),
               (int)common::FromJson<double>(in, "month"),
               (int)common::FromJson<double>(in, "day"),
               common::FromJson<std::string>(in, "timezone")};

  return date;
}

Date CalendarItem::DateFromJson(const picojson::object& in, const char* obj_name) {
  LoggerD("Enter");
  return DateFromJson(common::FromJson<picojson::object>(in, obj_name));
}

picojson::value CalendarItem::DateToJson(Date* date) {
  LoggerD("timestamp: %lld", date->utc_timestamp_);

  picojson::value date_val = picojson::value(picojson::object());
  picojson::object& date_obj = date_val.get<picojson::object>();

  date_obj["UTCTimestamp"] =
      picojson::value(static_cast<double>(date->utc_timestamp_));
  date_obj["year"] = picojson::value(static_cast<double>(date->year_));
  date_obj["month"] = picojson::value(static_cast<double>(date->month_));
  date_obj["day"] = picojson::value(static_cast<double>(date->day_));
  date_obj["timezone"] = picojson::value(date->time_zone_);

  return date_val;
}

PlatformResult CalendarItem::CategoriesFromJson(int type, calendar_record_h rec,
                                                const picojson::array& value) {
  LoggerD("Enter");
  std::string categories = "";
  for (auto iter = value.begin(); iter != value.end(); ++iter) {
    if (iter == value.begin()) {
      categories.append(iter->get<std::string>().c_str());
    } else {
      categories.append("," + iter->get<std::string>());
    }
  }

  PlatformResult status = SetString(type, rec, "categories", categories);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::CategoriesToJson(int type, calendar_record_h rec,
                                              picojson::array* value) {
  LoggerD("Enter");
  std::string categories;
  PlatformResult status = GetString(type, rec, "categories", &categories);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  *value = StringToArray(categories);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::AttendeesFromJson(int type, calendar_record_h rec,
                                               const picojson::array& value) {
  LoggerD("Enter");
  // Remove the preset child attendees before adding new ones.
  unsigned int property;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    property = _calendar_event.calendar_attendee;
  } else {
    property = _calendar_todo.calendar_attendee;
  }
  RemoveChildRecords(rec, property);

  calendar_record_h attendee;
  for (auto& item : value) {
    const picojson::object& obj = JsonCast<picojson::object>(item);

    int ret = calendar_record_create(_calendar_attendee._uri, &attendee);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Fail to create attendee record, error code: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Fail to create attendee record");
    }

    PlatformResult status =
        CalendarRecord::SetString(attendee, _calendar_attendee.email,
                                  common::FromJson<std::string>(obj, "uri"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (!IsNull(obj, "name")) {
      status =
          CalendarRecord::SetString(attendee, _calendar_attendee.name,
                                    common::FromJson<std::string>(obj, "name"));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

    status = SetEnum(attendee, _calendar_attendee.role, kAttendeeRole,
                     common::FromJson<std::string>(obj, "role"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetEnum(attendee, _calendar_attendee.status, kAttendeeStatus,
                     common::FromJson<std::string>(obj, "status"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = CalendarRecord::SetInt(attendee, _calendar_attendee.rsvp,
                                    common::FromJson<bool>(obj, "RSVP"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetEnum(attendee, _calendar_attendee.cutype, kAttendeeType,
                     common::FromJson<std::string>(obj, "type"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (!IsNull(obj, "group")) {
      status = CalendarRecord::SetString(attendee, _calendar_attendee.group,
                                         common::FromJson<std::string>(obj, "group"));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }
    if (!IsNull(obj, "delegatorURI")) {
      status = CalendarRecord::SetString(
          attendee, _calendar_attendee.delegator_uri,
          common::FromJson<std::string>(obj, "delegatorURI"));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }
    if (!IsNull(obj, "delegateURI")) {
      status = CalendarRecord::SetString(
          attendee, _calendar_attendee.delegatee_uri,
          common::FromJson<std::string>(obj, "delegateURI"));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

    if (!IsNull(obj, "contactRef")) {
      status = CalendarRecord::SetString(
          attendee, _calendar_attendee.uid,
          common::FromJson<std::string>(obj, "contactRef", "contactId"));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      const std::string& address_book =
          common::FromJson<std::string>(obj, "contactRef", "addressBookId");

      status = CalendarRecord::SetInt(attendee, _calendar_attendee.person_id,
                                      common::stol(address_book));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    } else {
      LoggerD("ContactRef not set");
    }

    status = AddChildRecord(rec, property, attendee);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::AttendeesToJson(int type, calendar_record_h rec,
                                             picojson::array* out) {
  LoggerD("Enter");
  unsigned int property;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    property = _calendar_event.calendar_attendee;
  } else {
    property = _calendar_todo.calendar_attendee;
  }

  unsigned int count = 0;
  PlatformResult status = GetChildRecordCount(rec, property, true, &count);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  if (!count) {
    LoggerD("No attendees to set.");
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  calendar_list_h list;
  if (CALENDAR_ERROR_NONE !=
      calendar_record_clone_child_record_list(rec, property, &list)) {
    LoggerE("Can't get attendee list");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Can't get attendee list");
  }
  CalendarListPtr(list, CalendarRecord::ListDeleter);

  calendar_record_h attendee;
  for (unsigned int i = 0; i < count; ++i) {
    LoggerD("Processing the attendee %d", i);

    if (GetChildRecordAt(rec, property, &attendee, i).IsError()) {
      LoggerW("Can't get attendee record");
      continue;
    }

    picojson::value attendee_val = picojson::value(picojson::object());
    picojson::object& attendee_obj = attendee_val.get<picojson::object>();

    std::string value_str;
    PlatformResult status = CalendarRecord::GetString(
        attendee, _calendar_attendee.email, &value_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["uri"] = picojson::value(value_str);

    status = CalendarRecord::GetString(attendee, _calendar_attendee.name,
                                       &value_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["name"] = picojson::value(value_str);

    std::string enum_str;
    status =
        GetEnum(attendee, _calendar_attendee.role, kAttendeeRole, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["role"] = picojson::value(enum_str);

    status = GetEnum(attendee, _calendar_attendee.status, kAttendeeStatus,
                     &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["status"] = picojson::value(enum_str);

    int value_int;
    status =
        CalendarRecord::GetInt(attendee, _calendar_attendee.rsvp, &value_int);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["RSVP"] = picojson::value(static_cast<bool>(value_int));

    status =
        GetEnum(attendee, _calendar_attendee.cutype, kAttendeeType, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["type"] = picojson::value(enum_str);

    status = CalendarRecord::GetString(attendee, _calendar_attendee.group,
                                       &value_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["group"] = picojson::value(value_str);

    status = CalendarRecord::GetString(
        attendee, _calendar_attendee.delegator_uri, &value_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["delegatorURI"] = picojson::value(value_str);

    status = CalendarRecord::GetString(
        attendee, _calendar_attendee.delegatee_uri, &value_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    attendee_obj["delegateURI"] = picojson::value(value_str);

    // contactRef
    std::string contact_id;
    status = CalendarRecord::GetString(attendee, _calendar_attendee.uid,
                                       &contact_id);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    int book_id;
    status = CalendarRecord::GetInt(attendee, _calendar_attendee.person_id,
                                    &book_id);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    attendee_obj["contactRef"] = picojson::value(
        picojson::object{{"contactId", picojson::value(contact_id)},
      {"addressBookId", picojson::value(std::to_string(book_id))}});

    out->push_back(attendee_val);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::AlarmsFromJson(int type, calendar_record_h rec,
                                            const picojson::array& alarms) {
  LoggerD("Enter");
  unsigned int property;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    property = _calendar_event.calendar_alarm;
  } else {
    property = _calendar_todo.calendar_alarm;
  }
  RemoveChildRecords(rec, property);

  calendar_record_h alarm;
  for (auto& item : alarms) {
    LoggerD("alarm: %s", item.serialize().c_str());
    const picojson::object& obj = JsonCast<picojson::object>(item);

    int ret = calendar_record_create(_calendar_alarm._uri, &alarm);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Fail to create attendee record, error code: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Fail to create attendee record");
    }

    int tick_unit = CALENDAR_ALARM_TIME_UNIT_SPECIFIC;
    if (!common::IsNull(obj, "absoluteDate")) {
      Date absolute = DateFromJson(obj, "absoluteDate");
      calendar_time_s absolute_date = DateToPlatform(absolute, false);
      PlatformResult status =
          SetCaltime(alarm, _calendar_alarm.alarm_time, absolute_date);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      status =
          CalendarRecord::SetInt(alarm, _calendar_alarm.tick_unit, tick_unit);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

    if (!common::IsNull(obj, "before")) {
      long long length = common::FromJson<double>(obj, "before", "length");
      const std::string& unit =
          common::FromJson<std::string>(obj, "before", "unit");
      long long tick = 0;
      if (kTimeDurationUnitMilliseconds == unit) {
        tick_unit =
            CALENDAR_ALARM_TIME_UNIT_MINUTE;  // minimum calendar time unit.
        tick = length / 60000;
      } else if (kTimeDurationUnitSeconds == unit) {
        tick_unit = CALENDAR_ALARM_TIME_UNIT_MINUTE;
        tick = length / 60;
      } else if (kTimeDurationUnitMinutes == unit) {
        tick_unit = CALENDAR_ALARM_TIME_UNIT_MINUTE;
        tick = length;
      } else if (kTimeDurationUnitHours == unit) {
        tick_unit = CALENDAR_ALARM_TIME_UNIT_HOUR;
        tick = length;
      } else if (kTimeDurationUnitDays == unit) {
        tick_unit = CALENDAR_ALARM_TIME_UNIT_DAY;
        tick = length;
      } else {
        LoggerW("Wrong alarm time unit: %s", unit.c_str());
      }

      PlatformResult status =
          CalendarRecord::SetInt(alarm, _calendar_alarm.tick, tick);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      status =
          CalendarRecord::SetInt(alarm, _calendar_alarm.tick_unit, tick_unit);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

    const auto it_method = obj.find("method");
    std::string method = "unknown";

    if (obj.end() != it_method && it_method->second.is<std::string>()) {
      method = it_method->second.get<std::string>();
    }

    PlatformResult status =
        SetEnum(alarm, _calendar_alarm.action, kAlarmMethod, method);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = CalendarRecord::SetString(
        alarm, _calendar_alarm.description,
        common::FromJson<std::string>(obj, "description"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = AddChildRecord(rec, property, alarm);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::AlarmsToJson(int type, calendar_record_h rec,
                                          picojson::array* out) {
  LoggerD("Enter");
  unsigned int property;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    property = _calendar_event.calendar_alarm;
  } else {
    property = _calendar_todo.calendar_alarm;
  }

  unsigned int count = 0;
  PlatformResult status = GetChildRecordCount(rec, property, true, &count);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  if (!count) {
    LoggerD("No attendees to set.");
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  calendar_list_h list;
  if (CALENDAR_ERROR_NONE !=
      calendar_record_clone_child_record_list(rec, property, &list)) {
    LoggerW("Can't get alarms list");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Can't get alarms list");
  }
  CalendarListPtr(list, CalendarRecord::ListDeleter);

  int tick, tick_unit;
  calendar_record_h alarm;
  for (unsigned int i = 0; i < count; ++i) {
    LoggerD("Processing the alarm %d", i);

    if (GetChildRecordAt(rec, property, &alarm, i).IsError()) {
      LoggerW("Can't get alarm record");
      continue;
    }

    picojson::value alarm_val = picojson::value(picojson::object());
    picojson::object& alarm_obj = alarm_val.get<picojson::object>();

    PlatformResult status =
        CalendarRecord::GetInt(alarm, _calendar_alarm.tick_unit, &tick_unit);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (tick_unit == CALENDAR_ALARM_TIME_UNIT_SPECIFIC) {
      calendar_time_s result;
      status = GetCaltime(alarm, _calendar_alarm.alarm_time, &result);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      alarm_obj["absoluteDate"] = picojson::value(static_cast<double>(result.time.utime));
    } else {
      status = CalendarRecord::GetInt(alarm, _calendar_alarm.tick, &tick);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      int length = 0;
      std::string unit = kTimeDurationUnitSeconds;
      if (CALENDAR_ALARM_TIME_UNIT_MINUTE == tick_unit) {
        unit = kTimeDurationUnitMinutes;
        length = tick;
      } else if (CALENDAR_ALARM_TIME_UNIT_HOUR == tick_unit) {
        unit = kTimeDurationUnitHours;
        length = tick;
      } else if (CALENDAR_ALARM_TIME_UNIT_DAY == tick_unit) {
        unit = kTimeDurationUnitDays;
        length = tick;
      } else if (CALENDAR_ALARM_TIME_UNIT_WEEK == tick_unit) {
        unit = kTimeDurationUnitDays;
        length = tick * 7;
      } else {
        LoggerW("Wrong tick unit: %d", tick_unit);
      }

      alarm_obj["before"] = picojson::value(
          picojson::object{{"length", picojson::value(static_cast<double>(length))},
                       {"unit", picojson::value(unit)}});
    }

    std::string enum_str;
    status = GetEnum(alarm, _calendar_alarm.action, kAlarmMethod, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    alarm_obj["method"] = picojson::value(enum_str);

    std::string value_str;
    status = CalendarRecord::GetString(alarm, _calendar_alarm.description,
                                       &value_str);
    alarm_obj["description"] = picojson::value(value_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    out->push_back(alarm_val);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::RecurrenceRuleFromJson(
    calendar_record_h rec, const picojson::object& rrule) {

  LoggerD("Enter");
  const std::string& frequency =
      common::FromJson<std::string>(rrule, "frequency");
  PlatformResult status =
      SetEnum(rec, _calendar_event.freq, kRecurrenceRuleFrequency, frequency);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  const unsigned short interval = common::FromJson<double>(rrule, "interval");
  status = CalendarRecord::SetInt(rec, _calendar_event.interval, interval);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  const long occurrence_count =
      common::FromJson<double>(rrule, "occurrenceCount");
  if (-1 != occurrence_count) {
    status =
        CalendarRecord::SetInt(rec, _calendar_event.count, occurrence_count);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = CalendarRecord::SetInt(rec, _calendar_event.range_type,
                                    CALENDAR_RANGE_COUNT);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  if (!common::IsNull(rrule, "untilDate")) {
    Date until = DateFromJson(rrule, "untilDate");
    status = SetCaltime(rec, _calendar_event.until_time,
                        DateToPlatform(until, false));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = CalendarRecord::SetInt(rec, _calendar_event.range_type,
                                    CALENDAR_RANGE_UNTIL);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  const picojson::array& byday_array =
      common::FromJson<picojson::array>(rrule, "daysOfTheWeek");
  std::string byday;
  for (auto iter = byday_array.begin(); iter != byday_array.end(); ++iter) {
    if (iter == byday_array.begin()) {
      byday.append(iter->get<std::string>());
    } else {
      byday.append("," + iter->get<std::string>());
    }
  }
  status = CalendarRecord::SetString(rec, _calendar_event.byday, byday);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  const picojson::array& bysetpos_array =
      common::FromJson<picojson::array>(rrule, "setPositions");
  std::string bysetpos;
  for (auto iter = bysetpos_array.begin(); iter != bysetpos_array.end();
      ++iter) {
    if (iter == bysetpos_array.begin()) {
      bysetpos.append(std::to_string((int)iter->get<double>()));
    } else {
      bysetpos.append("," + iter->get<std::string>());
    }
  }
  status = CalendarRecord::SetString(rec, _calendar_event.bysetpos, bysetpos);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = CalendarRecord::SetString(
      rec, _calendar_event.exdate,
      ExceptionsFromJson(
          common::FromJson<picojson::array>(rrule, "exceptions")));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

std::string CalendarItem::ExceptionsFromJson(const picojson::array &exceptions) {

  LoggerD("Enter");
  std::string result;
  Date date;
  for (auto iter = exceptions.begin(); iter != exceptions.end(); ++iter) {
    date = DateFromJson(iter->get<picojson::object>());
    calendar_time_s exception_date = DateToPlatform(date, false);
    std::stringstream ss;
    ss << exception_date.time.utime;

    if (iter == exceptions.begin()) {
      result.append(ss.str());
    } else {
      result.append("," + ss.str());
    }
  }

  return result;
}

PlatformResult CalendarItem::RecurrenceRuleToJson(calendar_record_h rec,
                                                  picojson::object* out_ptr) {
  LoggerD("Enter");
  picojson::object& out = *out_ptr;

  std::string enum_str;
  PlatformResult status =
      GetEnum(rec, _calendar_event.freq, kRecurrenceRuleFrequency, &enum_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["frequency"] = picojson::value(enum_str);

  int interval;
  status =
      CalendarRecord::GetInt(rec, _calendar_event.interval, &interval);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["interval"] = picojson::value(static_cast<double>(interval));

  int occurrence_count;
  status =
      CalendarRecord::GetInt(rec, _calendar_event.count, &occurrence_count);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["occurrenceCount"] =
      picojson::value(static_cast<double>(occurrence_count));

  calendar_time_s cal = {CALENDAR_TIME_UTIME, {0}};
  calendar_record_get_caltime(rec, _calendar_event.until_time, &cal);
  if (cal.time.utime > 0 && CALENDAR_RECORD_NO_UNTIL != cal.time.utime) {
    Date until = {cal.time.utime, 0, 0, 0, ""};
    out["untilDate"] = DateToJson(&until);
  } else {
    out["untilDate"] = picojson::value();
  }

  std::string value_str;
  status = CalendarRecord::GetString(rec, _calendar_event.byday, &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["daysOfTheWeek"] = picojson::value(StringToArray(value_str));

  status = CalendarRecord::GetString(rec, _calendar_event.bysetpos, &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["setPositions"] = picojson::value(StringToArray(value_str));

  status = CalendarRecord::GetString(rec, _calendar_event.exdate, &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  const picojson::array& exceptions = StringToArray(value_str);
  picojson::array dates = picojson::array();
  for (auto& exception : exceptions) {
    Date date = {common::stol(exception.get<std::string>()), 0, 0, 0, ""};
    dates.push_back(DateToJson(&date));
  }
  out["exceptions"] = picojson::value(dates);

  return PlatformResult(ErrorCode::NO_ERROR);
}

calendar_time_s CalendarItem::DateToPlatform(const Date& date,
                                             bool is_all_day) {
  LoggerD("Enter");
  calendar_time_s cal;

  if (is_all_day) {
    cal.type = CALENDAR_TIME_LOCALTIME;
    cal.time.date = {date.year_, date.month_ + 1, date.day_};
  } else {
    cal.type = CALENDAR_TIME_UTIME;
    cal.time.utime = date.utc_timestamp_;
  }

  return cal;
}

PlatformResult CalendarItem::DateFromPlatform(int type, calendar_record_h rec,
                                              const std::string& property,
                                              Date* date_from_platform) {
  LoggerD("Enter");
  calendar_time_s cal;
  PlatformResult status = GetCaltime(type, rec, property + "_time", &cal);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  std::string tzid;
  status = GetString(type, rec, property + "_tzid", &tzid);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  date_from_platform->utc_timestamp_ = cal.time.utime;
  date_from_platform->year_ = cal.time.date.year;
  date_from_platform->month_ = cal.time.date.month;
  date_from_platform->day_ = cal.time.date.mday;
  date_from_platform->time_zone_ = tzid;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::DateFromPlatform(calendar_record_h rec,
                                              unsigned int property,
                                              Date* date_from_platform) {
  LoggerD("Enter");
  calendar_time_s cal;
  PlatformResult status = GetCaltime(rec, property, &cal);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  date_from_platform->utc_timestamp_ = cal.time.utime;
  date_from_platform->year_ = cal.time.date.year;
  date_from_platform->month_ = cal.time.date.month;
  date_from_platform->day_ = cal.time.date.mday;
  date_from_platform->time_zone_ = "";

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::FromJson(int type, calendar_record_h rec,
                                      const picojson::object& in) {
  LoggerD("Enter");
  if (in.empty()) {
    LoggerE("Empty CalendarItem object.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Empty Calendar object.");
  }

  PlatformResult status = SetString(type, rec, "description", in, true);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = SetString(type, rec, "summary", in, true);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = SetString(type, rec, "location", in, true);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = SetString(type, rec, "organizer", in, true);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int is_all_day = common::FromJson<bool>(in, "isAllDay");
  const std::string& start_property = "startDate";
  const std::string& end_property =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? "endDate" : "dueDate";

  std::string start_label = start_property;
  if (common::IsNull(in, start_property.c_str())
      && !common::IsNull(in, end_property.c_str())) {
    // start date is not set, but end date is present, use it instead
    start_label = end_property;
  }

  if (!common::IsNull(in, start_label.c_str())) {
    Date start = DateFromJson(in, start_label.c_str());

    status = SetCaltime(type, rec, "startDate_time",
                        DateToPlatform(start, is_all_day));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetString(type, rec, "startDate_tzid", start.time_zone_);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  std::string end_label = end_property;
  if (!common::IsNull(in, start_property.c_str())
        && common::IsNull(in, end_property.c_str())) {
    // end date is not set, but start date is present, use it instead
    end_label = start_property;
  }

  if (!common::IsNull(in, end_label.c_str())) {
    Date end = DateFromJson(in, end_label.c_str());

    status = SetCaltime(type, rec, end_property + "_time",
                        DateToPlatform(end, is_all_day));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetString(type, rec, end_property + "_tzid", end.time_zone_);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  status = SetEnum(type, rec, "visibility", in, kItemVisibility);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!common::IsNull(in, "geolocation")) {
    PlatformResult status =
        SetDouble(type, rec, "latitude",
                  common::FromJson<double>(in, "geolocation", "latitude"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status =
        SetDouble(type, rec, "longitude",
                  common::FromJson<double>(in, "geolocation", "longitude"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  status = CategoriesFromJson(
      type, rec, common::FromJson<picojson::array>(in, "categories"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = AttendeesFromJson(
      type, rec, common::FromJson<picojson::array>(in, "attendees"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = AlarmsFromJson(type, rec,
                          common::FromJson<picojson::array>(in, "alarms"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    status = SetEnum(type, rec, "priority", in, kEventPriority);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetEnum(type, rec, "status", in, kEventStatus);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetEnum(type, rec, "availability", in, kEventAvailability);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (!common::IsNull(in, "recurrenceRule")) {
      status = RecurrenceRuleFromJson(
          rec, common::FromJson<picojson::object>(in, "recurrenceRule"));
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

  } else {
    status = SetEnum(type, rec, "priority", in, kTaskPriority);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = SetEnum(type, rec, "status", in, kTaskStatus);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (!common::IsNull(in, "completedDate")) {
      PlatformResult status =
          SetLli(rec, _calendar_todo.completed_time,
                 DateFromJson(in, "completedDate").utc_timestamp_);
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

    PlatformResult status = SetInt(type, rec, "progress", in);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CalendarItem::ToJson(int type, calendar_record_h rec,
                                    picojson::object* out_ptr) {
  LoggerD("Enter");
  if (NULL == rec) {
    LoggerE("Calendar record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Calendar record is null");
  }

  picojson::object& out = *out_ptr;

  int id;
  PlatformResult status = GetInt(type, rec, "id", &id);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  picojson::value id_val;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id_val = picojson::value(picojson::object());
    picojson::object& id_obj = id_val.get<picojson::object>();

    id_obj["uid"] = picojson::value(std::to_string(id));
    std::string rid;
    status = GetString(type, rec, "recurrence_id", &rid);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (rid.length() > 0) {
      id_obj["rid"] = picojson::value(rid);
    } else {
      id_obj["rid"] = picojson::value();
    }
  } else {
    id_val = picojson::value(std::to_string(id));
  }

  out["id"] = id_val;

  int calendar_id;
  status = GetInt(type, rec, "calendar_id", &calendar_id);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["calendarId"] = picojson::value(std::to_string(calendar_id));

  std::string value_str;
  status = GetString(type, rec, "description", &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["description"] = picojson::value(value_str);

  status = GetString(type, rec, "summary", &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["summary"] = picojson::value(value_str);

  status = GetString(type, rec, "location", &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["location"] = picojson::value(value_str);

  status = GetString(type, rec, "organizer", &value_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["organizer"] = picojson::value(value_str);

  int value_int;
  status = GetInt(type, rec, "isAllDay", &value_int);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["isAllDay"] = picojson::value(static_cast<bool>(value_int));

  // startDate
  Date date_from_platform;
  status = DateFromPlatform(type, rec, "startDate", &date_from_platform);
  if (status.IsError())return status;
  out["startDate"] = DateToJson(&date_from_platform);

  // endDate / dueDate
  const std::string& endProperty =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? "endDate" : "dueDate";
  status = DateFromPlatform(type, rec, endProperty, &date_from_platform);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out[endProperty] = DateToJson(&date_from_platform);

  long long int lli;
  status = GetLli(type, rec, "lastModificationDate", &lli);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["lastModificationDate"] = picojson::value(static_cast<double>(lli));

  double latitude;
  status = GetDouble(type, rec, "latitude", &latitude);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  double longitude;
  status = GetDouble(type, rec, "longitude", &longitude);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out["geolocation"] = picojson::value(
      picojson::object({{"latitude", picojson::value(latitude)},
    {"longitude", picojson::value(longitude)}}));

  std::string enum_str;
  status = GetEnum(type, rec, "visibility", kItemVisibility, &enum_str);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["visibility"] = picojson::value(enum_str);

  picojson::array attendees = picojson::array();
  status = AttendeesToJson(type, rec, &attendees);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["attendees"] = picojson::value(attendees);

  picojson::array categories = picojson::array();
  status = CategoriesToJson(type, rec, &categories);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["categories"] = picojson::value(categories);

  picojson::array alarms = picojson::array();
  status = AlarmsToJson(type, rec, &alarms);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }
  out["alarms"] = picojson::value(alarms);

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    status = GetEnum(type, rec, "status", kEventStatus, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["status"] = picojson::value(enum_str);

    status = GetEnum(type, rec, "priority", kEventPriority, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["priority"] = picojson::value(enum_str);

    status = GetEnum(type, rec, "availability", kEventAvailability, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["availability"] = picojson::value(enum_str);

    picojson::object rec_rule = picojson::object();
    status = RecurrenceRuleToJson(rec, &rec_rule);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["recurrenceRule"] = picojson::value(rec_rule);
  } else {
    status = GetEnum(type, rec, "status", kTaskStatus, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["status"] = picojson::value(enum_str);

    status = GetEnum(type, rec, "priority", kTaskPriority, &enum_str);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["priority"] =  picojson::value(enum_str);

    long long int lli;
    status = GetLli(rec, _calendar_todo.completed_time, &lli);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["completedDate"] = picojson::value(static_cast<double>(lli));

    status = GetInt(type, rec, "progress", &value_int);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out["progress"] = picojson::value(static_cast<double>(value_int));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

picojson::array CalendarItem::StringToArray(const std::string& string) {
  LoggerD("Enter");
  picojson::array out = picojson::array();

  size_t cstr_length = string.length() + 1;
  char* cstr = new char[cstr_length];
  strncpy(cstr, string.c_str(), cstr_length);

  char* saveptr = NULL;
  char* pch = strtok_r(cstr, ",", &saveptr);

  while (NULL != pch) {
    out.push_back(picojson::value(std::string(pch)));
    pch = strtok_r(NULL, ",", &saveptr);
  }

  delete[] cstr;

  return out;
}

}  // namespace calendar
}  // namespace extension
