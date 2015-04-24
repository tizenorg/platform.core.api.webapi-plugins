// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_instance.h"

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

namespace extension {
namespace application {

namespace {
// The privileges that are required in Application API
const std::string kPrivilegeAppManagerCertificate = "http://tizen.org/privilege/appmanager.certificate";
const std::string kPrivilegeAppManagerKill = "http://tizen.org/privilege/appmanager.kill";
const std::string kPrivilegeApplicationInfo = "http://tizen.org/privilege/application.info";
const std::string kPrivilegeApplicationLaunch = "http://tizen.org/privilege/application.launch";
}  // namespace

using namespace common;

ApplicationInstance::ApplicationInstance(const std::string& app_id) :
  manager_(*this),
  app_id_(app_id) {
  LoggerD("Entered");

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

  CHECK_PRIVILEGE_ACCESS(kPrivilegeAppManagerCertificate, &out);

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

  CHECK_PRIVILEGE_ACCESS(kPrivilegeApplicationInfo, &out);

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

  CHECK_PRIVILEGE_ACCESS(kPrivilegeApplicationInfo, &out);

  manager_.GetApplicationInformationSize(args, &out);
}

void ApplicationInstance::Kill(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  CHECK_PRIVILEGE_ACCESS(kPrivilegeAppManagerKill, &out);

  manager_.Kill(args);
}

void ApplicationInstance::Launch(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  CHECK_PRIVILEGE_ACCESS(kPrivilegeApplicationLaunch, &out);

  manager_.Launch(args);
}

void ApplicationInstance::LaunchAppControl(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  CHECK_PRIVILEGE_ACCESS(kPrivilegeApplicationLaunch, &out);

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

}  // namespace application
}  // namespace extension
