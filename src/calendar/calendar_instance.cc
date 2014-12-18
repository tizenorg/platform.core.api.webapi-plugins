// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "calendar/calendar_instance.h"

#include "common/converter.h"

#include "calendar/calendar_manager.h"
#include "calendar/calendar.h"
#include "common/task-queue.h"

#include <memory>
#include <map>
#include <calendar-service2/calendar.h>
#include "calendar_record.h"

namespace extension {
namespace calendar {

using namespace common;
using namespace extension::calendar;

CalendarInstance::CalendarInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&CalendarInstance::x, this, _1, _2));

  // Calendar
  REGISTER_SYNC("Calendar_get", Calendar_get);
  REGISTER_SYNC("Calendar_add", Calendar_add);
  REGISTER_SYNC("Calendar_addBatch", Calendar_addBatch);
  REGISTER_SYNC("Calendar_update", Calendar_update);
  REGISTER_SYNC("Calendar_updateBatch", Calendar_updateBatch);
  REGISTER_SYNC("Calendar_remove", Calendar_remove);
  REGISTER_SYNC("Calendar_removeBatch", Calendar_removeBatch);
  REGISTER_SYNC("Calendar_find", Calendar_find);
  REGISTER_SYNC("Calendar_addChangeListener", Calendar_addChangeListener);
  REGISTER_SYNC("Calendar_removeChangeListener", Calendar_removeChangeListener);

  // Calendar Manager
  REGISTER_SYNC("CalendarManager_addCalendar", CalendarManager_addCalendar);
  REGISTER_SYNC("CalendarManager_getCalendar", CalendarManager_getCalendar);
  //  REGISTER_SYNC("CalendarManager_getCalendars",
  //  CalendarManager_getCalendars);
  REGISTER_SYNC("CalendarManager_removeCalendar",
                CalendarManager_removeCalendar);

#undef REGISTER_SYNC
#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&CalendarInstance::x, this, _1, _2));
  REGISTER_ASYNC("CalendarManager_getCalendars", CalendarManager_getCalendars);
#undef REGISTER_ASYNC
}

CalendarInstance::~CalendarInstance() {}

void CalendarInstance::Calendar_get(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Get(common::JsonCast<JsonObject>(args),
                              val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::Calendar_add(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Add(common::JsonCast<JsonObject>(args),
                              val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::Calendar_addBatch(const JsonValue& args,
                                         JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().AddBatch(common::JsonCast<JsonObject>(args),
                                   val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::Calendar_update(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Update(common::JsonCast<JsonObject>(args),
                                 val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::Calendar_updateBatch(const JsonValue& args,
                                            JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().UpdateBatch(common::JsonCast<JsonObject>(args),
                                      val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::Calendar_remove(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Remove(common::JsonCast<JsonObject>(args),
                                 val.get<JsonObject>());
  ReportSuccess(out);
}

void CalendarInstance::Calendar_removeBatch(const JsonValue& args,
                                            JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().RemoveBatch(common::JsonCast<JsonObject>(args),
                                      val.get<JsonObject>());
  ReportSuccess(out);
}

void CalendarInstance::Calendar_find(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Find(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(out);
}

void CalendarInstance::Calendar_addChangeListener(const JsonValue& args,
                                                  JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().AddChangeListener(common::JsonCast<JsonObject>(args),
                                            val.get<JsonObject>());
  ReportSuccess(out);
}

void CalendarInstance::Calendar_removeChangeListener(const JsonValue& args,
                                                     JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().RemoveChangeListener(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  ReportSuccess(out);
}

// CalendarManager
void CalendarInstance::CalendarManager_addCalendar(const JsonValue& args,
                                                   JsonObject& out) {
  JsonValue val{JsonObject{}};
  CalendarManager::GetInstance().AddCalendar(common::JsonCast<JsonObject>(args),
                                             val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarManager_getCalendar(const JsonValue& args,
                                                   JsonObject& out) {
  JsonValue val{JsonObject{}};
  CalendarManager::GetInstance().GetCalendar(common::JsonCast<JsonObject>(args),
                                             val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarManager_getCalendars(const JsonValue& args,
                                                    JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    try {
      JsonValue result = JsonValue(JsonArray());
      CalendarManager::GetInstance().GetCalendars(
          common::JsonCast<JsonObject>(args), result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    } catch (const PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        LoggerD("callback isssssss %s", response->serialize().c_str());
        PostMessage(response->serialize().c_str());
      };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarManager_removeCalendar(const JsonValue& args,
                                                      JsonObject& out) {
  JsonValue val{JsonObject{}};
  CalendarManager::GetInstance().RemoveCalendar(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  ReportSuccess(val, out);
}

}  // namespace calendar
}  // namespace extension
