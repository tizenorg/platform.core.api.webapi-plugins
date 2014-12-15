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

#include "calendar-item.h"

#include <calendar_view.h>
#include <unicode/ucal.h>

#include "logger.h"
#include "converter.h"

namespace webapi {
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

using namespace webapi::common;

const PlatformPropertyMap CalendarItem::platform_property_map_ = {
    {"id", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.id},
            {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.id}}},
    {"calendar_id",
     {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.calendar_book_id},
      {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.calendar_book_id}}},
    {"description", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.description},
                     {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.description}}},
    {"summary", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.summary},
                 {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.summary}}},
    {"isAllDay", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.is_allday},
                  {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.is_allday}}},
    {"startDate_time", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.start_time},
                        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.start_time}}},
    {"startDate_tzid", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.start_tzid},
                        {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.start_tzid}}},
    {"location", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.location},
                  {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.location}}},
    {"latitude", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.latitude},
                  {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.latitude}}},
    {"longitude", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.longitude},
                   {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.longitude}}},
    {"organizer", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.organizer_name},
                   {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.organizer_name}}},
    {"visibility", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.sensitivity},
                    {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.sensitivity}}},
    {"status", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.event_status},
                {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.todo_status}}},
    {"priority", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.priority},
                  {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.priority}}},
    {"categories", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.categories},
                    {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.categories}}},
    {"lastModificationDate",
     {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.last_modified_time},
      {CALENDAR_BOOK_TYPE_TODO, _calendar_todo.last_modified_time}}},

    // event only
    {"endDate_time", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.end_time}}},
    {"endDate_tzid", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.end_tzid}}},
    {"recurrence_id",
     {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.recurrence_id}}},
    {"availability", {{CALENDAR_BOOK_TYPE_EVENT, _calendar_event.busy_status}}},

    // task only
    {"dueDate_time", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.due_time}}},
    {"dueDate_tzid", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.due_tzid}}},
    {"completedDate",
     {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.completed_time}}},
    {"progress", {{CALENDAR_BOOK_TYPE_TODO, _calendar_todo.progress}}}};

const PlatformEnumMap CalendarItem::platform_enum_map_ = {
    {kItemVisibility, {{kDefaultEnumKey, CALENDAR_SENSITIVITY_PUBLIC},
                       {"PUBLIC", CALENDAR_SENSITIVITY_PUBLIC},
                       {"PRIVATE", CALENDAR_SENSITIVITY_PRIVATE},
                       {"CONFIDENTIAL", CALENDAR_SENSITIVITY_CONFIDENTIAL}}},
    {kEventAvailability,
     {{kDefaultEnumKey, CALENDAR_EVENT_BUSY_STATUS_BUSY},
      {"FREE", CALENDAR_EVENT_BUSY_STATUS_FREE},
      {"BUSY", CALENDAR_EVENT_BUSY_STATUS_BUSY},
      {"BUSY-UNAVAILABLE", CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE},
      {"BUSY-TENTATIVE", CALENDAR_EVENT_BUSY_STATUS_TENTATIVE}}},
    {kEventAvailability,
     {{kDefaultEnumKey, CALENDAR_EVENT_BUSY_STATUS_BUSY},
      {"FREE", CALENDAR_EVENT_BUSY_STATUS_FREE},
      {"BUSY", CALENDAR_EVENT_BUSY_STATUS_BUSY},
      {"BUSY-UNAVAILABLE", CALENDAR_EVENT_BUSY_STATUS_UNAVAILABLE},
      {"BUSY-TENTATIVE", CALENDAR_EVENT_BUSY_STATUS_TENTATIVE}}},
    {kEventPriority, {{kDefaultEnumKey, CALENDAR_EVENT_PRIORITY_NORMAL},
                      {"LOW", CALENDAR_EVENT_PRIORITY_LOW},
                      {"MEDIUM", CALENDAR_EVENT_PRIORITY_NORMAL},
                      {"HIGH", CALENDAR_EVENT_PRIORITY_HIGH}}},
    {kTaskPriority, {{kDefaultEnumKey, CALENDAR_TODO_PRIORITY_NORMAL},
                     {"LOW", CALENDAR_TODO_PRIORITY_LOW},
                     {"MEDIUM", CALENDAR_TODO_PRIORITY_NORMAL},
                     {"HIGH", CALENDAR_TODO_PRIORITY_HIGH}}},
    {kEventStatus, {{kDefaultEnumKey, CALENDAR_EVENT_STATUS_NONE},
                    {"TENTATIVE", CALENDAR_EVENT_STATUS_TENTATIVE},
                    {"CONFIRMED", CALENDAR_EVENT_STATUS_CONFIRMED},
                    {"CANCELLED", CALENDAR_EVENT_STATUS_CANCELLED}}},
    {kTaskStatus, {{kDefaultEnumKey, CALENDAR_TODO_STATUS_NONE},
                   {"NEEDS_ACTION", CALENDAR_TODO_STATUS_NEEDS_ACTION},
                   {"COMPLETED", CALENDAR_TODO_STATUS_COMPLETED},
                   {"IN_PROCESS", CALENDAR_TODO_STATUS_IN_PROCESS},
                   {"CANCELLED", CALENDAR_TODO_STATUS_CANCELED}}},
    {kAttendeeRole,
     {{kDefaultEnumKey, CALENDAR_ATTENDEE_ROLE_CHAIR},
      {"REQ_PARTICIPANT", CALENDAR_ATTENDEE_ROLE_REQ_PARTICIPANT},
      {"OPT_PARTICIPANT", CALENDAR_ATTENDEE_ROLE_OPT_PARTICIPANT},
      {"NON_PARTICIPANT", CALENDAR_ATTENDEE_ROLE_NON_PARTICIPANT},
      {"CHAIR", CALENDAR_ATTENDEE_ROLE_CHAIR}}},
    {kAttendeeStatus, {{kDefaultEnumKey, CALENDAR_ATTENDEE_STATUS_PENDING},
                       {"PENDING", CALENDAR_ATTENDEE_STATUS_PENDING},
                       {"ACCEPTED", CALENDAR_ATTENDEE_STATUS_ACCEPTED},
                       {"DECLINED", CALENDAR_ATTENDEE_STATUS_DECLINED},
                       {"TENTATIVE", CALENDAR_ATTENDEE_STATUS_TENTATIVE},
                       {"DELEGATED", CALENDAR_ATTENDEE_STATUS_DELEGATED},
                       {"COMPLETED", CALENDAR_ATTENDEE_STATUS_COMPLETED},
                       {"IN_PROCESS", CALENDAR_ATTENDEE_STATUS_IN_PROCESS}, }},
    {kAttendeeType, {{kDefaultEnumKey, CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL},
                     {"INDIVIDUAL", CALENDAR_ATTENDEE_CUTYPE_INDIVIDUAL},
                     {"GROUP", CALENDAR_ATTENDEE_CUTYPE_GROUP},
                     {"RESOURCE", CALENDAR_ATTENDEE_CUTYPE_RESOURCE},
                     {"ROOM", CALENDAR_ATTENDEE_CUTYPE_ROOM},
                     {"UNKNOWN", CALENDAR_ATTENDEE_CUTYPE_UNKNOWN}}},
    {kAlarmMethod, {{kDefaultEnumKey, CALENDAR_ALARM_ACTION_AUDIO},
                    {"SOUND", CALENDAR_ALARM_ACTION_AUDIO},
                    {"DISPLAY", CALENDAR_ALARM_ACTION_DISPLAY}}},
    {kRecurrenceRuleFrequency, {{kDefaultEnumKey, CALENDAR_RECURRENCE_NONE},
                                {"", CALENDAR_RECURRENCE_NONE},
                                {"DAILY", CALENDAR_RECURRENCE_DAILY},
                                {"WEEKLY", CALENDAR_RECURRENCE_WEEKLY},
                                {"MONTHLY", CALENDAR_RECURRENCE_MONTHLY},
                                {"YEARLY", CALENDAR_RECURRENCE_YEARLY}}}};
PlatformEnumReverseMap CalendarItem::platform_enum_reverse_map_ = {};

CalendarRecordPtr CalendarItem::Create(int type) {
  LoggerD("enter");

  return CalendarRecord::Create(CalendarRecord::TypeToUri(type));
}

void CalendarItem::Remove(int type, int id) {
  LoggerD("enter");

  const char* view_uri = CalendarRecord::TypeToUri(type);
  CalendarRecordPtr record = GetById(id, view_uri);

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    const std::string& rid = GetString(type, record.get(), "recurrence_id");
    if (rid.length() > 0) {
      // @todo remove all occurrences
      return;
    }
  }

  if (CALENDAR_ERROR_NONE != calendar_db_delete_record(view_uri, id)) {
    LOGE("Calendar record delete error");
    throw UnknownException("Record deletion error");
  }
}

unsigned int CalendarItem::GetPlatformProperty(int type,
                                               const std::string& property) {
  if (platform_property_map_.find(property) == platform_property_map_.end()) {
    throw UnknownException(std::string("Undefined property ") + property);
  }

  auto prop = platform_property_map_.at(property);
  if (prop.find(type) == prop.end()) {
    LoggerD("Property %s not defined for type %d", property.c_str(), type);
    return -1u;
  }

  return prop.at(type);
}

int CalendarItem::StringToPlatformEnum(const std::string& field,
                                       const std::string& value) {
  auto iter = platform_enum_map_.find(field);
  if (iter == platform_enum_map_.end()) {
    throw UnknownException(std::string("Undefined platform enum type ") +
                           field);
  }

  auto def = platform_enum_map_.at(field);
  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    return def_iter->second;
  }

  // default value - if any
  def_iter = def.find("_DEFAULT");
  if (def_iter != def.end()) {
    return def_iter->second;
  }

  std::string message =
      "Platform enum value " + value + " not found for " + field;
  throw InvalidValuesException(message);
}

std::string CalendarItem::PlatformEnumToString(const std::string& field,
                                               int value) {
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
    throw UnknownException(std::string("Undefined platform enum type ") +
                           field);
  }

  auto def = platform_enum_reverse_map_.at(field);
  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    return def_iter->second;
  }

  std::string message = "Platform enum value " + std::to_string(value) +
                        " not found for " + field;
  throw InvalidValuesException(message);
}

void CalendarItem::SetString(int type, calendar_record_h rec,
                             const std::string& property,
                             const json::Object& in, bool optional) {
  LoggerD("set: %s", property.c_str());

  if (optional && IsNull(in, property.c_str())) {
    return;
  }

  const std::string& value =
      common::FromJson<json::String>(in, property.c_str());

  SetString(type, rec, property, value);
}

void CalendarItem::SetString(int type, calendar_record_h rec,
                             const std::string& property,
                             const std::string& value) {
  LoggerD("set: %s", property.c_str());

  unsigned int prop = GetPlatformProperty(type, property);

  if (prop != -1u) {
    CalendarRecord::SetString(rec, prop, value);
  }
}

std::string CalendarItem::GetString(int type, calendar_record_h rec,
                                    const std::string& property) {
  LoggerD("get: %s", property.c_str());

  return CalendarRecord::GetString(rec, GetPlatformProperty(type, property));
}

void CalendarItem::SetInt(int type, calendar_record_h rec,
                          const std::string& property, const json::Object& in,
                          bool optional) {
  LoggerD("set: %s", property.c_str());

  if (optional && IsNull(in, property.c_str())) {
    return;
  }

  int value = common::FromJson<double>(in, property.c_str());

  SetInt(type, rec, property, value);
}

void CalendarItem::SetInt(int type, calendar_record_h rec,
                          const std::string& property, int value) {
  LoggerD("set: %s", property.c_str());

  unsigned int prop = GetPlatformProperty(type, property);

  if (prop != -1u) {
    CalendarRecord::SetInt(rec, prop, value);
  }
}

int CalendarItem::GetInt(int type, calendar_record_h rec,
                         const std::string& property) {
  LoggerD("get: %s", property.c_str());

  return CalendarRecord::GetInt(rec, GetPlatformProperty(type, property));
}

void CalendarItem::SetEnum(int type, calendar_record_h rec,
                           const std::string& property, const json::Object& in,
                           const std::string& enum_name) {
  std::string value = common::FromJson<std::string>(in, property.c_str());
  SetInt(type, rec, property, StringToPlatformEnum(enum_name, value));
}

void CalendarItem::SetEnum(calendar_record_h rec, unsigned int property,
                           const std::string& enum_name,
                           const std::string& value) {
  CalendarRecord::SetInt(rec, property, StringToPlatformEnum(enum_name, value));
}

std::string CalendarItem::GetEnum(int type, calendar_record_h rec,
                                  const std::string& property,
                                  const std::string& enum_name) {
  return PlatformEnumToString(enum_name, GetInt(type, rec, property));
}

std::string CalendarItem::GetEnum(calendar_record_h rec, unsigned int property,
                                  const std::string& enum_name) {
  return PlatformEnumToString(enum_name, CalendarRecord::GetInt(rec, property));
}

void CalendarItem::SetDouble(int type, calendar_record_h rec,
                             const std::string& property, double value) {
  LoggerD("set: %s", property.c_str());

  unsigned int prop = GetPlatformProperty(type, property);

  if (prop != -1u) {
    int ret = calendar_record_set_double(rec, prop, value);

    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't set double value to record: %d", ret);
      throw common::UnknownException("Set double to record failed.");
    }
  }
}

double CalendarItem::GetDouble(int type, calendar_record_h rec,
                               const std::string& property) {
  LoggerD("get: %s", property.c_str());

  double value;
  int ret = calendar_record_get_double(rec, GetPlatformProperty(type, property),
                                       &value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get double value form record: %d", ret);
    throw common::UnknownException("Get int from record failed.");
  }

  return value;
}

void CalendarItem::SetCaltime(int type, calendar_record_h rec,
                              const std::string& property,
                              calendar_time_s value, bool throw_on_error) {
  LoggerD("enter");

  unsigned int prop = GetPlatformProperty(type, property);

  if (prop != -1u) {
    SetCaltime(rec, prop, value, throw_on_error);
  }
}

void CalendarItem::SetCaltime(calendar_record_h rec, unsigned int property,
                              calendar_time_s value, bool throw_on_error) {
  int ret = calendar_record_set_caltime(rec, property, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set caltime value to record: %d", ret);

    if (throw_on_error) {
      throw common::UnknownException("Set caltime to record failed.");
    }
  }
}

calendar_time_s CalendarItem::GetCaltime(int type, calendar_record_h rec,
                                         const std::string& property,
                                         bool throw_on_error) {
  LoggerD("get: %s", property.c_str());

  unsigned int prop = GetPlatformProperty(type, property);

  return GetCaltime(rec, prop, throw_on_error);
}

calendar_time_s CalendarItem::GetCaltime(calendar_record_h rec,
                                         unsigned int property,
                                         bool throw_on_error) {
  calendar_time_s cal;

  if (property != -1u) {
    int ret = calendar_record_get_caltime(rec, property, &cal);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerW("Can't get calendar_time value form record: %d", ret);
      if (throw_on_error) {
        throw common::UnknownException(
            "Can't get calendar_time value form record");
      }
    }
  }

  return cal;
}

void CalendarItem::SetLli(calendar_record_h rec, unsigned int property,
                          long long int value, bool throw_on_error) {

  int ret = calendar_record_set_lli(rec, property, value);

  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't set long long int value to record: %d", ret);

    if (throw_on_error) {
      throw common::UnknownException("Set long long int to record failed.");
    }
  }
}

long long int CalendarItem::GetLli(int type, calendar_record_h rec,
                                   const std::string& property) {
  LoggerD("get: %s", property.c_str());

  return GetLli(rec, GetPlatformProperty(type, property));
}

long long int CalendarItem::GetLli(calendar_record_h rec, unsigned int property,
                                   bool throw_on_error) {
  long long int value;
  int ret = calendar_record_get_lli(rec, property, &value);
  if (CALENDAR_ERROR_NONE != ret) {
    LoggerW("Can't get lli value form record: %d", ret);
    if (throw_on_error) {
      throw common::UnknownException("Get lli from record failed.");
    }
  }

  return value;
}

Date CalendarItem::DateFromJson(const json::Object& in) {
  LoggerD("json date %s", json::Value(in).serialize().c_str());

  Date date = {(long long int)common::FromJson<double>(in, "UTCTimestamp"),
               (int)common::FromJson<double>(in, "year"),
               (int)common::FromJson<double>(in, "month"),
               (int)common::FromJson<double>(in, "day"),
               common::FromJson<json::String>(in, "timezone")};

  return date;
}

Date CalendarItem::DateFromJson(const json::Object& in, const char* obj_name) {
  return DateFromJson(common::FromJson<json::Object>(in, obj_name));
}

json::Value CalendarItem::DateToJson(Date date) {
  LoggerD("timestamp: %lld", date.utc_timestamp_);

  json::Value date_val = json::Value(json::Object());
  json::Object& date_obj = date_val.get<json::Object>();

  date_obj["UTCTimestamp"] =
      json::Value(static_cast<double>(date.utc_timestamp_));
  date_obj["year"] = json::Value(static_cast<double>(date.year_));
  date_obj["month"] = json::Value(static_cast<double>(date.month_));
  date_obj["day"] = json::Value(static_cast<double>(date.day_));
  date_obj["timezone"] = json::Value(date.time_zone_);

  return date_val;
}

void CalendarItem::CategoriesFromJson(int type, calendar_record_h rec,
                                      const json::Array& value) {
  std::string categories = "";
  for (auto iter = value.begin(); iter != value.end(); ++iter) {
    if (iter == value.begin()) {
      categories.append(iter->get<json::String>().c_str());
    } else {
      categories.append("," + iter->get<json::String>());
    }
  }

  SetString(type, rec, "categories", categories);
}

json::Array CalendarItem::CategoriesToJson(int type, calendar_record_h rec) {
  LoggerD("enter");

  std::string categories = GetString(type, rec, "categories");

  return StringToArray(categories);
}

void CalendarItem::AttendeesFromJson(int type, calendar_record_h rec,
                                     const json::Array& value) {
  LoggerD("enter");

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
    const json::Object& obj = JsonCast<json::Object>(item);

    int ret = calendar_record_create(_calendar_attendee._uri, &attendee);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Fail to create attendee record, error code: %d", ret);
      throw common::UnknownException("Fail to create attendee record");
    }

    CalendarRecord::SetString(attendee, _calendar_attendee.email,
                              common::FromJson<json::String>(obj, "uri"));

    if (!IsNull(obj, "name")) {
      CalendarRecord::SetString(attendee, _calendar_attendee.name,
                                common::FromJson<json::String>(obj, "name"));
    }

    SetEnum(attendee, _calendar_attendee.role, kAttendeeRole,
            common::FromJson<json::String>(obj, "role"));

    SetEnum(attendee, _calendar_attendee.status, kAttendeeStatus,
            common::FromJson<json::String>(obj, "status"));

    CalendarRecord::SetInt(attendee, _calendar_attendee.rsvp,
                           common::FromJson<bool>(obj, "RSVP"));

    SetEnum(attendee, _calendar_attendee.cutype, kAttendeeType,
            common::FromJson<json::String>(obj, "type"));

    if (!IsNull(obj, "group")) {
      CalendarRecord::SetString(attendee, _calendar_attendee.group,
                                common::FromJson<json::String>(obj, "group"));
    }
    if (!IsNull(obj, "delegatorURI")) {
      CalendarRecord::SetString(
          attendee, _calendar_attendee.delegator_uri,
          common::FromJson<json::String>(obj, "delegatorURI"));
    }
    if (!IsNull(obj, "delegateURI")) {
      CalendarRecord::SetString(
          attendee, _calendar_attendee.delegatee_uri,
          common::FromJson<json::String>(obj, "delegateURI"));
    }

    if (!IsNull(obj, "contactRef")) {
      CalendarRecord::SetString(
          attendee, _calendar_attendee.uid,
          common::FromJson<json::String>(obj, "contactRef", "contactId"));

      const std::string& address_book =
          common::FromJson<json::String>(obj, "contactRef", "addressBookId");
      CalendarRecord::SetInt(attendee, _calendar_attendee.person_id,
                             common::stol(address_book));
    } else {
      LoggerD("ContactRef not set");
    }

    AddChildRecord(rec, property, attendee);
  }
}

json::Array CalendarItem::AttendeesToJson(int type, calendar_record_h rec) {
  LoggerD("enter");

  json::Array out = json::Array();

  unsigned int property;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    property = _calendar_event.calendar_attendee;
  } else {
    property = _calendar_todo.calendar_attendee;
  }

  unsigned int count = 0;
  if (!(count = GetChildRecordCount(rec, property))) {
    LoggerD("No attendees to set.");
    return out;
  }

  calendar_list_h list;
  if (CALENDAR_ERROR_NONE !=
      calendar_record_clone_child_record_list(rec, property, &list)) {
    LoggerE("Can't get attendee list");
    return out;
  }
  CalendarListPtr(list, CalendarRecord::ListDeleter);

  calendar_record_h attendee;
  for (unsigned int i = 0; i < count; ++i) {
    LoggerD("Processing the attendee %d", i);

    if (!GetChildRecordAt(rec, property, &attendee, i, false)) {
      LoggerW("Can't get attendee record");
      continue;
    }

    json::Value attendee_val = json::Value(json::Object());
    json::Object& attendee_obj = attendee_val.get<json::Object>();

    attendee_obj["uri"] = json::Value(
        CalendarRecord::GetString(attendee, _calendar_attendee.email, false));

    attendee_obj["name"] = json::Value(
        CalendarRecord::GetString(attendee, _calendar_attendee.name, false));

    attendee_obj["role"] =
        json::Value(GetEnum(attendee, _calendar_attendee.role, kAttendeeRole));

    attendee_obj["status"] = json::Value(
        GetEnum(attendee, _calendar_attendee.status, kAttendeeStatus));

    attendee_obj["RSVP"] = json::Value(
        (bool)CalendarRecord::GetInt(attendee, _calendar_attendee.rsvp, false));

    attendee_obj["type"] = json::Value(
        GetEnum(attendee, _calendar_attendee.cutype, kAttendeeType));

    attendee_obj["group"] = json::Value(
        CalendarRecord::GetString(attendee, _calendar_attendee.group, false));

    attendee_obj["delegatorURI"] = json::Value(CalendarRecord::GetString(
        attendee, _calendar_attendee.delegator_uri, false));

    attendee_obj["delegateURI"] = json::Value(CalendarRecord::GetString(
        attendee, _calendar_attendee.delegatee_uri, false));

    // contactRef
    const std::string& contact_id =
        CalendarRecord::GetString(attendee, _calendar_attendee.uid, false);
    int book_id =
        CalendarRecord::GetInt(attendee, _calendar_attendee.person_id, false);
    attendee_obj["contactRef"] = json::Value(
        json::Object{{"contactId", json::Value(contact_id)},
                     {"addressBookId", json::Value(std::to_string(book_id))}});

    out.push_back(attendee_val);
  }

  return out;
}

void CalendarItem::AlarmsFromJson(int type, calendar_record_h rec,
                                  const common::json::Array& alarms) {
  LoggerD("enter");

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
    const json::Object& obj = JsonCast<json::Object>(item);

    int ret = calendar_record_create(_calendar_alarm._uri, &alarm);
    if (CALENDAR_ERROR_NONE != ret) {
      LoggerE("Fail to create attendee record, error code: %d", ret);
      throw common::UnknownException("Fail to create attendee record");
    }

    int tick_unit = CALENDAR_ALARM_TIME_UNIT_SPECIFIC;
    if (!common::IsNull(obj, "absoluteDate")) {
      Date absolute = DateFromJson(obj, "absoluteDate");
      calendar_time_s absolute_date = DateToPlatform(absolute, false);
      SetLli(alarm, _calendar_alarm.time, absolute_date.time.utime);
      CalendarRecord::SetInt(alarm, _calendar_alarm.tick_unit, tick_unit);
    }

    if (!common::IsNull(obj, "before")) {
      long long length = common::FromJson<double>(obj, "before", "length");
      const std::string& unit =
          common::FromJson<json::String>(obj, "before", "unit");
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

      CalendarRecord::SetInt(alarm, _calendar_alarm.tick, tick);
      CalendarRecord::SetInt(alarm, _calendar_alarm.tick_unit, tick_unit);
    }

    SetEnum(alarm, _calendar_alarm.action, kAlarmMethod,
            common::FromJson<json::String>(obj, "method"));

    CalendarRecord::SetString(
        alarm, _calendar_alarm.description,
        common::FromJson<json::String>(obj, "description"));

    AddChildRecord(rec, property, alarm);
  }
}

json::Array CalendarItem::AlarmsToJson(int type, calendar_record_h rec) {
  LoggerD("enter");

  json::Array out = json::Array();

  unsigned int property;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    property = _calendar_event.calendar_alarm;
  } else {
    property = _calendar_todo.calendar_alarm;
  }

  unsigned int count = 0;
  if (!(count = GetChildRecordCount(rec, property))) {
    LoggerD("No attendees to set.");
    return out;
  }

  calendar_list_h list;
  if (CALENDAR_ERROR_NONE !=
      calendar_record_clone_child_record_list(rec, property, &list)) {
    LoggerW("Can't get alarms list");
    return out;
  }
  CalendarListPtr(list, CalendarRecord::ListDeleter);

  int tick, tick_unit;
  calendar_record_h alarm;
  for (unsigned int i = 0; i < count; ++i) {
    LoggerD("Processing the alarm %d", i);

    if (!GetChildRecordAt(rec, property, &alarm, i, false)) {
      LoggerW("Can't get alarm record");
      continue;
    }

    json::Value alarm_val = json::Value(json::Object());
    json::Object& alarm_obj = alarm_val.get<json::Object>();

    tick_unit = CalendarRecord::GetInt(alarm, _calendar_alarm.tick_unit, false);

    if (tick_unit == CALENDAR_ALARM_TIME_UNIT_SPECIFIC) {
      long long int time = GetLli(alarm, _calendar_alarm.time, false);
      alarm_obj["absoluteDate"] = json::Value(static_cast<double>(time));
    } else {
      tick = CalendarRecord::GetInt(alarm, _calendar_alarm.tick, false);

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

      alarm_obj["before"] = json::Value(
          json::Object{{"length", json::Value(static_cast<double>(length))},
                       {"unit", json::Value(unit)}});
    }

    alarm_obj["method"] =
        json::Value(GetEnum(alarm, _calendar_alarm.action, kAlarmMethod));

    alarm_obj["description"] = json::Value(
        CalendarRecord::GetString(alarm, _calendar_alarm.description, false));

    out.push_back(alarm_val);
  }

  return out;
}

void CalendarItem::RecurrenceRuleFromJson(calendar_record_h rec,
                                          const json::Object& rrule) {
  LoggerD("enter");

  const std::string& frequency =
      common::FromJson<json::String>(rrule, "frequency");
  SetEnum(rec, _calendar_event.freq, kRecurrenceRuleFrequency, frequency);

  const unsigned short interval = common::FromJson<double>(rrule, "interval");
  CalendarRecord::SetInt(rec, _calendar_event.interval, interval);

  const long occurrence_count =
      common::FromJson<double>(rrule, "occurrenceCount");
  if (-1 != occurrence_count) {
    CalendarRecord::SetInt(rec, _calendar_event.count, occurrence_count);
    CalendarRecord::SetInt(rec, _calendar_event.range_type,
                           CALENDAR_RANGE_COUNT);
  }

  if (!common::IsNull(rrule, "untilDate")) {
    Date until = DateFromJson(rrule, "untilDate");
    SetCaltime(rec, _calendar_event.until_time, DateToPlatform(until, false));
    CalendarRecord::SetInt(rec, _calendar_event.range_type,
                           CALENDAR_RANGE_UNTIL);
  }

  const json::Array& byday_array =
      common::FromJson<json::Array>(rrule, "daysOfTheWeek");
  std::string byday;
  for (auto iter = byday_array.begin(); iter != byday_array.end(); ++iter) {
    if (iter == byday_array.begin()) {
      byday.append(iter->get<json::String>());
    } else {
      byday.append("," + iter->get<json::String>());
    }
  }
  CalendarRecord::SetString(rec, _calendar_event.byday, byday);

  const json::Array& bysetpos_array =
      common::FromJson<json::Array>(rrule, "setPositions");
  std::string bysetpos;
  for (auto iter = bysetpos_array.begin(); iter != bysetpos_array.end();
       ++iter) {
    if (iter == bysetpos_array.begin()) {
      bysetpos.append(std::to_string((int)iter->get<double>()));
    } else {
      bysetpos.append("," + iter->get<json::String>());
    }
  }
  CalendarRecord::SetString(rec, _calendar_event.bysetpos, bysetpos);

  CalendarRecord::SetString(
      rec, _calendar_event.exdate,
      ExceptionsFromJson(common::FromJson<json::Array>(rrule, "exceptions")));
}

std::string CalendarItem::ExceptionsFromJson(const json::Array& exceptions) {
  std::string result;
  Date date;
  for (auto iter = exceptions.begin(); iter != exceptions.end(); ++iter) {
    date = DateFromJson(iter->get<json::Object>());
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

json::Object CalendarItem::RecurrenceRuleToJson(calendar_record_h rec) {
  LoggerD("enter");

  json::Object out = json::Object();

  out["frequency"] =
      json::Value(GetEnum(rec, _calendar_event.freq, kRecurrenceRuleFrequency));

  int interval = CalendarRecord::GetInt(rec, _calendar_event.interval, false);
  out["interval"] = json::Value(static_cast<double>(interval));

  int occurrence_count = CalendarRecord::GetInt(rec, _calendar_event.count);
  out["occurrenceCount"] = json::Value(static_cast<double>(occurrence_count));

  calendar_time_s cal = {CALENDAR_TIME_UTIME, {0}};
  calendar_record_get_caltime(rec, _calendar_event.until_time, &cal);
  if (cal.time.utime > 0 && CALENDAR_RECORD_NO_UNTIL != cal.time.utime) {
    Date until = {cal.time.utime, 0, 0, 0, ""};
    out["untilDate"] = DateToJson(until);
  } else {
    out["untilDate"] = json::Value();
  }

  out["daysOfTheWeek"] = json::Value(
      StringToArray(CalendarRecord::GetString(rec, _calendar_event.byday)));

  out["setPositions"] = json::Value(
      StringToArray(CalendarRecord::GetString(rec, _calendar_event.bysetpos)));

  const json::Array& exceptions =
      StringToArray(CalendarRecord::GetString(rec, _calendar_event.exdate));
  json::Array dates = json::Array();
  for (auto& exception : exceptions) {
    Date date = {common::stol(exception.get<std::string>()), 0, 0, 0, ""};
    dates.push_back(DateToJson(date));
  }
  out["exceptions"] = json::Value(dates);

  return out;
}

calendar_time_s CalendarItem::DateToPlatform(const Date& date,
                                             bool is_all_day) {
  LoggerD("enter");

  calendar_time_s cal;

  if (is_all_day) {
    cal.type = CALENDAR_TIME_LOCALTIME;
    cal.time.date = {date.year_, date.month_, date.day_};
  } else {
    cal.type = CALENDAR_TIME_UTIME;
    cal.time.utime = date.utc_timestamp_;
  }

  return cal;
}

Date CalendarItem::DateFromPlatform(int type, calendar_record_h rec,
                                    const std::string& property) {
  LoggerD("enter");

  calendar_time_s cal = GetCaltime(type, rec, property + "_time");
  std::string tzid = GetString(type, rec, property + "_tzid");

  Date date = {cal.time.utime,     cal.time.date.year, cal.time.date.month,
               cal.time.date.mday, tzid};

  return date;
}

Date CalendarItem::DateFromPlatform(calendar_record_h rec,
                                    unsigned int property) {
  LoggerD("enter");

  calendar_time_s cal = GetCaltime(rec, property);

  Date date = {cal.time.utime,     cal.time.date.year, cal.time.date.month,
               cal.time.date.mday, ""};

  return date;
}

void CalendarItem::FromJson(int type, calendar_record_h rec,
                            const json::Object& in) {
  LoggerD("enter");

  if (in.empty()) {
    LoggerE("Empty CalendarItem object.");
    throw InvalidValuesException("Empty Calendar object.");
  }

  SetString(type, rec, "description", in, true);
  SetString(type, rec, "summary", in, true);
  SetString(type, rec, "location", in, true);
  SetString(type, rec, "organizer", in, true);

  int is_all_day = common::FromJson<bool>(in, "isAllDay");

  if (!common::IsNull(in, "startDate")) {
    Date start = DateFromJson(in, "startDate");

    SetCaltime(type, rec, "startDate_time", DateToPlatform(start, is_all_day));
    SetString(type, rec, "startDate_tzid", start.time_zone_);
  }

  const std::string& endProperty =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? "endDate" : "dueDate";
  if (!common::IsNull(in, endProperty.c_str())) {
    Date end = DateFromJson(in, endProperty.c_str());

    SetCaltime(type, rec, endProperty + "_time",
               DateToPlatform(end, is_all_day));
    SetString(type, rec, endProperty + "_tzid", end.time_zone_);
  }

  SetEnum(type, rec, "visibility", in, kItemVisibility);

  if (!common::IsNull(in, "geolocation")) {
    SetDouble(type, rec, "latitude",
              common::FromJson<double>(in, "geolocation", "latitude"));
    SetDouble(type, rec, "longitude",
              common::FromJson<double>(in, "geolocation", "longitude"));
  }

  CategoriesFromJson(type, rec,
                     common::FromJson<json::Array>(in, "categories"));
  AttendeesFromJson(type, rec, common::FromJson<json::Array>(in, "attendees"));
  AlarmsFromJson(type, rec, common::FromJson<json::Array>(in, "alarms"));

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    SetEnum(type, rec, "priority", in, kEventPriority);
    SetEnum(type, rec, "status", in, kEventStatus);
    SetEnum(type, rec, "availability", in, kEventAvailability);

    if (!common::IsNull(in, "recurrenceRule")) {
      RecurrenceRuleFromJson(
          rec, common::FromJson<json::Object>(in, "recurrenceRule"));
    }

  } else {
    SetEnum(type, rec, "priority", in, kTaskPriority);
    SetEnum(type, rec, "status", in, kTaskStatus);

    if (!common::IsNull(in, "completedDate")) {
      SetLli(rec, _calendar_todo.completed_time,
             DateFromJson(in, "completedDate").utc_timestamp_);
    }
    SetInt(type, rec, "progress", in);
  }
}

void CalendarItem::ToJson(int type, calendar_record_h rec,
                          json::Object* out_ptr) {
  LoggerD("enter");

  if (NULL == rec) {
    LoggerE("Calendar record is null");
    throw UnknownException("Calendar record is null");
  }

  json::Object& out = *out_ptr;

  int id = GetInt(type, rec, "id");

  json::Value id_val;
  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    id_val = json::Value(json::Object());
    json::Object& id_obj = id_val.get<json::Object>();

    id_obj["uid"] = json::Value(std::to_string(id));
    const std::string& rid = GetString(type, rec, "recurrence_id");
    if (rid.length() > 0) {
      id_obj["rid"] = json::Value(rid);
    } else {
      id_obj["rid"] = json::Value();
    }
  } else {
    id_val = json::Value(std::to_string(id));
  }

  out["id"] = id_val;

  int calendar_id = GetInt(type, rec, "calendar_id");
  out["calendarId"] = json::Value(std::to_string(calendar_id));

  out["description"] = json::Value(GetString(type, rec, "description"));
  out["summary"] = json::Value(GetString(type, rec, "summary"));
  out["location"] = json::Value(GetString(type, rec, "location"));
  out["organizer"] = json::Value(GetString(type, rec, "organizer"));
  out["isAllDay"] = json::Value((bool)GetInt(type, rec, "isAllDay"));

  // startDate
  out["startDate"] = DateToJson(DateFromPlatform(type, rec, "startDate"));

  // endDate / dueDate
  const std::string& endProperty =
      (type == CALENDAR_BOOK_TYPE_EVENT) ? "endDate" : "dueDate";
  out[endProperty] = DateToJson(DateFromPlatform(type, rec, endProperty));

  out["lastModificationDate"] = json::Value(
      static_cast<double>(GetLli(type, rec, "lastModificationDate")));

  out["geolocation"] = json::Value(json::Object(
      {{"latitude", json::Value(GetDouble(type, rec, "latitude"))},
       {"longitude", json::Value(GetDouble(type, rec, "longitude"))}}));

  out["visibility"] =
      json::Value(GetEnum(type, rec, "visibility", kItemVisibility));

  out["attendees"] = json::Value(AttendeesToJson(type, rec));
  out["categories"] = json::Value(CategoriesToJson(type, rec));
  out["alarms"] = json::Value(AlarmsToJson(type, rec));

  if (type == CALENDAR_BOOK_TYPE_EVENT) {
    out["status"] = json::Value(GetEnum(type, rec, "status", kEventStatus));
    out["priority"] =
        json::Value(GetEnum(type, rec, "priority", kEventPriority));
    out["availability"] =
        json::Value(GetEnum(type, rec, "availability", kEventAvailability));
    out["recurrenceRule"] = json::Value(RecurrenceRuleToJson(rec));
  } else {
    out["status"] = json::Value(GetEnum(type, rec, "status", kTaskStatus));
    out["priority"] =
        json::Value(GetEnum(type, rec, "priority", kTaskPriority));

    out["completedDate"] = json::Value(
        static_cast<double>(GetLli(rec, _calendar_todo.completed_time)));
    out["progress"] =
        json::Value(static_cast<double>(GetInt(type, rec, "progress")));
  }
}

json::Array CalendarItem::StringToArray(const std::string& string) {
  json::Array out = json::Array();

  char* cstr = new char[string.length() + 1];
  strcpy(cstr, string.c_str());

  char* saveptr = NULL;
  char* pch = strtok_r(cstr, ",", &saveptr);

  while (NULL != pch) {
    out.push_back(json::Value(std::string(pch)));
    pch = strtok_r(NULL, ",", &saveptr);
  }

  delete[] cstr;

  return out;
}

}  // namespace calendar
}  // namespace webapi
