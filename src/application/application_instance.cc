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

#include "application/application_instance.h"

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"
#include "common/current_application.h"

namespace extension {
namespace application {

using namespace common;

ApplicationInstance::ApplicationInstance() :
  manager_(*this) {
  LoggerD("Entered");

  app_id_ = CurrentApplication::GetInstance().GetApplicationId();
  LoggerD("app_id: %s", app_id_.c_str());

  if (app_id_.empty()) {
    LoggerE("app_id_ is empty. Application instance will not be created.");
    //return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot find caller.");
  }

  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
        RegisterSyncHandler(c, std::bind(&ApplicationInstance::x, this, _1, _2));
  //ApplicationManager
  REGISTER_SYNC("ApplicationManager_getCurrentApplication", GetCurrentApplication);
  REGISTER_SYNC("ApplicationManager_getAppContext", GetAppContext);
  REGISTER_SYNC("ApplicationManager_getAppInfo", GetAppInfo);
  REGISTER_SYNC("ApplicationManager_getAppCerts", GetAppCerts);
  REGISTER_SYNC("ApplicationManager_getAppSharedURI", GetAppSharedURI);
  REGISTER_SYNC("ApplicationManager_getAppMetaData", GetAppMetaData);
  REGISTER_SYNC("ApplicationManager_addAppInfoEventListener", AddAppInfoEventListener);
  REGISTER_SYNC("ApplicationManager_removeAppInfoEventListener", RemoveAppInfoEventListener);

  //Application
  REGISTER_SYNC("Application_getRequestedAppControl", GetRequestedAppControl);
  REGISTER_SYNC("Application_broadcastEvent", BroadcastEvent);
  REGISTER_SYNC("Application_broadcastTrustedEvent", BroadcastTrustedEvent);
  REGISTER_SYNC("Application_addEventListener", AddEventListener);
  REGISTER_SYNC("Application_removeEventListener", RemoveEventListener);

  //RequestedApplicationControl
  REGISTER_SYNC("RequestedApplicationControl_replyResult", ReplyResult);
  REGISTER_SYNC("RequestedApplicationControl_replyFailure", ReplyFailure);

  //ApplicationInformation
  REGISTER_SYNC("ApplicationInformation_getSize", GetSize);
#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
        RegisterSyncHandler(c, std::bind(&ApplicationInstance::x, this, _1, _2));
  //ApplicationManager
  REGISTER_ASYNC("ApplicationManager_kill", Kill);
  REGISTER_ASYNC("ApplicationManager_launch", Launch);
  REGISTER_ASYNC("ApplicationManager_launchAppControl", LaunchAppControl);
  REGISTER_ASYNC("ApplicationManager_findAppControl", FindAppControl);
  REGISTER_ASYNC("ApplicationManager_getAppsContext", GetAppsContext);
  REGISTER_ASYNC("ApplicationManager_getAppsInfo", GetAppsInfo);
#undef REGISTER_ASYNC
}

ApplicationInstance::~ApplicationInstance() {
  LoggerD("Entered");
}

void ApplicationInstance::GetCurrentApplication(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.GetCurrentApplication(app_id_, &out);
}

void ApplicationInstance::GetAppContext(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.GetAppContext(args, &out);
}

void ApplicationInstance::GetAppInfo(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  std::string app_id = app_id_;
  const auto& id = args.get("id");
  if (id.is<std::string>()) {
    app_id = id.get<std::string>();
  }

  manager_.GetAppInfo(app_id, &out);
}

void ApplicationInstance::GetAppCerts(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  std::string app_id = app_id_;
  const auto& id = args.get("id");
  if (id.is<std::string>()) {
    app_id = id.get<std::string>();
  }

  manager_.GetAppCerts(app_id, &out);
}

void ApplicationInstance::GetAppSharedURI(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  std::string app_id = app_id_;
  const auto& id = args.get("id");
  if (id.is<std::string>()) {
    app_id = id.get<std::string>();
  }

  manager_.GetAppSharedUri(app_id, &out);
}

void ApplicationInstance::GetAppMetaData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  std::string app_id = app_id_;
  const auto& id = args.get("id");
  if (id.is<std::string>()) {
    app_id = id.get<std::string>();
  }

  manager_.GetAppMetaData(app_id, &out);
}

void ApplicationInstance::AddAppInfoEventListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.StartAppInfoEventListener(&out);
 }

void ApplicationInstance::RemoveAppInfoEventListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.StopAppInfoEventListener();
  ReportSuccess(out);
}

void ApplicationInstance::GetRequestedAppControl(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  current_application_.GetRequestedAppControl(args, &out);
}

void ApplicationInstance::ReplyResult(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  current_application_.app_control().ReplyResult(args, &out);
}

void ApplicationInstance::ReplyFailure(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  current_application_.app_control().ReplyFailure(&out);
}

void ApplicationInstance::GetSize(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.GetApplicationInformationSize(args, &out);
}

void ApplicationInstance::Kill(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.Kill(args);
}

void ApplicationInstance::Launch(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.Launch(args);
}

void ApplicationInstance::LaunchAppControl(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.LaunchAppControl(args);
}

void ApplicationInstance::FindAppControl(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.FindAppControl(args);
}

void ApplicationInstance::GetAppsContext(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.GetAppsContext(args);
}

void ApplicationInstance::GetAppsInfo(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.GetAppsInfo(args);
}

void ApplicationInstance::BroadcastEvent(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.BroadcastEventHelper(args, out, false);
}

void ApplicationInstance::BroadcastTrustedEvent(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  manager_.BroadcastEventHelper(args, out, true);
}

void ApplicationInstance::AddEventListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  const std::string& event_name = args.get("name").get<std::string>();

  LOGGER(DEBUG) << "event_name: " << event_name;

  JsonCallback cb = [this, args](picojson::value* event) -> void {
   picojson::object& event_o = event->get<picojson::object>();
   event_o["listenerId"] = args.get("listenerId");
   LOGGER(DEBUG) << event->serialize().c_str();
   Instance::PostMessage(this, event->serialize().c_str());
   LOGGER(DEBUG) << event->serialize().c_str();
  };

  PlatformResult result = manager_.StartEventListener(event_name, cb);
  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void ApplicationInstance::RemoveEventListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  const std::string& event_name = args.get("name").get<std::string>();

  LOGGER(DEBUG) << "event_name: " << event_name;

  manager_.StopEventListener(event_name);
}

}  // namespace application
}  // namespace extension
