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

CalendarInstance::CalendarInstance()
    : calendar_(*this) {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

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
  RegisterSyncHandler(c, std::bind(&CalendarInstance::x, this, _1, _2));
  REGISTER_ASYNC("Calendar_addBatch", CalendarAddBatch);
  REGISTER_ASYNC("Calendar_updateBatch", CalendarUpdateBatch);
  REGISTER_ASYNC("Calendar_removeBatch", CalendarRemoveBatch);
  REGISTER_ASYNC("Calendar_updateBatch", CalendarUpdateBatch);
  REGISTER_ASYNC("CalendarManager_getCalendars", CalendarManagerGetCalendars);
  REGISTER_ASYNC("Calendar_find", CalendarFind);
#undef REGISTER_ASYNC
}

CalendarInstance::~CalendarInstance() {
  LoggerD("Enter");
}

void CalendarInstance::CalendarGet(const JsonValue& args, JsonObject& out) {
  LoggerD("Enter");

  JsonValue val{JsonObject{}};

  PlatformResult status = calendar_.Get(common::JsonCast<JsonObject>(args),
                                        val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void CalendarInstance::CalendarAdd(const JsonValue& args, JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};

  PlatformResult status = calendar_.Add(common::JsonCast<JsonObject>(args),
                                        val.get<JsonObject>());

  if (status.IsSuccess()) {
    ReportSuccess(val, out);
  } else{
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void CalendarInstance::CalendarAddBatch(const JsonValue& args,
                                        JsonObject& out) {
  LoggerD("Enter");

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarAddBatch->get");
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = calendar_.AddBatch(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<picojson::object>());
    else
    {
      LoggerE("Failed");
      ReportError(status, &response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarAddBatch->get_response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback is %s", response->serialize().c_str());
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarUpdate(const JsonValue& args, JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};

  PlatformResult status = calendar_.Update(common::JsonCast<JsonObject>(args),
                                           val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void CalendarInstance::CalendarUpdateBatch(const JsonValue& args,
                                           JsonObject& out) {
  LoggerD("Enter");

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarUpdateBatch->get");
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = calendar_.UpdateBatch(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<picojson::object>());
    else
    {
      LoggerE("Failed");
      ReportError(status, &response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarUpdateBatch->get_response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback is %s", response->serialize().c_str());
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarRemove(const JsonValue& args, JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};

  PlatformResult status = calendar_.Remove(common::JsonCast<JsonObject>(args),
                                           val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &val.get<JsonObject>());
  }
}

void CalendarInstance::CalendarRemoveBatch(const JsonValue& args,
                                           JsonObject& out) {
  LoggerD("Enter");

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarRemoveBatch->get");
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = calendar_.RemoveBatch(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<picojson::object>());
    else
    {
      LoggerE("Failed");
      ReportError(status, &response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarRemoveBatch->get_response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback is %s", response->serialize().c_str());
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarFind(const JsonValue& args, JsonObject& out) {
  LoggerD("Enter");

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarFind->get");
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = calendar_.Find(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<picojson::object>());
    else
    {
      LoggerE("Failed");
      ReportError(status, &response->get<picojson::object>());
    }
  };

  auto get_response =
      [callback_id, this](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarFind->get_response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback isssssss %s", response->serialize().c_str());
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarAddChangeListener(const JsonValue& args,
                                                 JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};

  PlatformResult status = calendar_.AddChangeListener(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &val.get<JsonObject>());
  }
}

void CalendarInstance::CalendarRemoveChangeListener(const JsonValue& args,
                                                    JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};

  PlatformResult status = calendar_.RemoveChangeListener(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &val.get<JsonObject>());
  }
}

// CalendarManager
void CalendarInstance::CalendarManagerAddCalendar(const JsonValue& args,
                                                  JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};
  PlatformResult status = CalendarManager::GetInstance().AddCalendar(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void CalendarInstance::CalendarManagerGetCalendar(const JsonValue& args,
                                                  JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};
  PlatformResult status = CalendarManager::GetInstance().GetCalendar(common::JsonCast<JsonObject>(args),
                                             val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void CalendarInstance::CalendarManagerGetCalendars(const JsonValue& args,
                                                   JsonObject& out) {
  LoggerD("Enter");

  const double callback_id = args.get("callbackId").get<double>();
  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    LoggerD("CalendarManagerGetCalendars->get");
    JsonValue result = JsonValue(JsonArray());

    PlatformResult status = CalendarManager::GetInstance().GetCalendars(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<picojson::object>());
    else
    {
      LoggerE("Failed");
      ReportError(status, &response->get<JsonObject>());
    }
  };

  auto get_response = [ callback_id, this ](const std::shared_ptr<JsonValue> &
                                            response)->void {
    LoggerD("CalendarManagerGetCalendars->get_response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    LoggerD("callback is %s", response->serialize().c_str());
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void CalendarInstance::CalendarManagerRemoveCalendar(const JsonValue& args,
                                                     JsonObject& out) {
  LoggerD("Enter");
  JsonValue val{JsonObject{}};
  PlatformResult status = CalendarManager::GetInstance().RemoveCalendar(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());

  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
  {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

}  // namespace calendar
}  // namespace extension
