// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_instance.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <sstream>
#include <unistd.h>
#include <pthread.h>
#include <glib.h>

#include <app.h>

// to launch app by aul
#include <aul.h>

// to get package name by appid
#include <app_info.h>
#include <app_manager.h>

// To get cert information from package
#include <package_manager.h>
#include <package-manager.h>

// To get app size and installed time
#include <pkgmgr-info.h>

#include <plugins-ipc-message/ipc_message_support.h>

// To get ppid
#include <unistd.h>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include "application.h"
#include "application_context.h"

namespace extension {
namespace application {

namespace {
// The privileges that required in Application API
const std::string kPrivilegeApplication = "";

} // namespace

using namespace common;
using namespace extension::application;

static int category_cb(const char *category, void *user_data) {
  LoggerD("category: %s", category);
  if (category == NULL)
    return true;

  ApplicationInformation* appInfo = (ApplicationInformation*)user_data;
  appInfo->add_categories(category);
  return true;
}


ApplicationInstance::ApplicationInstance(const std::string& app_id) {
  manager_handle_ = NULL;
  watch_id_ = 0;
  app_id_ = app_id;

  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ApplicationInstance::x, this, _1, _2));
  REGISTER_SYNC("ApplicationManager_getAppCerts", ApplicationManagerGetappcerts);
  REGISTER_SYNC("Application_getRequestedAppControl", ApplicationGetrequestedappcontrol);
  REGISTER_SYNC("ApplicationManager_addAppInfoEventListener", ApplicationManagerAddappinfoeventlistener);
  REGISTER_SYNC("ApplicationManager_getAppMetaData", ApplicationManagerGetappmetadata);
  REGISTER_SYNC("ApplicationManager_launchAppControl", ApplicationManagerLaunchappcontrol);
  REGISTER_SYNC("ApplicationManager_removeAppInfoEventListener", ApplicationManagerRemoveappinfoeventlistener);
  REGISTER_SYNC("ApplicationManager_getAppInfo", ApplicationManagerGetappinfo);
  REGISTER_SYNC("ApplicationManager_getAppSharedURI", ApplicationManagerGetappshareduri);
  REGISTER_SYNC("RequestedApplicationControl_replyResult", RequestedApplicationControlReplyresult);
  REGISTER_SYNC("ApplicationManager_kill", ApplicationManagerKill);
  REGISTER_SYNC("ApplicationManager_getAppsInfo", ApplicationManagerGetappsinfo);
  REGISTER_SYNC("ApplicationManager_launch", ApplicationManagerLaunch);
  REGISTER_SYNC("Application_hide", ApplicationHide);
  REGISTER_SYNC("ApplicationManager_getAppsContext", ApplicationManagerGetappscontext);
  REGISTER_SYNC("ApplicationManager_getAppContext", ApplicationManagerGetappcontext);
  REGISTER_SYNC("RequestedApplicationControl_replyFailure", RequestedApplicationControlReplyfailure);
  REGISTER_SYNC("Application_exit", ApplicationExit);
  REGISTER_SYNC("ApplicationManager_getCurrentApplication", ApplicationManagerGetcurrentapplication);
  REGISTER_SYNC("ApplicationManager_findAppControl", ApplicationManagerFindappcontrol);
  #undef REGISTER_SYNC

  set_installed_size(GetAppInstalledSize(app_id_));
/*
  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id_.c_str(), &handle);
  if (ret == PMINFO_R_OK) {
    ret = pkgmgrinfo_appinfo_foreach_category(handle, category_cb, NULL);
    if (ret != PMINFO_R_OK) {
      LoggerD("Failed to get category info");
      pkgmgrinfo_appinfo_destroy_appinfo(handle);
    } else {
      LoggerD("Waiting for callback");
    }
  } else {
    LoggerD("Failed to get handle"); 
  }
*/
}

ApplicationInstance::~ApplicationInstance() {
  LoggerD("Destructor of ApplicationInstance called.");
}


enum ApplicationErrors {
  APP_ERROR_OK = 0,
  APP_ERROR_UNKNOWN = 1,
  APP_ERROR_TYPE_MISMATCH = 2,
  APP_ERROR_IO = 3,
  APP_ERROR_SVC_NOT_AVAILABLE = 4,
  APP_ERROR_SECURITY = 5,
  APP_ERROR_NETWORK = 6,
  APP_ERROR_NOT_SUPPORTED = 7,
  APP_ERROR_NOT_FOUND = 8,
  APP_ERROR_INVALID_ACCESS = 9,
  APP_ERROR_ABORT = 10,
  APP_ERROR_QUOTA_EXCEEDED = 11,
  APP_ERROR_INVALID_STATE = 12,
  APP_ERROR_INVALID_MODIFICATION = 13,
};

enum ApplicationCallbacks {
  ApplicationManagerGetappcertsCallback, 
  ApplicationGetrequestedappcontrolCallback, 
  ApplicationManagerAddappinfoeventlistenerCallback, 
  ApplicationManagerGetappmetadataCallback, 
  ApplicationManagerLaunchappcontrolCallback, 
  ApplicationManagerRemoveappinfoeventlistenerCallback, 
  ApplicationManagerGetappinfoCallback, 
  ApplicationManagerGetappshareduriCallback, 
  RequestedApplicationControlReplyresultCallback, 
  ApplicationManagerKillCallback, 
  ApplicationManagerGetappsinfoCallback, 
  ApplicationManagerLaunchCallback, 
  ApplicationHideCallback, 
  ApplicationManagerGetappscontextCallback, 
  ApplicationManagerGetappcontextCallback, 
  RequestedApplicationControlReplyfailureCallback, 
  ApplicationExitCallback, 
  ApplicationManagerGetcurrentapplicationCallback, 
  ApplicationManagerFindappcontrolCallback
};

struct CallbackInfo {
  ApplicationInstance* instance;
  bool is_success;
  int error_type;
  char error_msg[256];
  char id[256];
  int callback_id;
};

static picojson::value GetAppError(int type, const char* msg) {
  switch (type) {
  case APP_ERROR_UNKNOWN:
    return UnknownException(msg).ToJSON();
  case APP_ERROR_TYPE_MISMATCH:
    return TypeMismatchException(msg).ToJSON();
  case APP_ERROR_IO:
    return IOException(msg).ToJSON();
  case APP_ERROR_SVC_NOT_AVAILABLE:
    return ServiceNotAvailableException(msg).ToJSON();
  case APP_ERROR_SECURITY:
    return SecurityException(msg).ToJSON();
  case APP_ERROR_NETWORK:
    return NetworkException(msg).ToJSON();
  case APP_ERROR_NOT_SUPPORTED:
    return NotSupportedException(msg).ToJSON();
  case APP_ERROR_NOT_FOUND:
    return NotFoundException(msg).ToJSON();
  case APP_ERROR_INVALID_ACCESS:
    return InvalidAccessException(msg).ToJSON();
  case APP_ERROR_ABORT:
    return AbortException(msg).ToJSON();
  case APP_ERROR_QUOTA_EXCEEDED:
    return QuotaExceededException(msg).ToJSON();
  case APP_ERROR_INVALID_STATE:
    return InvalidStateException(msg).ToJSON();
  case APP_ERROR_INVALID_MODIFICATION:
    return InvalidModificationException(msg).ToJSON();
  default:
    return UnknownException(msg).ToJSON();
  }
}
static void ReplyAsync(ApplicationInstance* instance, ApplicationCallbacks cbfunc, 
                       int callback_id, bool isSuccess, picojson::object& param, int err_id, const char* err_msg) {
  param["callbackId"] = picojson::value(static_cast<double>(callback_id));
  if (isSuccess) {
    param["status"] = picojson::value("success");
  } else {
    param.insert(std::make_pair("status", picojson::value("error")));
    param.insert(std::make_pair("error", GetAppError(err_id, err_msg)));
  }
  
  // insert result for async callback to param
  switch(cbfunc) {
    case ApplicationManagerGetcurrentapplicationCallback: {
      // do something...
      break;
    }
    case ApplicationManagerKillCallback: {
      LoggerD("ApplicationManagerKillCallbak called");
      break;
    }
    case ApplicationManagerLaunchCallback: {
      LoggerD("ApplicationManagerLaunchCallbak called");
      break;
    }
    case ApplicationManagerLaunchappcontrolCallback: {
      LoggerD("ApplicationManagerLaunchCallbak called");
      break;
    }
    case ApplicationManagerFindappcontrolCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappscontextCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappcontextCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappsinfoCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappinfoCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappcertsCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappshareduriCallback: {
      // do something...
      break;
    }
    case ApplicationManagerGetappmetadataCallback: {
      // do something...
      break;
    }
    case ApplicationManagerAddappinfoeventlistenerCallback: {
      LoggerD("ApplicationManagerAppappinfoeventlistenerCallback called");
      break;
    }
    case ApplicationManagerRemoveappinfoeventlistenerCallback: {
      // do something...
      break;
    }
    case ApplicationExitCallback: {
      // do something...
      break;
    }
    case ApplicationHideCallback: {
      // do something...
      break;
    }
    case ApplicationGetrequestedappcontrolCallback: {
      // do something...
      break;
    }
    case RequestedApplicationControlReplyresultCallback: {
      // do something...
      break;
    }
    case RequestedApplicationControlReplyfailureCallback: {
      // do something...
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
      return;
    }
  }

  picojson::value result = picojson::value(param);
  LoggerD("async result: %s", result.serialize().c_str());
  instance->PostMessage(result.serialize().c_str());
}

static ApplicationInformationPtr get_app_info(pkgmgrinfo_appinfo_h handle) {
  char* app_id = NULL;
  char* name = NULL;
  char* icon_path = NULL;
  bool no_display = false;
  char* pkg_id = NULL;
  int ret = 0;

  ApplicationInformationPtr app_info(new ApplicationInformation());
  ret = pkgmgrinfo_appinfo_get_appid(handle, &app_id);
  if (ret != PMINFO_R_OK) {
    LoggerD("Fail to get appid");
  } else {
    app_info->set_app_id(app_id);
  }

  ret = pkgmgrinfo_appinfo_get_label(handle, &name);
  if ((ret != PMINFO_R_OK) || (name == NULL)) {
    LoggerD("Fail to get label");
  } else {
    app_info->set_name(name);
  }

  ret = pkgmgrinfo_appinfo_get_icon(handle, &icon_path);
  if ((ret != PMINFO_R_OK) || (icon_path == NULL)) {
    LoggerD("Fail to get icon");
  } else {
    app_info->set_icon_path(icon_path);
  }

  ret = pkgmgrinfo_appinfo_foreach_category(handle, category_cb, (void*)app_info.get());
  if (ret != PMINFO_R_OK) {
    LoggerD("Fail to get categories");
  } else {
    LoggerD("Waiting for callback response...");
  }

  ret = pkgmgrinfo_appinfo_is_nodisplay(handle, &no_display);
  if (ret != PMINFO_R_OK) {
    LoggerD("Fail to get nodisplay");
  } else {
    app_info->set_show(!no_display);
  }

  ret = pkgmgrinfo_appinfo_get_pkgid(handle, &pkg_id);
  if ((ret != PMINFO_R_OK) || (pkg_id == NULL)) {
    LoggerD("Fail to get pkgId");
    return app_info;
  } else {
    app_info->set_package_id(pkg_id);
  }

  char* version = NULL;
  int installed_time = 0;
  pkgmgrinfo_pkginfo_h pkginfo_h;

  ret = pkgmgrinfo_pkginfo_get_pkginfo(pkg_id, &pkginfo_h);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get pkginfo");
  } else {
    ret = pkgmgrinfo_pkginfo_get_version(pkginfo_h, &version);
    if (ret != PMINFO_R_OK) {
      LoggerE("Fail to get version");
    } else {
      app_info->set_version(version);
    }

    ret = pkgmgrinfo_pkginfo_get_installed_time(pkginfo_h, &installed_time);
    if (ret != PMINFO_R_OK) {
      LoggerE("Fail to get installed time");
    } else {
      app_info->set_install_date(installed_time);
      LoggerD("installed_time: %d", installed_time);
    }

    pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_h);
  }

  // size
  return app_info;
}

// Callback from 'app_manager_set_app_context_event_cb'
// Used by 'kill'
static void app_manager_app_context_event_callback(app_context_h app_context,
  app_context_event_e event, void *user_data) {

  CallbackInfo *info = (CallbackInfo*)user_data;
  int ret = 0;

  LoggerD("context_id: %s, callback_id: %d", info->id, info->callback_id);
  if(event != APP_CONTEXT_EVENT_TERMINATED) {
    picojson::object data;
    info->error_type = APP_ERROR_UNKNOWN;
    sprintf(info->error_msg, "Not terminated.");
    ReplyAsync(info->instance, ApplicationManagerKillCallback, info->callback_id, 
               false, data, info->error_type, info->error_msg);
  } else {
    picojson::object data;
    ReplyAsync(info->instance, ApplicationManagerKillCallback, info->callback_id,
               true, data, 0, NULL);
  }

  if (user_data)
    free(user_data);
}

static gboolean LaunchCompleted(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered");
  picojson::object data;
   
  if (user_data->is_success) {
    ReplyAsync(user_data->instance, ApplicationManagerLaunchCallback, user_data->callback_id,
               true, data, 0, NULL);
  } else {
    ReplyAsync(user_data->instance, ApplicationManagerLaunchCallback, user_data->callback_id,
               false, data, user_data->error_type, user_data->error_msg);
  }
    return true;
}
static void* LaunchThread(const std::shared_ptr<CallbackInfo>& user_data)
{
  int ret;

  LoggerD("app_id: %s, callback_id: %d", user_data->id, user_data->callback_id);

  ret = aul_open_app(user_data->id);
  if (ret < 0) {
    std::string msg;
    int type = APP_ERROR_OK;
    switch (ret) {
      case AUL_R_EINVAL:
      case AUL_R_ERROR:
        LoggerE("Not Found error");
        msg = "Not Found error";
        type = APP_ERROR_NOT_FOUND;
        break;
      case AUL_R_ECOMM:
        LoggerE("Internal IPC error");
        msg = "Internal IPC error";
        type = APP_ERROR_IO;
        break;
      default:
        LoggerE("Unknown error");
        msg = "Unknown error";
        type = APP_ERROR_UNKNOWN;
        break;
    }
    user_data->is_success = false;
    user_data->error_type = type;
    sprintf(user_data->error_msg, msg.c_str());
  } else {
    LoggerD("Success to launch.");
    user_data->is_success = true; 
  }
} 

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void ApplicationInstance::ApplicationManagerGetcurrentapplication(const picojson::value& args, picojson::object& out) {

  try {
    ApplicationPtr app = GetCurrentApplication(app_id_);
    LoggerD("context id = %s", app->get_context_id().c_str());
    ReportSuccess(app->Value(), out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerKill(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& contextId = args.get("contextId").get<std::string>();

  LoggerD("callbackId = %d", callbackId);
  LoggerD("contextId = %s", contextId.c_str());

  try {
    Kill(contextId, callbackId);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerLaunch(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& id = args.get("id").get<std::string>();

  LoggerD("callbackId = %d", callbackId);
  LoggerD("appId = %s", id.c_str());

  try {
    Launch(id, callbackId);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerLaunchappcontrol(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& id = args.get("id").get<std::string>();

  LoggerD("callbackId = %d", callbackId);
  LoggerD("appId = %s", id.c_str());

  try {
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerFindappcontrol(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::ApplicationManagerGetappscontext(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::ApplicationManagerGetappcontext(const picojson::value& args, picojson::object& out) {
  const std::string& contextId = args.get("contextId").get<std::string>();

  LoggerD("contextId = %s", contextId.c_str());

  try {
    ApplicationContextPtr appCtx;
    if (contextId.compare("null") == 0)
      appCtx = GetAppContext("");
    else
      appCtx = GetAppContext(contextId);

    LoggerD("appCtx: id = %s", appCtx->get_context_id().c_str());
    LoggerD("appCtx: appId = %s", appCtx->get_app_id().c_str());

    ReportSuccess(picojson::value(appCtx->Value()), out);
  } catch (const PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerGetappsinfo(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::ApplicationManagerGetappinfo(const picojson::value& args, picojson::object& out) {
  std::string id;
  if (args.contains("id")) {
    id = args.get("id").get<std::string>();
    if (id.empty()) {
      LoggerD("Id is null. use current app id");
      id = app_id_;
    }
  } else {
    id = app_id_;
  }

  LoggerD("app_id = %s", id.c_str());

  try {
    ApplicationInformationPtr app_info_ptr = GetAppInfo(id);
    ReportSuccess(app_info_ptr->Value(), out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }

}
void ApplicationInstance::ApplicationManagerGetappcerts(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::ApplicationManagerGetappshareduri(const picojson::value& args, picojson::object& out) {
  std::string id;
  if (args.contains("id")) {
    id = args.get("id").get<std::string>();
    if (id.empty()) {
      LoggerD("Id is null. use current app id");
      id = app_id_;
    }
  } else {
    id = app_id_;
  }

  LoggerD("app_id = %s", id.c_str());

  try {
    const std::string& ret = GetAppSharedURI(id);
    ReportSuccess(picojson::value(ret), out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerGetappmetadata(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::ApplicationManagerAddappinfoeventlistener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  try {
    const double ret = static_cast<double>(AddAppInfoEventListener(callbackId));
    ReportSuccess(picojson::value(ret), out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationManagerRemoveappinfoeventlistener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)
  long watch_id = static_cast<long>(args.get("watchId").get<double>());

  try {
    RemoveAppInfoEventListener(watch_id);
    ReportSuccess(out);
  } catch (const PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationExit(const picojson::value& args, picojson::object& out) {
  LoggerD("Hide is called");

  try {
    //Blink
    //IPCSupport::Instance().Post(IPCMsg::MsgExitApp(), "" );
    IPCMessageSupport::sendAsyncMessageToUiProcess(IPCMessageSupport::TIZEN_EXIT, NULL, NULL, NULL);
    ReportSuccess(out);
  } catch (const PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationHide(const picojson::value& args, picojson::object& out) {
  LoggerD("Hide is called");
 
  try {
    //Blink
    //IPCSupport::Instance().Post(IPCMsg::MsgHideApp(), "" );

    IPCMessageSupport::sendAsyncMessageToUiProcess(IPCMessageSupport::TIZEN_HIDE, NULL, NULL, NULL);
    ReportSuccess(out);
  } catch (const PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}
void ApplicationInstance::ApplicationGetrequestedappcontrol(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::RequestedApplicationControlReplyresult(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ApplicationInstance::RequestedApplicationControlReplyfailure(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
ApplicationPtr ApplicationInstance::GetCurrentApplication(const std::string app_id) {

  LoggerD("app_id: %s", app_id.c_str());  

  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get appInfo");
    throw UnknownException("pkgmgrinfo_appinfo_get_appinfo error : unknown error");
  }

  ApplicationInformationPtr app_info_ptr = get_app_info(handle);
  app_info_ptr->set_size(get_installed_size());
 
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  Application *app = new Application();
  ApplicationPtr app_ptr = ApplicationPtr(app);
  app_ptr->set_app_info(app_info_ptr);

  LoggerD("set appinfo to application");
  {
    int pid = getppid();
    LoggerD("context id = %d", pid);

    std::stringstream sstr;
    sstr << pid;
    app_ptr->set_context_id(sstr.str());
  }
  
  return app_ptr;
}
ApplicationInformationPtr ApplicationInstance::GetAppInfo(const std::string app_id) {

  LoggerD("app_id: %s", app_id.c_str());  

  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get appInfo");
    throw NotFoundException("Given app id is not found");
  }

  ApplicationInformationPtr app_info_ptr = get_app_info(handle);
  if (app_id.compare(app_id_) == 0) {
    app_info_ptr->set_size(get_installed_size());
  } else {
    app_info_ptr->set_size(GetAppInstalledSize(app_id));   
  }
 
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  return app_info_ptr;
}
void ApplicationInstance::Kill(const std::string contextId, int callbackId) {
  if (contextId.empty()) {
    LoggerE("contextId is mandatory field.");
    throw InvalidValuesException("Context id is mandatory field.");
  }

  int ret;
  int pid;
  std::stringstream(contextId) >> pid;

  if (pid <= 0) {
    LoggerE("Given context id is wrong.");
    throw InvalidValuesException("Given context id is wrong.");
  }

  // if kill request is come for current context, throw InvalidValueException by spec
  if (pid == getppid()) {
    LoggerE("Given context id is same with me.");
    throw InvalidValuesException("Given context id is same with me.");
  }

  char *appIdCStr = NULL;
  ret = app_manager_get_app_id(pid, &appIdCStr);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Error while getting app id");
    throw NotFoundException("Error while getting app id");
  }

  std::string appId = appIdCStr;
  free(appIdCStr);

  app_context_h appContext;
  ret = app_manager_get_app_context (appId.c_str(), &appContext);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Error while getting app context");
    throw NotFoundException("Error while getting app context");
  }
  
  CallbackInfo* info = (CallbackInfo*)malloc(sizeof(CallbackInfo));
  info->instance = this;
  sprintf(info->id, "%s", contextId.c_str()); 
  info->callback_id = callbackId;

  // TODO thread
  ret = app_manager_set_app_context_event_cb(app_manager_app_context_event_callback, info);
  if (ret != APP_MANAGER_ERROR_NONE) {
    if (info)
      free(info);
    LoggerE("Error while registering app context event");
    throw InvalidValuesException("Error while registering app context event");
  }

  ret = app_manager_terminate_app(appContext);
  if (ret != APP_MANAGER_ERROR_NONE) {
    if (info)
      free(info);
    LoggerE("Error while terminating app");
    throw InvalidValuesException("Error while terminating app");
  }

}
void ApplicationInstance::Launch(const std::string appId, int callbackId) {
  if (appId.empty()) {
    LoggerE("appId is mandatory field.");
    throw InvalidValuesException("App id is mandatory field.");
  }

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  sprintf(user_data->id, "%s", appId.c_str());
  user_data->callback_id = callbackId;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(LaunchThread, LaunchCompleted, user_data);
}
 
ApplicationContextPtr ApplicationInstance::GetAppContext(const std::string contextId) {
  int ret = 0;

  LoggerD("contextId: %s", contextId.c_str());

  std::string curCtxId = contextId;
  int pid;

  if (curCtxId.empty()) {
    pid = getppid();
    std::stringstream sstr;
    sstr << pid;
    curCtxId = sstr.str();
  }
  else {
    std::stringstream(contextId) >> pid;
    if (pid <= 0) {
      LoggerE("Given contextId is wrong");
      throw NotFoundException("Given contextId is wrong");
    }
  }

  char *app_id = NULL;
  LoggerD("pid: %d", pid);

  ret = app_manager_get_app_id(pid, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    if (app_id) {
      free(app_id);
    }
    switch (ret) {
      case APP_MANAGER_ERROR_INVALID_PARAMETER:
        LoggerE("app_manager_get_app_id error : invalid parameter");
          throw NotFoundException("app_manager_get_app_id error : invalid parameter");
      case APP_MANAGER_ERROR_NO_SUCH_APP:
        LoggerE("app_manager_get_app_id error : no such app");
          throw NotFoundException("app_manager_get_app_id error : no such app");
      case APP_MANAGER_ERROR_DB_FAILED:
        LoggerE("app_manager_get_app_id error : db failed");
          throw NotFoundException("app_manager_get_app_id error : db failed");
      case APP_MANAGER_ERROR_OUT_OF_MEMORY:
        LoggerE("app_manager_get_app_id error : out of memory");
          throw NotFoundException("app_manager_get_app_id error : out of memory");
      default:
        LoggerE("app_manager_get_app_id error");
          throw UnknownException("app_manager_get_app_id error : unknown error");
    }
  }
  
  ApplicationContextPtr appContext(new ApplicationContext());
  appContext->set_app_id(app_id);
  appContext->set_context_id(curCtxId);

  if(app_id)
    free(app_id);

  return appContext;
}

std::string ApplicationInstance::GetAppSharedURI(const std::string app_id) {

#define TIZENAPIS_APP_FILE_SCHEME      "file://"
#define TIZENAPIS_APP_SLASH            "/"
#define TIZENAPIS_APP_SHARED           "shared"

  app_info_h handle;
  char* pkg_name = NULL;
  
  int ret = app_manager_get_app_info(app_id.c_str(), &handle);
  if (ret != APP_ERROR_NONE) {
    LoggerD("Fail to get appinfo");
    throw NotFoundException("Fail to get appinfo");
  }

  ret = app_info_get_package(handle, &pkg_name);
  if ((ret != APP_ERROR_NONE) || (pkg_name == NULL)) {
    LoggerD("Fail to get pkg_name");
    throw NotFoundException("Fail to get pkg_name");
  }

  app_info_destroy(handle);

  pkgmgrinfo_pkginfo_h pkginfo_h;
  char* root_path = NULL;

  ret = pkgmgrinfo_pkginfo_get_pkginfo(pkg_name, &pkginfo_h);
  if (ret != PMINFO_R_OK) {
    free(pkg_name);
    throw UnknownException("Fail to get pkginfo");
  }

  ret = pkgmgrinfo_pkginfo_get_root_path(pkginfo_h, &root_path);
  if ((ret != PMINFO_R_OK) || (root_path == NULL)) {
     LoggerE("Fail to get root path");
     free(pkg_name);
     throw UnknownException("Fail to get root path");
  }

  std::string sharedURI = TIZENAPIS_APP_FILE_SCHEME + std::string(root_path) + TIZENAPIS_APP_SLASH + TIZENAPIS_APP_SHARED + TIZENAPIS_APP_SLASH;
  free(pkg_name);

  pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_h);

  return sharedURI;
}

int ApplicationInstance::AddAppInfoEventListener(const int& callback_id) {

  if (manager_handle_ != NULL) {
    LoggerD("AppListChanged callback is already registered. watch_id_ = %d", watch_id_);
    throw UnknownException("Listener is already registered.");    
  }
  LoggerD("pkgmgr_client_new() ----1----");
  manager_handle_ = pkgmgr_client_new(PC_LISTENING);
  LoggerD("pkgmgr_client_new() ----2----");
  if (manager_handle_ == NULL) {
    throw UnknownException("Error while registering listener to pkgmgr");
  }

  pkgmgr_client_listen_status(manager_handle_, app_list_changed_cb, this); 

  callback_id_list_.push_back(callback_id);
  return get_watch_id_and_increase();
}
int ApplicationInstance::app_list_changed_cb(int id, const char *type, const char *package, const char *key, const char *val, const void *msg, void *data)
{
  static app_info_event_e event_type;

  LoggerD("ENTERED");

  // pre-process
  if (!strcasecmp(key, "start")) {
    if (!strcasecmp(val, "install")) {
      LoggerD("start: install");
      event_type = APP_INFO_EVENT_INSTALLED;
    } else if (!strcasecmp(val, "uninstall")) {
      // After uninstallation, we cannot get app ids from package name.
      // So, we have to store app ids which is included to target package.
      LoggerD("start: uninstall");
      package_info_h package_info;

      int ret = package_info_create(package, &package_info);
      if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LoggerE("Cannot create package info");
      }

      ret = package_info_foreach_app_from_package(package_info, PACKAGE_INFO_ALLAPP, app_callback, data);
      if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LoggerE("failed while getting appids");
      }
      ret = package_info_destroy(package_info);
      if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LoggerE("Cannot destroy package info");
      }
      event_type = APP_INFO_EVENT_UNINSTALLED;
    } else if (!strcasecmp(val, "update")) {
      LoggerD("start: update");
      event_type = APP_INFO_EVENT_UPDATED;
    }
  // post-process
  } else if (!strcasecmp(key, "end") && !strcasecmp(val, "ok")) {
    if (event_type >= 0) {
      if (data != NULL) {
        LoggerD("end & ok is called: package = %s", package);
        ApplicationInstance* app_instance = (ApplicationInstance*)data;
        app_instance->ReplyAppListChangedCallback(event_type, package, data);
      }
    }
  }

  return APP_MANAGER_ERROR_NONE;
}

bool ApplicationInstance::app_callback(package_info_app_component_type_e comp_type, const char *app_id, void *user_data)
{
  LoggerD("ENTERED");

  if(app_id == NULL) {
    LoggerE("Callback is called. but no package name is passed. skip this request");
    return true;
  }

  if(user_data == NULL) {
    LoggerE("user data is not exist. skip this request");
    return true;
  }

  LoggerD("app_id = %s", app_id);
  ApplicationInstance* app_instance = (ApplicationInstance*)user_data;
  app_instance->app_list_.push_back(app_id);

  return true;
}

void ApplicationInstance::ReplyAppListChangedCallback(app_info_event_e event_type, const char* pkg_id, void* user_data) {
  LoggerD("ENTERED");  

  ApplicationInstance* app_instance = (ApplicationInstance*)user_data;

  if (event_type == APP_INFO_EVENT_UNINSTALLED) {
    for (size_t i = 0; i < app_list_.size(); i++) {

      // onuninstalled
      LoggerD("onuninstalled: %d of %d", i, app_list_.size());
      std::string app_id = app_list_.at(i);

      for (size_t j = 0; j < callback_id_list_.size(); j++) {
        int callback_id = callback_id_list_.at(j);
        LoggerD("%d th callback_id(%d) of %d", j, callback_id, callback_id_list_.size());
        picojson::object data;
        data.insert(std::make_pair("type", picojson::value("onuninstalled")));
        data.insert(std::make_pair("id", picojson::value(app_id)));
        ReplyAsync(this, ApplicationManagerAddappinfoeventlistenerCallback, 
                   callback_id, true, data, 0, NULL);
      }
    }
  } else {
    package_info_h package_info;

    int ret = package_info_create(pkg_id, &package_info);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      LoggerE("Cannot create package info");
      return;
    }

    // app_callback is called immediately 
    ret = package_info_foreach_app_from_package(package_info, PACKAGE_INFO_ALLAPP, app_callback, user_data);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      LoggerE("failed while getting appids");
      package_info_destroy(package_info);
      return;
    }

    ret = package_info_destroy(package_info);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      LoggerE("Cannot destroy package info");
    }

    for (size_t i = 0; i < app_list_.size(); i++) {
      switch(event_type) {
        case APP_INFO_EVENT_INSTALLED:
          {
            // oninstalled
            LoggerD("oninstalled: %d of %d", i, app_list_.size());
            std::string app_id = app_list_.at(i);

            ApplicationInformationPtr app_info_ptr = GetAppInfo(app_id);

            for (size_t j = 0; j < callback_id_list_.size(); j++) {
              int callback_id = callback_id_list_.at(j);
              LoggerD("%d th callback_id(%d) of %d", j, callback_id, callback_id_list_.size());
              picojson::object data;
              data.insert(std::make_pair("type", picojson::value("oninstalled")));
              data.insert(std::make_pair("info", picojson::value(app_info_ptr->Value())));
              ReplyAsync(this, ApplicationManagerAddappinfoeventlistenerCallback, 
                         callback_id, true, data, 0, NULL);
            }
          }
          break;
        case APP_INFO_EVENT_UPDATED:
          {
            // onupdated
            LoggerD("onupdated: %d of %d", i, app_list_.size());
            std::string app_id = app_list_.at(i);

            ApplicationInformationPtr app_info_ptr = GetAppInfo(app_id);

            for (size_t j = 0; j < callback_id_list_.size(); j++) {
              int callback_id = callback_id_list_.at(j);
              LoggerD("%d th callback_id(%d) of %d", j, callback_id, callback_id_list_.size());
              picojson::object data;
              data.insert(std::make_pair("type", picojson::value("onupdated")));
              data.insert(std::make_pair("info", picojson::value(app_info_ptr->Value())));
              ReplyAsync(this, ApplicationManagerAddappinfoeventlistenerCallback, 
                         callback_id, true, data, 0, NULL);
            }
          }
          break;
        default:
          LoggerE("app_manager listener gave wrong event_type");
          break;
      }
    }
  }

  // clean-up applist
  app_list_.clear();
}

void ApplicationInstance::RemoveAppInfoEventListener(long watch_id) {
  LoggerD("RemoveAppInfoEventListener called. watch_id = %d", watch_id);

  if (manager_handle_ == NULL) {
    LoggerE("Listener is not added before.");
    throw UnknownException("Listener is not added before.");
  }

  callback_id_list_.clear();

  if (watch_id_ != watch_id) {
    LoggerE("Invalid watch id: %d", watch_id);
    throw InvalidValuesException("Watch id is invalid.");
  }

  pkgmgr_client_free(manager_handle_);
  manager_handle_ = NULL;
}



int ApplicationInstance::GetAppInstalledSize(const std::string &app_id) {
  LoggerD("Get app size: %s", app_id.c_str());
  char* package_id = NULL;
  int size = 0;
  int ret = 0;

  ret = package_manager_get_package_id_by_app_id(app_id.c_str(), &package_id);
  if ((ret != PACKAGE_MANAGER_ERROR_NONE) || (package_id == NULL)) {
    LoggerE("Failed to get package id(%s)", package_id);
  } else {
    // get installed size from package server (to solve smack issue)
    pkgmgr_client *pc = pkgmgr_client_new(PC_REQUEST);
    if (pc == NULL) {
      LoggerE("Failed to create pkgmgr client");
    } else {
      size = pkgmgr_client_request_service(PM_REQUEST_GET_SIZE, PM_GET_TOTAL_SIZE, pc, NULL, package_id, NULL, NULL, NULL);
      if (size < 0) {
        LoggerE("Failed to get installed size");
      }
      pkgmgr_client_free(pc);
      pc = NULL;
    }

    if (package_id) {
      free(package_id);
    }
  }
  return size; 
}

int ApplicationInstance::get_installed_size() {
  return installed_size_;
}

void ApplicationInstance::set_installed_size(const int &installed_size) {
  LoggerD("app size: installed_size: %d", installed_size);
  installed_size_ = installed_size;
}

int ApplicationInstance::get_watch_id_and_increase() {
  return ++watch_id_;
}
#undef CHECK_EXIST

} // namespace application
} // namespace extension
