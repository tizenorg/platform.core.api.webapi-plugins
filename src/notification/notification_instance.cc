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

#include "notification/notification_instance.h"

#include <functional>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

#include "notification/notification_manager.h"

namespace extension {
namespace notification {

using namespace common;

NotificationInstance::NotificationInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&NotificationInstance::x, this, _1, _2));
  REGISTER_SYNC("NotificationManager_get", NotificationManagerGet);
  REGISTER_SYNC("NotificationManager_update", NotificationManagerUpdate);
  REGISTER_SYNC("NotificationManager_remove", NotificationManagerRemove);
  REGISTER_SYNC("NotificationManager_getAll", NotificationManagerGetAll);
  REGISTER_SYNC("NotificationManager_post", NotificationManagerPost);
  REGISTER_SYNC("NotificationManager_removeAll", NotificationManagerRemoveAll);
  REGISTER_SYNC("NotificationManager_playLEDCustomEffect",
      NotificationManagerPlayLEDCustomEffect);
  REGISTER_SYNC("NotificationManager_stopLEDCustomEffect",
      NotificationManagerStopLEDCustomEffect);
#undef REGISTER_SYNC

  manager_ = NotificationManager::GetInstance();
}

NotificationInstance::~NotificationInstance() {
  LoggerD("Enter");
}

#define CHECK_EXIST(args, name, out)                                       \
  if (!args.contains(name)) {                                              \
    ReportError(TypeMismatchException(name " is required argument"), out); \
    return;                                                                \
  }

void NotificationInstance::NotificationManagerPost(const picojson::value& args,
                                                   picojson::object& out) {

  LoggerD("Enter");
  picojson::value val{picojson::object{}};
  PlatformResult status =
      manager_->Post(args.get<picojson::object>(), val.get<picojson::object>());

  if (status.IsSuccess()) {
    ReportSuccess(val, out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerUpdate(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  PlatformResult status = manager_->Update(args.get<picojson::object>());

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerRemove(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  PlatformResult status = manager_->Remove(args.get<picojson::object>());

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerRemoveAll(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  PlatformResult status = manager_->RemoveAll();

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerGet(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Enter");
  picojson::value val{picojson::object{}};

  PlatformResult status =
      manager_->Get(args.get<picojson::object>(), val.get<picojson::object>());

  if (status.IsSuccess()) {
    ReportSuccess(val, out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerGetAll(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("Enter");
  picojson::value val{picojson::array{}};

  PlatformResult status = manager_->GetAll(val.get<picojson::array>());

  if (status.IsSuccess()) {
    ReportSuccess(val, out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerPlayLEDCustomEffect(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Enter");

  PlatformResult status = manager_->PlayLEDCustomEffect(args.get<picojson::object>());

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void NotificationInstance::NotificationManagerStopLEDCustomEffect(
    const picojson::value& /*args*/, picojson::object& out) {

  LoggerD("Enter");

  PlatformResult status = manager_->StopLEDCustomEffect();

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

#undef CHECK_EXIST

}  // namespace notification
}  // namespace extension
