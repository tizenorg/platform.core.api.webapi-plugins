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
#include "application_control.h"

namespace extension {
namespace application {

namespace {
// The privileges that required in Application API
const std::string kPrivilegeApplication = "";

} // namespace

using namespace common;
using namespace extension::application;

static ApplicationInformationPtr get_app_info(pkgmgrinfo_appinfo_h handle);

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

struct CallbackInfo {
  ApplicationInstance* instance;
  bool is_success;
  int error_type;
  char error_msg[256];
  char id[256];
  int callback_id;
  picojson::object data; 
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

static void ReplyAsync(ApplicationInstance* instance,
                       int callback_id, bool isSuccess, picojson::object& param, int err_id, const char* err_msg) {
  param["callbackId"] = picojson::value(static_cast<double>(callback_id));
  if (isSuccess) {
    param["status"] = picojson::value("success");
  } else {
    param.insert(std::make_pair("status", picojson::value("error")));
    param.insert(std::make_pair("error", GetAppError(err_id, err_msg)));
  }
  
  picojson::value result = picojson::value(param);
  char print_buf[300] = {0};
  snprintf(print_buf, sizeof(print_buf), result.serialize().c_str());
  LoggerD("async result: %s", print_buf);
  instance->PostMessage(result.serialize().c_str());
}

// Callback of 'app_manager_foreach_app_context'
// Used by 'getAppsContext'
static bool app_manager_app_context_callback(app_context_h app_context, void *user_data) {
  int ret = 0;

  char *app_id = NULL;
  int pid;

  std::string context_id;

  if (user_data == NULL) {
    LoggerD("user_data is NULL");
    return false;
  }

  ret = app_context_get_app_id(app_context, &app_id);
  if ((ret != APP_MANAGER_ERROR_NONE) || (app_id == NULL)) {
    LoggerE("Fail to get app id from context");
    return false;
  }
  LoggerD("app_context_get_app_id: %s", app_id);
  ret = app_context_get_pid(app_context, &pid);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Fail to get pid from context");
    if (app_id)
      free(app_id);
    return false;
  }
  LoggerD("app_context_get_pid: %d", pid);

  std::stringstream sstream;
  sstream << pid;
  context_id = sstream.str();

  ApplicationContextPtr app_context_ptr(new ApplicationContext());
  app_context_ptr->set_app_id(app_id);
  app_context_ptr->set_context_id(context_id);

  ApplicationContextArray* app_context_array_ptr = (ApplicationContextArray*)user_data;
  app_context_array_ptr->push_back(app_context_ptr);

  if (app_id)
    free(app_id);

  return true;
}

static int get_app_installed_size(const char* app_id) {
  char* package_id = NULL;
  int size = 0;
  int ret = 0;

  ret = package_manager_get_package_id_by_app_id(app_id, &package_id);
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

  LoggerD("Get app size: %s[%d]", app_id, size);
  return size; 
}

// Callback from 'app_control_foreach_app_matched'
// Used by 'findAppControl'
static bool app_control_app_matched_callback(app_control_h service, const char *appid, void *user_data) {
  if (appid == NULL) {
    LoggerD("appid is NULL");
    return false;
  }

  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(appid, &handle);
  if (ret != PMINFO_R_OK) {
    LoggerD("Fail to get appInfo from appId : %s", appid);
  } else {
    LoggerD("Getting app info: %s", appid);
    ApplicationInformationPtr app_info_ptr = get_app_info(handle);
    pkgmgrinfo_appinfo_destroy_appinfo(handle);

    ApplicationInformationArray* app_info_array = (ApplicationInformationArray*)user_data;
    app_info_array->push_back(app_info_ptr);
  }

  return true;
}

static bool app_control_extra_data_callback(app_control_h service, const char* key, void* user_data) {
  int ret = 0;
  LoggerD("Handing extra data");  

  ApplicationControlDataArray* app_ctr_data_array = (ApplicationControlDataArray*)user_data;

  bool is_array = false;
  ret = app_control_is_extra_data_array(service, key, &is_array);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("service_is_extra_data_array passes error");
    // fail to checking. go to next extra data.
    return true;
  }

  std::string key_str(key);

  if (is_array) {
    LoggerD("extra data is array");
    int length = 0;
    char **value = NULL;

    ret = app_control_get_extra_data_array(service, key, &value, &length);
    switch (ret) {
      case APP_CONTROL_ERROR_NONE: {
        std::vector<std::string> val_array;
        LoggerD("key: %s, value length : %d", key, length);
        for (int i = 0; i < length; i++) {
          if (value[i]) {
            LoggerD("[%d] value: %s", i, value[i]);
            val_array.push_back(value[i]);
          }
        }

        ApplicationControlDataPtr app_control_data(new ApplicationControlData());
        app_control_data->set_ctr_key(key_str);
        app_control_data->set_ctr_value(val_array);
        app_ctr_data_array->push_back(app_control_data);

        for (int i = 0; i < length; i++) {
          if (value[i])
            free(value[i]);
        }
        if (value)
          free(value);
        break;
      }
      case APP_CONTROL_ERROR_INVALID_PARAMETER:
        LoggerE("app_control_get_extra_data retuns APP_CONTROL_ERROR_INVALID_PARAMETER");
        break;
      case APP_CONTROL_ERROR_KEY_NOT_FOUND:
        LoggerE("app_control_get_extra_data retuns APP_CONTROL_ERROR_KEY_NOT_FOUND");
        break;
      case APP_CONTROL_ERROR_OUT_OF_MEMORY:
        LoggerE("app_control_get_extra_data retuns APP_CONTROL_ERROR_OUT_OF_MEMORY");
        break;
      default:
        LoggerE("app_control_get_extra_data retuns Error");
        break;
    }
  } else { // (!is_array)
    LoggerD("extra data is not array");
    char *value = NULL;

    ret = app_control_get_extra_data(service, key, &value);
    switch (ret) {
    case APP_CONTROL_ERROR_NONE: {
      if (value == NULL) {
        LoggerE("app_control_get_extra_data returns NULL");
        break;
      }

      std::vector<std::string> val_array;
      val_array.push_back(value);

      ApplicationControlDataPtr app_control_data(new ApplicationControlData());
      LoggerD("key: %s, value: %s", key, value);
      app_control_data->set_ctr_key(key_str);
      app_control_data->set_ctr_value(val_array);
      app_ctr_data_array->push_back(app_control_data);

      if (value)
        free(value);

      break;
    }
    case APP_CONTROL_ERROR_INVALID_PARAMETER:
      LoggerE("app_control_get_extra_data retuns APP_CONTROL_ERROR_INVALID_PARAMETER");
      break;
    case APP_CONTROL_ERROR_KEY_NOT_FOUND:
      LoggerE("app_control_get_extra_data retuns APP_CONTROL_ERROR_KEY_NOT_FOUND");
      break;
    case APP_CONTROL_ERROR_OUT_OF_MEMORY:
      LoggerE("app_control_get_extra_data retuns APP_CONTROL_ERROR_OUT_OF_MEMORY");
      break;
    default:
      LoggerE("app_control_get_extra_data retuns Error");
      break;
    }
  }
  return true;
}

// Callback of 'app_control_send_launch_request'
// Used by 'launchAppControl'
static void app_control_reply_callback(app_control_h request, app_control_h reply,
        app_control_result_e result, void *user_data) {

  CallbackInfo* info = (CallbackInfo*)user_data;

  if (result == APP_CONTROL_RESULT_SUCCEEDED) {

    LoggerD("APP_CONTROL_RESULT_SUCCEEDED");

    // create new service object to store result.
    ApplicationControlDataArrayPtr app_ctr_data_array(new ApplicationControlDataArray());

    int result = app_control_foreach_extra_data(reply,
                                  app_control_extra_data_callback, app_ctr_data_array.get());
    if (result == APP_CONTROL_ERROR_NONE) {
      LoggerD("Getting extra data is succeeded."); 
    } else {
      LoggerD("Getting extra data is failed.");
    }
    
    picojson::value replied_data = picojson::value(picojson::array());
    picojson::array& replied_data_array = replied_data.get<picojson::array>();

    for (int i = 0; i < app_ctr_data_array->size(); i++) {
      ApplicationControlDataPtr ctr_data_ptr = app_ctr_data_array->at(i);

      replied_data_array.push_back(ctr_data_ptr->Value());
    }

    // ReplyAsync
    picojson::object data;
    data.insert(std::make_pair("type", picojson::value("onsuccess")));
    data.insert(std::make_pair("data", replied_data));
    ReplyAsync(info->instance, info->callback_id, true, data, 0, NULL);

  } else if (result == APP_CONTROL_RESULT_FAILED || APP_CONTROL_RESULT_CANCELED) {
    LoggerD("APP_CONTROL_RESULT_FAILED or CANCELED");

    // ReplyAsync
    picojson::object data;
    data.insert(std::make_pair("type", picojson::value("onfailure")));
    ReplyAsync(info->instance, info->callback_id, false, data, APP_ERROR_ABORT, "Failed or Canceled");
  }

  if (info)
    free(info);
}

// get package name by id
static char* getPackageByAppId(const char* app_id) {
  app_info_h handle;
  char* pkgName;
  int ret = 0;

  // TODO: gPkgIdMapInited

  ret = app_manager_get_app_info(app_id, &handle);
  if (ret < 0) {
    LoggerE("Fail to get appinfo");
    return NULL;
  }

  ret = app_info_get_package(handle, &pkgName);
  if (ret < 0) {
    LoggerE("Fail to get pkgName");
    pkgName = NULL;
  }
  ret = app_info_destroy(handle);
  if (ret < 0) {
    LoggerE("Fail to get destory appinfo");
    return NULL;
  }
  return pkgName;
}

static int category_cb(const char *category, void *user_data) {
  LoggerD("category: %s", category);
  if (category == NULL)
    return true;

  ApplicationInformation* appInfo = (ApplicationInformation*)user_data;
  appInfo->add_categories(category);
  return true;
}

static bool package_certificate_cb(package_info_h handle, package_cert_type_e cert_type, const char *cert_value, void *user_data) {
  ApplicationCertificatePtr cert(new ApplicationCertificate());
  std::string cert_type_name;

  switch(cert_type) {
    case PACKAGE_INFO_AUTHOR_ROOT_CERT:
      cert_type_name = "AUTHOR_ROOT";
      break;
    case PACKAGE_INFO_AUTHOR_INTERMEDIATE_CERT:
      cert_type_name = "AUTHOR_INTERMEDIATE";
      break;
    case PACKAGE_INFO_AUTHOR_SIGNER_CERT:
      cert_type_name = "AUTHOR_SIGNER";
      break;
    case PACKAGE_INFO_DISTRIBUTOR_ROOT_CERT:
      cert_type_name = "DISTRIBUTOR_ROOT";
      break;
    case PACKAGE_INFO_DISTRIBUTOR_INTERMEDIATE_CERT:
      cert_type_name = "DISTRIBUTOR_INTERMEDIATE";
      break;
    case PACKAGE_INFO_DISTRIBUTOR_SIGNER_CERT:
      cert_type_name = "DISTRIBUTOR_SIGNER";
      break;
    case PACKAGE_INFO_DISTRIBUTOR2_ROOT_CERT:
      cert_type_name = "DISTRIBUTOR2_ROOT";
      break;
    case PACKAGE_INFO_DISTRIBUTOR2_INTERMEDIATE_CERT:
      cert_type_name = "DISTRIBUTOR2_INTERMEDIATE";
      break;
    case PACKAGE_INFO_DISTRIBUTOR2_SIGNER_CERT:
      cert_type_name = "DISTRIBUTOR2_SIGNER";
      break;
    default:
      LoggerE("Unknow Cert type!!!");
      cert_type_name = "UNKNOWN";
      break;
  }

  cert->set_cert_type(cert_type_name);
  std::string cert_type_value = cert_value;
  cert->set_cert_value(cert_type_value);

  ApplicationCertificateArray *certs = (ApplicationCertificateArray*)user_data;
  certs->push_back(cert);
  return true;
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
  app_info->set_size(get_app_installed_size(app_id));
  return app_info;
}

static int app_meta_data_cb(const char *meta_key, const char *meta_value, void *user_data) {
  if ((meta_key == NULL)  || (meta_value == NULL)) {
    LoggerE("meta_key or meta_value is null");
    return 0;
  }

  LoggerD("key = %s, value = %s", meta_key, meta_value);
  ApplicationMetaDataPtr meta_data(new ApplicationMetaData());

  std::string key = meta_key;
  meta_data->set_meta_key(key);
  std::string value = meta_value;
  meta_data->set_meta_value(value);

  ApplicationMetaDataArray* meta_data_array = (ApplicationMetaDataArray*)user_data;
  meta_data_array->push_back(meta_data);

  return 0;
}

static int installed_app_info_cb(pkgmgrinfo_appinfo_h handle, void *user_data) {
  LoggerD("ENTER");
  ApplicationInformationPtr app_info = get_app_info(handle);
  ApplicationInformationArray *app_info_array = (ApplicationInformationArray*)user_data;
  app_info_array->push_back(app_info);
  return 0;
}

// Callback from 'app_manager_set_app_context_event_cb'
// Used by 'kill'
static void app_manager_app_context_event_callback(app_context_h app_context,
  app_context_event_e event, void *user_data) {

  CallbackInfo* info = (CallbackInfo*)user_data;
  int ret = 0;

  LoggerD("context_id: %s, callback_id: %d", info->id, info->callback_id);
  if(event != APP_CONTEXT_EVENT_TERMINATED) {
    picojson::object data;
    info->error_type = APP_ERROR_UNKNOWN;
    sprintf(info->error_msg, "Not terminated.");
    ReplyAsync(info->instance, info->callback_id, 
               false, data, info->error_type, info->error_msg);
  } else {
    picojson::object data;
    ReplyAsync(info->instance, info->callback_id,
               true, data, 0, NULL);
  }

  if (info)
    free(info);
}

static gboolean getappsinfo_callback_thread_completed(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered");
  
  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, user_data->data, 0, NULL);
  } else {
    picojson::object data;
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, data, user_data->error_type, user_data->error_msg);
  }
    return true;
}

static void* getappsinfo_callback_thread(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered.");
  ApplicationInformationArrayPtr app_info_array_ptr(new ApplicationInformationArray());

  int ret = pkgmgrinfo_appinfo_get_installed_list(installed_app_info_cb, app_info_array_ptr.get());
  if (ret == PMINFO_R_OK) {
    LoggerE("pkgmgrinfo_appinfo_get_installed_list: ERROR_NONE");
    user_data->is_success = true;

    picojson::value apps_infos = picojson::value(picojson::array());
    picojson::array& apps_infos_array = apps_infos.get<picojson::array>();

    for (int i = 0; i < app_info_array_ptr->size(); i++) {
      ApplicationInformationPtr app_info_ptr = app_info_array_ptr->at(i);

      apps_infos_array.push_back(app_info_ptr->Value());
    }
    user_data->data.insert(std::make_pair("informationArray", apps_infos)); 

  } else {
    LoggerE("pkgmgrinfo_appinfo_get_installed_list: ERROR");

    user_data->error_type = APP_ERROR_UNKNOWN;
    sprintf(user_data->error_msg, "Unknown");

    user_data->is_success = false;
  } 
}

static gboolean getappsctx_callback_thread_completed(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered");
  
  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, user_data->data, 0, NULL);
  } else {
    picojson::object data;
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, data, user_data->error_type, user_data->error_msg);
  }
    return true;
}

static void* getappsctx_callback_thread(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered.");
  ApplicationContextArrayPtr app_context_array_ptr(new ApplicationContextArray());

  int ret = app_manager_foreach_app_context(app_manager_app_context_callback, app_context_array_ptr.get());
  if (ret == APP_MANAGER_ERROR_NONE) {
    LoggerE("app_manager_foreach_app_context error: ERROR_NONE");
    user_data->is_success = true;

    picojson::value apps_contexts = picojson::value(picojson::array());
    picojson::array& apps_contexts_array = apps_contexts.get<picojson::array>();

    for (int i = 0; i < app_context_array_ptr->size(); i++) {
      ApplicationContextPtr app_ctx_ptr = app_context_array_ptr->at(i);

      apps_contexts_array.push_back(app_ctx_ptr->Value());
    }
    user_data->data.insert(std::make_pair("contexts", apps_contexts)); 

  } else {
    LoggerE("app_manager_foreach_app_context error: ERROR");

    if (ret == APP_MANAGER_ERROR_INVALID_PARAMETER) {
      user_data->error_type = APP_ERROR_TYPE_MISMATCH;
      sprintf(user_data->error_msg, "Invalid parameter");
    } else if(ret == APP_MANAGER_ERROR_PERMISSION_DENIED) {
      user_data->error_type = APP_ERROR_ABORT;
      sprintf(user_data->error_msg, "Permission denied");
    } else {
      user_data->error_type = APP_ERROR_UNKNOWN;
      sprintf(user_data->error_msg, "Unknown");
    }

    user_data->is_success = false;
  } 
}

static gboolean callback_thread_completed(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered");
  picojson::object data;
  
  if (user_data->is_success) {

    ReplyAsync(user_data->instance, user_data->callback_id,
               true, data, 0, NULL);
  } else {
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, data, user_data->error_type, user_data->error_msg);
  }
    return true;
}

static gboolean find_callback_thread_completed(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered");
  picojson::object data;
  
  if (user_data->is_success) {

    ReplyAsync(user_data->instance, user_data->callback_id,
               true, user_data->data, 0, NULL);
  } else {
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, data, user_data->error_type, user_data->error_msg);
  }
    return true;
}

static void* callback_thread(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered. currently, nothing to do");
 
}

static gboolean launch_completed(const std::shared_ptr<CallbackInfo>& user_data)
{
  LoggerD("Entered");
  picojson::object data;
   
  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, data, 0, NULL);
  } else {
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, data, user_data->error_type, user_data->error_msg);
  }
    return true;
}

static void* launch_thread(const std::shared_ptr<CallbackInfo>& user_data)
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

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& context_id = args.get("contextId").get<std::string>();

  LoggerD("callbackId = %d", callback_id);
  LoggerD("contextId = %s", context_id.c_str());

  try {
    Kill(context_id, callback_id);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerLaunch(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& id = args.get("id").get<std::string>();

  LoggerD("callbackId = %d", callback_id);
  LoggerD("appId = %s", id.c_str());

  try {
    Launch(id, callback_id);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerLaunchappcontrol(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  std::string id; // app id is optional
  if (args.contains("id"))
    id = args.get("id").get<std::string>();

  LoggerD("app_id = %s", id.c_str());

  LoggerD("callbackId = %d", callback_id);

  //LoggerD("args = %s", args.serialize().c_str());
  /* example: args.serialize().c_str()
    args = 
{
  "appControl":
  {
    "category":null,
    "data":[
            {
              "key":"images1",
              "value":["first1","second1"]
            },
            {
              "key":"images2",
              "value":["first2","second2"]
            }
           ],
    "mime":"image\/*",
    "operation":"http:\/\/tizen.org\/appcontrol\/operation\/pick",
    "uri":null
  },
  "callbackId":0
}
  */

  ApplicationControlPtr app_ctr_ptr(new ApplicationControl());

  picojson::value app_control = args.get("appControl");
  std::string operation = app_control.get("operation").get<std::string>();
  app_ctr_ptr->set_operation(operation);
  LoggerD("operation: %s", operation.c_str());
  
  if (app_control.contains("uri")) {
    if (app_control.get("uri").is<picojson::null>() == false) {
      std::string uri = app_control.get("uri").get<std::string>();
      app_ctr_ptr->set_uri(uri);  
    } else {
      LoggerD("uri is null");
    }
  }

  if (app_control.contains("mime")) {
    if (app_control.get("mime").is<picojson::null>() == false) {
      std::string mime = app_control.get("mime").get<std::string>();
      app_ctr_ptr->set_mime(mime);  
    } else {
      LoggerD("mime is null");
    }
  }

  if (app_control.contains("category")) {
    if (app_control.get("category").is<picojson::null>() == false) {
      std::string category = app_control.get("category").get<std::string>();
      app_ctr_ptr->set_category(category);  
    } else {
      LoggerD("category is null");
    }
  }

  std::vector<picojson::value> data_array = app_control.get("data").get<picojson::array>();
  for (int i = 0; i < data_array.size(); i++) {
    ApplicationControlDataPtr ctr_data_ptr(new ApplicationControlData());

    picojson::value each_data = data_array.at(i);
    std::string key = each_data.get("key").get<std::string>();
    LoggerD("%d: key = %s", i, key.c_str());   

    ctr_data_ptr->set_ctr_key(key);
 
    std::vector<picojson::value> values = each_data.get("value").get<picojson::array>();
    for (int j = 0; j < values.size(); j++) {
      std::string val = values.at(i).to_str();

      ctr_data_ptr->add_ctr_value(val);
      LoggerD("-%d val = %s", j, val.c_str());
    }

    app_ctr_ptr->add_data_array(ctr_data_ptr);
  }
  
  try {
    LaunchAppControl(app_ctr_ptr, id, callback_id);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerFindappcontrol(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  LoggerD("callbackId = %d", callback_id);

  ApplicationControlPtr app_ctr_ptr(new ApplicationControl());

  picojson::value app_control = args.get("appControl");
  std::string operation = app_control.get("operation").get<std::string>();
  app_ctr_ptr->set_operation(operation);
  LoggerD("operation: %s", operation.c_str());
  
  if (app_control.contains("uri")) {
    if (app_control.get("uri").is<picojson::null>() == false) {
      std::string uri = app_control.get("uri").get<std::string>();
      app_ctr_ptr->set_uri(uri);  
    } else {
      LoggerD("uri is null");
    }
  }

  if (app_control.contains("mime")) {
    if (app_control.get("mime").is<picojson::null>() == false) {
      std::string mime = app_control.get("mime").get<std::string>();
      app_ctr_ptr->set_mime(mime);  
    } else {
      LoggerD("mime is null");
    }
  }

  if (app_control.contains("category")) {
    if (app_control.get("category").is<picojson::null>() == false) {
      std::string category = app_control.get("category").get<std::string>();
      app_ctr_ptr->set_category(category);  
    } else {
      LoggerD("category is null");
    }
  }

  std::vector<picojson::value> data_array = app_control.get("data").get<picojson::array>();
  for (int i = 0; i < data_array.size(); i++) {
    ApplicationControlDataPtr ctr_data_ptr(new ApplicationControlData());

    picojson::value each_data = data_array.at(i);
    std::string key = each_data.get("key").get<std::string>();
    LoggerD("%d: key = %s", i, key.c_str());   

    ctr_data_ptr->set_ctr_key(key);
 
    std::vector<picojson::value> values = each_data.get("value").get<picojson::array>();
    for (int j = 0; j < values.size(); j++) {
      std::string val = values.at(i).to_str();

      ctr_data_ptr->add_ctr_value(val);
      LoggerD("-%d val = %s", j, val.c_str());
    }

    app_ctr_ptr->add_data_array(ctr_data_ptr);
  }
  
  try {
    FindAppControl(app_ctr_ptr, callback_id);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerGetappscontext(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  try {
    GetAppsContext(callback_id);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerGetappcontext(const picojson::value& args, picojson::object& out) {

  LoggerD("ENTER");
  std::string context_id;
  if (args.contains("contextId")) {
    LoggerD("ENTER2");
    context_id = args.get("contextId").get<std::string>();
    LoggerD("contextId = %s", context_id.c_str());
  } else {
    LoggerD("contextId is null");
  }

  try {
    ApplicationContextPtr app_ctx = GetAppContext(context_id);

    LoggerD("appCtx: id = %s", app_ctx->get_context_id().c_str());
    LoggerD("appCtx: appId = %s", app_ctx->get_app_id().c_str());

    ReportSuccess(picojson::value(app_ctx->Value()), out);
  } catch (const PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerGetappsinfo(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  try {
    GetAppsInfo(callback_id);
    ReportSuccess(out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
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
    ApplicationCertificateArrayPtr cert_data_array_ptr = GetAppCertificate(id);
    picojson::value cert_data = picojson::value(picojson::array());
    picojson::array& cert_data_array = cert_data.get<picojson::array>();

    for (int i = 0; i < cert_data_array_ptr->size(); i++) {
      ApplicationCertificatePtr cert_data_ptr = cert_data_array_ptr->at(i);

      cert_data_array.push_back(cert_data_ptr->Value());
    }
    ReportSuccess(cert_data, out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
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
    ApplicationMetaDataArrayPtr meta_data_array_ptr = GetAppMetaData(id);
    picojson::value meta_data = picojson::value(picojson::array());
    picojson::array& meta_data_array = meta_data.get<picojson::array>();

    for (int i = 0; i < meta_data_array_ptr->size(); i++) {
      ApplicationMetaDataPtr meta_data_ptr = meta_data_array_ptr->at(i);

      meta_data_array.push_back(meta_data_ptr->Value());
    }
    ReportSuccess(meta_data, out);
  } catch (const common::PlatformException& err) {
    ReportError(err, out);
  } catch (...) {
    ReportError(out);
  }
}

void ApplicationInstance::ApplicationManagerAddappinfoeventlistener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  try {
    const double ret = static_cast<double>(AddAppInfoEventListener(callback_id));
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
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  Application *app = new Application();
  ApplicationPtr app_ptr = ApplicationPtr(app);
  app_ptr->set_app_info(app_info_ptr);

  LoggerD("set appinfo to application");
  {
    int pid = getpid(); // DO NOT USE getppid();
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
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  return app_info_ptr;
}

void ApplicationInstance::Kill(const std::string context_id, int callback_id) {
  if (context_id.empty()) {
    LoggerE("contextId is mandatory field.");
    throw InvalidValuesException("Context id is mandatory field.");
  }

  int ret;
  int pid;
  std::stringstream(context_id) >> pid;

  if (pid <= 0) {
    LoggerE("Given context id is wrong.");
    throw InvalidValuesException("Given context id is wrong.");
  }

  // if kill request is come for current context, throw InvalidValueException by spec
  if (pid == getpid()) {
    LoggerE("Given context id is same with me.");
    throw InvalidValuesException("Given context id is same with me.");
  }

  char *app_id_cstr = NULL;
  ret = app_manager_get_app_id(pid, &app_id_cstr);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Error while getting app id");
    throw NotFoundException("Error while getting app id");
  }

  std::string app_id = app_id_cstr;
  free(app_id_cstr);

  app_context_h appContext;
  ret = app_manager_get_app_context (app_id.c_str(), &appContext);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Error while getting app context");
    throw NotFoundException("Error while getting app context");
  }
  
  CallbackInfo* info = new CallbackInfo;
  info->instance = this;
  sprintf(info->id, "%s", context_id.c_str()); 
  info->callback_id = callback_id;

  // TODO thread
  ret = app_manager_set_app_context_event_cb(app_manager_app_context_event_callback, (void*)info);
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

void ApplicationInstance::Launch(const std::string app_id, int callback_id) {
  if (app_id.empty()) {
    LoggerE("app_id is mandatory field.");
    throw InvalidValuesException("App id is mandatory field.");
  }

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  sprintf(user_data->id, "%s", app_id.c_str());
  user_data->callback_id = callback_id;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(launch_thread, launch_completed, user_data);
}

void ApplicationInstance::LaunchAppControl(const ApplicationControlPtr& app_ctr_ptr, const std::string& app_id, const int& callback_id) {
  std::string operation = app_ctr_ptr->get_operation();
  if (operation.empty()) {
    LoggerE("operation is mandatory field.");
    throw InvalidValuesException("operation is mandatory field.");
  }

  app_control_h service;
  int ret = app_control_create(&service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    throw UnknownException("Creating app_control is failed.");
  }

  if (app_id.empty() == false) {
    ret = app_control_set_app_id(service, app_id_.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw UnknownException("Setting app_id is failed.");
    }
  } else {
    LoggerD("app_id is empty");
  }

  ret = app_control_set_operation(service, operation.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    throw InvalidValuesException("operation is invalid parameter");
  }
  
  std::string uri = app_ctr_ptr->get_uri();
  if (!uri.empty()) {
    ret = app_control_set_uri(service, uri.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw InvalidValuesException("uri is invalid parameter");
    }
  }

  std::string mime = app_ctr_ptr->get_mime();
  if (!mime.empty()) {
    ret = app_control_set_mime(service, mime.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw InvalidValuesException("mime is invalid parameter");
    }
  }

  std::string category = app_ctr_ptr->get_category();
  if (!category.empty()) {
    ret = app_control_set_category(service, category.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw InvalidValuesException("category is invalid parameter");
    }
  }

  CallbackInfo* info = new CallbackInfo;
  info->instance = this;
  info->callback_id = callback_id;

  LoggerD("Try to launch...");
  ret = app_control_send_launch_request(service, app_control_reply_callback, (void*)info);
 
  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  if(ret != APP_CONTROL_ERROR_NONE) {
    switch (ret) {
      case APP_CONTROL_ERROR_INVALID_PARAMETER:
        LoggerD("launch_request is failed. APP_CONTROL_ERROR_INVALID_PARAMETER");
        user_data->error_type = APP_ERROR_TYPE_MISMATCH;
        sprintf(user_data->error_msg, "launch_request is failed. INVALID_PARAMETER");
        break;
      case APP_CONTROL_ERROR_OUT_OF_MEMORY:
        LoggerD("launch_request is failed. APP_CONTROL_ERROR_OUT_OF_MEMORY");
        user_data->error_type = APP_ERROR_UNKNOWN;
        sprintf(user_data->error_msg, "launch_request is failed. OUT_OF_MEMORY");
        break;
      case APP_CONTROL_ERROR_LAUNCH_REJECTED:
        LoggerD("launch_request is failed. APP_CONTROL_ERROR_LAUNCH_REJECTED");
        user_data->error_type = APP_ERROR_ABORT;
        sprintf(user_data->error_msg, "launch_request is failed. LAUNCH_REJECTED");
        break;
      case APP_CONTROL_ERROR_APP_NOT_FOUND:
        LoggerD("launch_request is failed. APP_CONTROL_ERROR_APP_NOT_FOUND");
        user_data->error_type = APP_ERROR_NOT_FOUND;
        sprintf(user_data->error_msg, "launch_request is failed. NOT_FOUND");
        break;
      default:
        LoggerD("launch_request is failed.");
        user_data->error_type = APP_ERROR_UNKNOWN;
        sprintf(user_data->error_msg, "launch_request is failed. UNKNOWN");
        break;
    }
    user_data->is_success = false;
  } else {
    user_data->is_success = true;
  }
  common::TaskQueue::GetInstance().Queue<CallbackInfo>(callback_thread, callback_thread_completed, user_data);

  ret = app_control_destroy(service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerD("app_control_destroy is failed");
  }
}

void ApplicationInstance::FindAppControl(const ApplicationControlPtr& app_ctr_ptr, const int& callback_id) {
  std::string operation = app_ctr_ptr->get_operation();
  if (operation.empty()) {
    LoggerE("operation is mandatory field.");
    throw InvalidValuesException("operation is mandatory field.");
  }

  app_control_h service;
  int ret = app_control_create(&service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    throw UnknownException("Creating app_control is failed.");
  }

  ret = app_control_set_operation(service, operation.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    throw InvalidValuesException("operation is invalid parameter");
  }
  
  std::string uri = app_ctr_ptr->get_uri();
  if (!uri.empty()) {
    ret = app_control_set_uri(service, uri.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw InvalidValuesException("uri is invalid parameter");
    }
  }

  std::string mime = app_ctr_ptr->get_mime();
  if (!mime.empty()) {
    ret = app_control_set_mime(service, mime.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw InvalidValuesException("mime is invalid parameter");
    }
  }

  std::string category = app_ctr_ptr->get_category();
  if (!category.empty()) {
    ret = app_control_set_category(service, category.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      throw InvalidValuesException("category is invalid parameter");
    }
  }
 
  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;
 
  ApplicationInformationArrayPtr app_info_array_ptr(new ApplicationInformationArray());

  LoggerD("Try to find...");
  ret = app_control_foreach_app_matched(service, app_control_app_matched_callback, (void*)app_info_array_ptr.get());
  if (ret == APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_foreach_app_matched: ERROR_NONE");
    user_data->is_success = true;

    picojson::value app_infos = picojson::value(picojson::array());
    picojson::array& app_infos_array = app_infos.get<picojson::array>();

    for (int i = 0; i < app_info_array_ptr->size(); i++) {
      ApplicationInformationPtr app_info_ptr = app_info_array_ptr->at(i);
      app_infos_array.push_back(app_info_ptr->Value());
    }

    // ReplyAsync
    user_data->data.insert(std::make_pair("informationArray", app_infos));
    user_data->data.insert(std::make_pair("appControl", app_ctr_ptr->Value())); 

 } else if (ret == APP_CONTROL_ERROR_INVALID_PARAMETER) {
    LoggerD("launch_request is failed. APP_CONTROL_ERROR_INVALID_PARAMETER");
    user_data->error_type = APP_ERROR_TYPE_MISMATCH;
    sprintf(user_data->error_msg, "launch_request is failed. INVALID_PARAMETER");
    user_data->is_success = false;
  } else {
    LoggerD("launch_request is failed. UNKNOWN");
    user_data->error_type = APP_ERROR_UNKNOWN;
    sprintf(user_data->error_msg, "launch_request is failed. OUT_OF_MEMORY");
    user_data->is_success = false;
  }
 
  common::TaskQueue::GetInstance().Queue<CallbackInfo>(callback_thread, find_callback_thread_completed, user_data);

  ret = app_control_destroy(service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerD("app_control_destroy is failed");
  }
}

void ApplicationInstance::GetAppsContext(const int& callback_id) {

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(getappsctx_callback_thread, getappsctx_callback_thread_completed, user_data);

}

void ApplicationInstance::GetAppsInfo(const int& callback_id) {

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(getappsinfo_callback_thread, getappsinfo_callback_thread_completed, user_data);

}

ApplicationContextPtr ApplicationInstance::GetAppContext(const std::string context_id) {
  int ret = 0;

  LoggerD("contextId: %s", context_id.c_str());

  std::string cur_ctx_id = context_id;
  int pid;

  if (cur_ctx_id.empty()) {
    pid = getpid();
    std::stringstream sstr;
    sstr << pid;
    cur_ctx_id = sstr.str();
  }
  else {
    std::stringstream(context_id) >> pid;
    if (pid <= 0) {
      LoggerE("Given context_id is wrong");
      throw NotFoundException("Given context_id is wrong");
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
  
  ApplicationContextPtr app_context(new ApplicationContext());
  app_context->set_app_id(app_id);
  app_context->set_context_id(cur_ctx_id);

  if(app_id)
    free(app_id);

  return app_context;
}

ApplicationCertificateArrayPtr ApplicationInstance::GetAppCertificate(const std::string app_id) {

  int ret = 0;
  char* package = getPackageByAppId(app_id.c_str());
  if (package == NULL) {
    LoggerE("Can not get package");
    throw NotFoundException("Can not get package");
  }

  // TODO: gPkgIdMapInited
  package_info_h pkg_info;
  int result = 0;
  result = package_info_create(package, &pkg_info);
  if (result != PACKAGE_MANAGER_ERROR_NONE) {
    throw UnknownException("Can not get package info");
  }

  ApplicationCertificateArrayPtr cert_array(new ApplicationCertificateArray());

  result = package_info_foreach_cert_info(pkg_info, package_certificate_cb,
            (void*) cert_array.get());
  if ((result != PACKAGE_MANAGER_ERROR_NONE) &&
      (result != PACKAGE_MANAGER_ERROR_IO_ERROR)) {
    throw UnknownException("Can not get package cert info");
  }

  return cert_array;
}

ApplicationMetaDataArrayPtr ApplicationInstance::GetAppMetaData(const std::string id) {
  std::string app_id = id;

  int ret = 0;
  pkgmgrinfo_appinfo_h handle;
  ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    throw NotFoundException("Cannot found application with given app_id");
  }

  ApplicationMetaDataArrayPtr metaDataArray(new ApplicationMetaDataArray());
  ret = pkgmgrinfo_appinfo_foreach_metadata(handle, app_meta_data_cb, (void*) metaDataArray.get());
  if (ret != PMINFO_R_OK) {
    LoggerE("pkgmgrinfo_appinfo_metadata_filter_foreach() failed");
    pkgmgrinfo_appinfo_destroy_appinfo(handle);
    throw UnknownException("fail to get custom tag");
  }
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  return metaDataArray;
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
        ReplyAsync(this, callback_id, true, data, 0, NULL);
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
              ReplyAsync(this, callback_id, true, data, 0, NULL);
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
              ReplyAsync(this, callback_id, true, data, 0, NULL);
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

int ApplicationInstance::get_watch_id_and_increase() {
  return ++watch_id_;
}
#undef CHECK_EXIST

} // namespace application
} // namespace extension
