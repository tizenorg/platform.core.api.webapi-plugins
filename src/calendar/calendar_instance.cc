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

CalendarInstance& CalendarInstance::GetInstance() {
  static CalendarInstance instance;
  return instance;
}

CalendarInstance::CalendarInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&CalendarInstance::x, this, _1, _2));

  // Calendar
  REGISTER_SYNC("Calendar_get", CalendarGet);
  REGISTER_SYNC("Calendar_add", CalendarAdd);
  REGISTER_SYNC("Calendar_update", CalendarUpdate);
  REGISTER_SYNC("Calendar_remove", CalendarRemove);
  REGISTER_SYNC("Calendar_addChangeListener", CalendarAddChangeListener);
  REGISTER_SYNC("Calendar_removeChangeListener", CalendarRemoveChangeListener);

  // Calendar Manager
  REGISTER_SYNC("CalendarManager_addCalendar", CalendarManagerAddCalendar);
  REGISTER_SYNC("CalendarManager_getCalendar", CalendarManagerGetCalendar);
  REGISTER_SYNC("CalendarManager_removeCalendar",
                CalendarManagerRemoveCalendar);
#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&CalendarInstance::x, this, _1, _2));
  REGISTER_ASYNC("Calendar_addBatch", CalendarAddBatch);
  REGISTER_ASYNC("Calendar_updateBatch", CalendarUpdateBatch);
  REGISTER_ASYNC("Calendar_removeBatch", CalendarRemoveBatch);
  REGISTER_ASYNC("Calendar_updateBatch", CalendarUpdateBatch);
  REGISTER_ASYNC("CalendarManager_getCalendars", CalendarManagerGetCalendars);
  REGISTER_ASYNC("Calendar_find", CalendarFind);
#undef REGISTER_ASYNC
}

CalendarInstance::~CalendarInstance() {}

void CalendarInstance::CalendarGet(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Get(common::JsonCast<JsonObject>(args),
                              val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarAdd(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Add(common::JsonCast<JsonObject>(args),
                              val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarAddBatch(const JsonValue& args,
                                        JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue> & response)->void {
    try {
      JsonValue result = JsonValue(JsonArray());
      Calendar::GetInstance().AddBatch(common::JsonCast<JsonObject>(args),
                                       result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    }
    catch (const PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response = [ callback_id, this ](const std::shared_ptr<JsonValue> &
                                            response)->void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", callback_id));
    LoggerD("callback is %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarUpdate(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Update(common::JsonCast<JsonObject>(args),
                                 val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarUpdateBatch(const JsonValue& args,
                                           JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue> & response)->void {
    try {
      JsonValue result = JsonValue(JsonArray());
      Calendar::GetInstance().UpdateBatch(common::JsonCast<JsonObject>(args),
                                          result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    }
    catch (const PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response = [ callback_id, this ](const std::shared_ptr<JsonValue> &
                                            response)->void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", callback_id));
    LoggerD("callback is %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarRemove(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().Remove(common::JsonCast<JsonObject>(args),
                                 val.get<JsonObject>());
  ReportSuccess(out);
}

void CalendarInstance::CalendarRemoveBatch(const JsonValue& args,
                                           JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue> & response)->void {
    try {
      JsonValue result = JsonValue(JsonArray());
      Calendar::GetInstance().RemoveBatch(common::JsonCast<JsonObject>(args),
                                          result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    }
    catch (const PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response = [ callback_id, this ](const std::shared_ptr<JsonValue> &
                                            response)->void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", callback_id));
    LoggerD("callback is %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarFind(const JsonValue& args, JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue> & response)->void {
    try {
      JsonValue result = JsonValue(JsonArray());
      Calendar::GetInstance().Find(common::JsonCast<JsonObject>(args),
                                   result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    }
    catch (const PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response = [ callback_id, this ](const std::shared_ptr<JsonValue> &
                                            response)->void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", callback_id));
    LoggerD("callback isssssss %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarAddChangeListener(const JsonValue& args,
                                                 JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().AddChangeListener(common::JsonCast<JsonObject>(args),
                                            val.get<JsonObject>());
  ReportSuccess(out);
}

void CalendarInstance::CalendarRemoveChangeListener(const JsonValue& args,
                                                    JsonObject& out) {
  JsonValue val{JsonObject{}};
  Calendar::GetInstance().RemoveChangeListener(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  ReportSuccess(out);
}

// CalendarManager
void CalendarInstance::CalendarManagerAddCalendar(const JsonValue& args,
                                                  JsonObject& out) {
  JsonValue val{JsonObject{}};
  CalendarManager::GetInstance().AddCalendar(common::JsonCast<JsonObject>(args),
                                             val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarManagerGetCalendar(const JsonValue& args,
                                                  JsonObject& out) {
  JsonValue val{JsonObject{}};
  CalendarManager::GetInstance().GetCalendar(common::JsonCast<JsonObject>(args),
                                             val.get<JsonObject>());
  ReportSuccess(val, out);
}

void CalendarInstance::CalendarManagerGetCalendars(const JsonValue& args,
                                                   JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue> & response)->void {
    try {
      JsonValue result = JsonValue(JsonArray());
      CalendarManager::GetInstance().GetCalendars(
          common::JsonCast<JsonObject>(args), result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    }
    catch (const PlatformException& e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response = [ callback_id, this ](const std::shared_ptr<JsonValue> &
                                            response)->void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", callback_id));
    LoggerD("callback is %s", response->serialize().c_str());
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
