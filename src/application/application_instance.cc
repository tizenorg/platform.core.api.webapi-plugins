// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_instance.h"

// To get pid
#include <unistd.h>
#include <pthread.h>
#include <glib.h>

// to launch app by aul
#include <aul.h>

// to get package name by appid
#include <app_info.h>
#include <app_control_internal.h>

// To get app size and installed time
#include <pkgmgr-info.h>

//#include <plugins-ipc-message/ipc_message_support.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <sstream>
#include <utility>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include "application/application.h"
#include "application/application_context.h"
#include "application/application_control.h"

namespace extension {
namespace application {

namespace {
// The privileges that required in Application API
const std::string kPrivilegeApplication = "";

}  // namespace

using common::PlatformException;
using common::UnknownException;
using common::TypeMismatchException;
using common::IOException;
using common::ServiceNotAvailableException;
using common::SecurityException;
using common::NetworkException;
using common::NotSupportedException;
using common::NotFoundException;
using common::InvalidAccessException;
using common::AbortException;
using common::QuotaExceededException;
using common::InvalidStateException;
using common::InvalidModificationException;
using common::InvalidValuesException;
using common::TaskQueue;

static ApplicationInformationPtr get_app_info(pkgmgrinfo_appinfo_h handle);

ApplicationInstance::ApplicationInstance(const std::string& app_id) {
  manager_handle_ = NULL;
  watch_id_ = 0;
  app_id_ = app_id;

  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&ApplicationInstance::x, this, _1, _2));
  REGISTER_SYNC("ApplicationManager_getAppCerts",
    AppMgrGetAppCerts);
  REGISTER_SYNC("Application_getRequestedAppControl",
    AppGetRequestedAppControl);
  REGISTER_SYNC("ApplicationManager_addAppInfoEventListener",
    AppMgrAddAppInfoEventListener);
  REGISTER_SYNC("ApplicationManager_getAppMetaData",
    AppMgrGetAppMetaData);
  REGISTER_SYNC("ApplicationManager_launchAppControl",
    AppMgrLaunchAppControl);
  REGISTER_SYNC("ApplicationManager_removeAppInfoEventListener",
    AppMgrRemoveAppInfoEventListener);
  REGISTER_SYNC("ApplicationManager_getAppInfo",
    AppMgrGetAppInfo);
  REGISTER_SYNC("ApplicationManager_getAppSharedURI",
    AppMgrGetAppSharedURI);
  REGISTER_SYNC("RequestedApplicationControl_replyResult",
    RequestedAppControlReplyResult);
  REGISTER_SYNC("ApplicationManager_kill",
    AppMgrKill);
  REGISTER_SYNC("ApplicationManager_getAppsInfo",
    AppMgrGetAppsInfo);
  REGISTER_SYNC("ApplicationManager_launch",
    AppMgrLaunch);
  REGISTER_SYNC("Application_hide",
    AppHide);
  REGISTER_SYNC("ApplicationManager_getAppsContext",
    AppMgrGetAppsContext);
  REGISTER_SYNC("ApplicationManager_getAppContext",
    AppMgrGetAppContext);
  REGISTER_SYNC("RequestedApplicationControl_replyFailure",
    RequestedAppControlReplyFailure);
  REGISTER_SYNC("Application_exit",
    AppExit);
  REGISTER_SYNC("ApplicationManager_getCurrentApplication",
    AppMgrGetCurrentApplication);
  REGISTER_SYNC("ApplicationManager_findAppControl",
    AppMgrFindAppControl);
  #undef REGISTER_SYNC
}

ApplicationInstance::~ApplicationInstance() {
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
  APP_ERROR_INVALID_VALUES = 14,
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
  case APP_ERROR_INVALID_VALUES:
    return InvalidValuesException(msg).ToJSON();
  default:
    return UnknownException(msg).ToJSON();
  }
}

static void ReplyAsync(ApplicationInstance* instance,
  int callback_id, bool isSuccess, picojson::object* param,
  int err_id, const char* err_msg) {
  (*param)["callbackId"] = picojson::value(static_cast<double>(callback_id));
  if (isSuccess) {
    (*param)["status"] = picojson::value("success");
  } else {
    (*param).insert(std::make_pair("status", picojson::value("error")));
    (*param).insert(std::make_pair("error", GetAppError(err_id, err_msg)));
  }

  picojson::value result = picojson::value(*param);
  char print_buf[300] = {0};
  snprintf(print_buf, sizeof(print_buf), result.serialize().c_str());
  LoggerD("async result: %s", print_buf);
  instance->PostMessage(result.serialize().c_str());
}

// Callback of 'app_manager_foreach_app_context'
// Used by 'getAppsContext'
static bool app_manager_app_context_callback(app_context_h app_context,
  void *user_data) {
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

  ApplicationContextArray* app_context_array_ptr =
    reinterpret_cast<ApplicationContextArray*>(user_data);
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
      size = pkgmgr_client_request_service(PM_REQUEST_GET_SIZE,
        PM_GET_TOTAL_SIZE, pc, NULL, package_id, NULL, NULL, NULL);
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

  // LoggerD("Get app size: %s[%d]", app_id, size);
  return size;
}

// Callback from 'app_control_foreach_app_matched'
// Used by 'findAppControl'
static bool app_control_app_matched_callback(app_control_h service,
  const char *appid, void *user_data) {
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

    ApplicationInformationArray* app_info_array =
    reinterpret_cast<ApplicationInformationArray*>(user_data);

    app_info_array->push_back(app_info_ptr);
  }

  return true;
}

static bool app_control_extra_data_callback(app_control_h service,
  const char* key, void* user_data) {
  int ret = 0;
  LoggerD("Handing extra data");

  ApplicationControlDataArray* app_ctr_data_array =
    reinterpret_cast<ApplicationControlDataArray*>(user_data);

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

        ApplicationControlDataPtr app_control_data(
          new ApplicationControlData());
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
        LoggerE("get_extra_data retuns ERROR_INVALID_PARAMETER");
        break;
      case APP_CONTROL_ERROR_KEY_NOT_FOUND:
        LoggerE("get_extra_data retuns ERROR_KEY_NOT_FOUND");
        break;
      case APP_CONTROL_ERROR_OUT_OF_MEMORY:
        LoggerE("get_extra_data retuns ERROR_OUT_OF_MEMORY");
        break;
      default:
        LoggerE("get_extra_data retuns Unknown Error");
        break;
    }
  } else {  // (!is_array)
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
      LoggerE("get_extra_data retuns ERROR_INVALID_PARAMETER");
      break;
    case APP_CONTROL_ERROR_KEY_NOT_FOUND:
      LoggerE("get_extra_data retuns ERROR_KEY_NOT_FOUND");
      break;
    case APP_CONTROL_ERROR_OUT_OF_MEMORY:
      LoggerE("get_extra_data retuns ERROR_OUT_OF_MEMORY");
      break;
    default:
      LoggerE("get_extra_data retuns Known Error");
      break;
    }
  }
  return true;
}

// Callback of 'app_control_send_launch_request'
// Used by 'launchAppControl'
static void app_control_reply_callback(app_control_h request,
  app_control_h reply, app_control_result_e result, void *user_data) {
  CallbackInfo* info = reinterpret_cast<CallbackInfo*>(user_data);

  if (result == APP_CONTROL_RESULT_SUCCEEDED) {
    LoggerD("APP_CONTROL_RESULT_SUCCEEDED");

    // create new service object to store result.
    ApplicationControlDataArrayPtr app_ctr_data_array_ptr(
      new ApplicationControlDataArray());

    int result = app_control_foreach_extra_data(reply,
      app_control_extra_data_callback, app_ctr_data_array_ptr.get());
    if (result == APP_CONTROL_ERROR_NONE) {
      LoggerD("Getting extra data is succeeded.");
    } else {
      LoggerD("Getting extra data is failed.");
    }

    picojson::value replied_data = picojson::value(picojson::array());
    picojson::array& replied_data_array = replied_data.get<picojson::array>();

    for (int i = 0; i < app_ctr_data_array_ptr->size(); i++) {
      ApplicationControlDataPtr ctr_data_ptr = app_ctr_data_array_ptr->at(i);

      replied_data_array.push_back(ctr_data_ptr->Value());
    }

    // ReplyAsync
    picojson::object data;
    data.insert(std::make_pair("type", picojson::value("onsuccess")));
    data.insert(std::make_pair("data", replied_data));
    ReplyAsync(info->instance, info->callback_id, true, &data, 0, NULL);

  } else if (result == APP_CONTROL_RESULT_FAILED ||
             result == APP_CONTROL_RESULT_CANCELED) {
    LoggerD("APP_CONTROL_RESULT_FAILED or CANCELED");

    // ReplyAsync
    picojson::object data;
    data.insert(std::make_pair("type", picojson::value("onfailure")));
    ReplyAsync(info->instance, info->callback_id, false, &data,
      APP_ERROR_ABORT, "Failed or Canceled");
  }

  if (info)
    free(info);
}

// get package name by id
static char* getPackageByAppId(const char* app_id) {
  app_info_h handle;
  char* pkgName;
  int ret = 0;

  // TODO(sunggyu.choi): gPkgIdMapInited

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

  ApplicationInformation* appInfo =
    reinterpret_cast<ApplicationInformation*>(user_data);
  appInfo->add_categories(category);
  return true;
}

static bool package_certificate_cb(package_info_h handle,
  package_cert_type_e cert_type, const char *cert_value, void *user_data) {
  ApplicationCertificatePtr cert(new ApplicationCertificate());
  std::string cert_type_name;

  switch (cert_type) {
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

  ApplicationCertificateArray *certs =
    reinterpret_cast<ApplicationCertificateArray*>(user_data);

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

  ret = pkgmgrinfo_appinfo_foreach_category(handle, category_cb,
    reinterpret_cast<void*>(app_info.get()));
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
      // LoggerD("installed_time: %d", installed_time);
    }

    pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_h);
  }

  // size
  app_info->set_size(get_app_installed_size(app_id));
  return app_info;
}

static int app_meta_data_cb(const char *meta_key, const char *meta_value,
  void *user_data) {
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

  ApplicationMetaDataArray* meta_data_array =
    reinterpret_cast<ApplicationMetaDataArray*>(user_data);
  meta_data_array->push_back(meta_data);

  return 0;
}

static int installed_app_info_cb(pkgmgrinfo_appinfo_h handle,
  void *user_data) {
  // LoggerD("ENTER");
  ApplicationInformationPtr app_info = get_app_info(handle);
  ApplicationInformationArray* app_info_array =
    reinterpret_cast<ApplicationInformationArray*>(user_data);
  app_info_array->push_back(app_info);
  return 0;
}

// Callback from 'app_manager_set_app_context_event_cb'
// Used by 'kill'
static void app_manager_app_context_event_callback(app_context_h app_context,
  app_context_event_e event, void *user_data) {
  CallbackInfo* info =
    reinterpret_cast<CallbackInfo*>(user_data);
  int ret = 0;

  LoggerD("context_id: %s, callback_id: %d", info->id, info->callback_id);
  if (event != APP_CONTEXT_EVENT_TERMINATED) {
    picojson::object data;
    info->error_type = APP_ERROR_UNKNOWN;
    snprintf(info->error_msg, sizeof(info->error_msg), "Not terminated.");
    ReplyAsync(info->instance, info->callback_id,
               false, &data, info->error_type, info->error_msg);
  } else {
    picojson::object data;
    ReplyAsync(info->instance, info->callback_id,
               true, &data, 0, NULL);
  }

  if (info)
    free(info);
}

static gboolean getappsinfo_callback_thread_completed(
  const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered");

  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, &(user_data->data), 0, NULL);
  } else {
    picojson::object data;
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, &data, user_data->error_type, user_data->error_msg);
  }
    return true;
}

static void* getappsinfo_callback_thread(
  const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered.");
  ApplicationInformationArrayPtr app_info_array_ptr(
    new ApplicationInformationArray());

  int ret = pkgmgrinfo_appinfo_get_installed_list(installed_app_info_cb,
    app_info_array_ptr.get());
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
    snprintf(user_data->error_msg, sizeof(user_data->error_msg), "Unknown");

    user_data->is_success = false;
  }
}

static gboolean getappsctx_callback_thread_completed(
  const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered");

  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, &(user_data->data), 0, NULL);
  } else {
    picojson::object data;
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, &data, user_data->error_type, user_data->error_msg);
  }
  return true;
}

static void* getappsctx_callback_thread(
  const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered.");
  ApplicationContextArrayPtr app_context_array_ptr(
    new ApplicationContextArray());

  int ret = app_manager_foreach_app_context(app_manager_app_context_callback,
    app_context_array_ptr.get());
  if (ret == APP_MANAGER_ERROR_NONE) {
    LoggerE("app_manager_foreach_app_context error: ERROR_NONE");
    user_data->is_success = true;

    picojson::value apps_contexts = picojson::value(picojson::array());
    picojson::array& apps_contexts_array =
      apps_contexts.get<picojson::array>();

    for (int i = 0; i < app_context_array_ptr->size(); i++) {
      ApplicationContextPtr app_ctx_ptr = app_context_array_ptr->at(i);

      apps_contexts_array.push_back(app_ctx_ptr->Value());
    }
    user_data->data.insert(std::make_pair("contexts", apps_contexts));

  } else {
    LoggerE("app_manager_foreach_app_context error: ERROR");

    if (ret == APP_MANAGER_ERROR_INVALID_PARAMETER) {
      user_data->error_type = APP_ERROR_TYPE_MISMATCH;
      snprintf(user_data->error_msg, sizeof(user_data->error_msg),
        "Invalid parameter");
    } else if (ret == APP_MANAGER_ERROR_PERMISSION_DENIED) {
      user_data->error_type = APP_ERROR_ABORT;
      snprintf(user_data->error_msg, sizeof(user_data->error_msg),
        "Permission denied");
    } else {
      user_data->error_type = APP_ERROR_UNKNOWN;
      snprintf(user_data->error_msg, sizeof(user_data->error_msg),
        "Unknown");
    }

    user_data->is_success = false;
  }
}

static gboolean callback_thread_completed
  (const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered");
  picojson::object data;

  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, &data, 0, NULL);
  } else {
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, &data, user_data->error_type, user_data->error_msg);
  }
  return true;
}

static gboolean find_callback_thread_completed
  (const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered");
  picojson::object data;

  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, &(user_data->data), 0, NULL);
  } else {
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, &data, user_data->error_type, user_data->error_msg);
  }
  return true;
}

static void* callback_thread
  (const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered. currently, nothing to do");
}

static gboolean launch_completed
  (const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered");
  picojson::object data;

  if (user_data->is_success) {
    ReplyAsync(user_data->instance, user_data->callback_id,
               true, &data, 0, NULL);
  } else {
    ReplyAsync(user_data->instance, user_data->callback_id,
               false, &data, user_data->error_type, user_data->error_msg);
  }
  return true;
}

static gboolean self_kill_completed
  (const std::shared_ptr<CallbackInfo>& user_data) {
  LoggerD("Entered");
  picojson::object data;

  // call error callback with InvalidValuesException
  ReplyAsync(user_data->instance, user_data->callback_id,
             false, &data, user_data->error_type, user_data->error_msg);
  return true;
}

static void* launch_thread(const std::shared_ptr<CallbackInfo>& user_data) {
  int ret;

  LoggerD("app_id: %s, callback_id: %d", user_data->id,
    user_data->callback_id);

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
    snprintf(user_data->error_msg, sizeof(user_data->error_msg), msg.c_str());
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

void ApplicationInstance::AppMgrGetCurrentApplication(
  const picojson::value& args, picojson::object& out) {
  const std::string& app_id = app_id_;
  LoggerD("app_id: %s", app_id.c_str());

  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get appInfo");
    ReportError(UnknownException("get_appinfo error : unknown error"), out);
    return;
  }

  ApplicationInformationPtr app_info_ptr = get_app_info(handle);
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  ApplicationPtr app_ptr = ApplicationPtr(new Application());
  app_ptr->set_app_info(app_info_ptr);

  LoggerD("set appinfo to application");
  {
    int pid = getpid();  // DO NOT USE getppid();
    LoggerD("context id = %d", pid);

    std::stringstream sstr;
    sstr << pid;
    app_ptr->set_context_id(sstr.str());
  }

  ReportSuccess(app_ptr->Value(), out);
}

void ApplicationInstance::AppMgrKill(const picojson::value& args,
  picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& context_id = args.get("contextId").get<std::string>();

  LoggerD("callbackId = %d", callback_id);
  LoggerD("contextId = %s", context_id.c_str());

  if (context_id.empty()) {
    LoggerE("contextId is mandatory field.");
    ReportError(InvalidValuesException("Context id is mandatory field."), out);
    return;
  }

  int ret;
  int pid;
  std::stringstream(context_id) >> pid;

  if (pid <= 0) {
    LoggerE("Given context id is wrong.");
    ReportError(InvalidValuesException("Given context id is wrong."), out);
    return;
  }

  // TC checks this case via error callback... moved it to callback
  // if kill request is come for current context,
  // throw InvalidValueException by spec
  if (pid == getpid()) {
    LoggerE("Given context id is same with me.");

    auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
    user_data->instance = this;
    user_data->callback_id = callback_id;
    user_data->error_type = APP_ERROR_INVALID_VALUES;
    snprintf(user_data->error_msg, sizeof(user_data->error_msg),
      "Given context id is same with me.");

    common::TaskQueue::GetInstance().Queue<CallbackInfo>(
      callback_thread, self_kill_completed, user_data);

    ReportSuccess(out);
    return;
  }

  char *app_id_cstr = NULL;
  ret = app_manager_get_app_id(pid, &app_id_cstr);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Error while getting app id");
    ReportError(NotFoundException("Error while getting app id"), out);
    return;
  }

  std::string app_id = app_id_cstr;
  free(app_id_cstr);

  app_context_h app_context;
  ret = app_manager_get_app_context(app_id.c_str(), &app_context);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Error while getting app context");
    ReportError(NotFoundException("Error while getting app context"), out);
    return;
  }

  CallbackInfo* info = new CallbackInfo;
  info->instance = this;
  snprintf(info->id, sizeof(info->id), "%s", context_id.c_str());
  info->callback_id = callback_id;

  ret = app_manager_set_app_context_event_cb(
    app_manager_app_context_event_callback, reinterpret_cast<void*>(info));
  if (ret != APP_MANAGER_ERROR_NONE) {
    if (info)
      free(info);
    LoggerE("Error while registering app context event");
    ReportError(InvalidValuesException(
      "Error while registering app context event"), out);
    return;
  }

  ret = app_manager_terminate_app(app_context);
  if (ret != APP_MANAGER_ERROR_NONE) {
    if (info)
      free(info);
    LoggerE("Error while terminating app");
    ReportError(InvalidValuesException("Error while terminating app"), out);
    return;
  }

  ReportSuccess(out);
}

void ApplicationInstance::AppMgrLaunch(const picojson::value& args,
  picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& id = args.get("id").get<std::string>();

  LoggerD("callbackId = %d", callback_id);
  LoggerD("appId = %s", id.c_str());

  if (id.empty()) {
    LoggerE("app_id is mandatory field.");
    ReportError(InvalidValuesException("App id is mandatory field."), out);
    return;
  }

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  snprintf(user_data->id, sizeof(user_data->id), "%s", id.c_str());
  user_data->callback_id = callback_id;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(
    launch_thread, launch_completed, user_data);

  ReportSuccess(out);
}

void ApplicationInstance::AppMgrLaunchAppControl(const picojson::value& args,
  picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  std::string id;  // app id is optional
  if (args.contains("id")) {
    id = args.get("id").get<std::string>();
    LoggerD("app_id = %s", id.c_str());
  }
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

  if (app_control.contains("data")) {
    if (app_control.get("data").is<picojson::null>() == false) {
      std::vector<picojson::value> data_array =
        app_control.get("data").get<picojson::array>();
      for (int i = 0; i < data_array.size(); i++) {
        ApplicationControlDataPtr ctr_data_ptr(new ApplicationControlData());

        picojson::value each_data = data_array.at(i);
        std::string key = each_data.get("key").get<std::string>();
        LoggerD("%d: key = %s", i, key.c_str());

        ctr_data_ptr->set_ctr_key(key);

        std::vector<picojson::value> values =
          each_data.get("value").get<picojson::array>();
        for (int j = 0; j < values.size(); j++) {
          std::string val = values.at(i).to_str();

          ctr_data_ptr->add_ctr_value(val);
          LoggerD("-%d val = %s", j, val.c_str());
        }
        app_ctr_ptr->add_data_array(ctr_data_ptr);
      }
    } else {
      LoggerD("data is null.");
    }
  }

  // check parameters
  if (operation.empty()) {
    LoggerE("operation is mandatory field.");
    ReportError(InvalidValuesException("operation is mandatory field."), out);
    return;
  }

  app_control_h service;
  int ret = app_control_create(&service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    ReportError(UnknownException("Creating app_control is failed."), out);
    return;
  }

  if (id.empty() == false) {
    LoggerD("set_app_id: %s", id.c_str());
    ret = app_control_set_app_id(service, id.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(UnknownException("Setting app_id is failed."), out);
      app_control_destroy(service);
      return;
    }
  } else {
    LoggerD("app_id is empty");
  }

  ret = app_control_set_operation(service, operation.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    ReportError(InvalidValuesException("operation is invalid parameter"), out);
    app_control_destroy(service);
    return;
  }

  std::string uri = app_ctr_ptr->get_uri();
  if (!uri.empty()) {
    ret = app_control_set_uri(service, uri.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(InvalidValuesException("uri is invalid parameter"), out);
      app_control_destroy(service);
      return;
    }
  }

  std::string mime = app_ctr_ptr->get_mime();
  if (!mime.empty()) {
    ret = app_control_set_mime(service, mime.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(InvalidValuesException("mime is invalid parameter"), out);
      app_control_destroy(service);
      return;
    }
  }

  std::string category = app_ctr_ptr->get_category();
  if (!category.empty()) {
    ret = app_control_set_category(service, category.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(InvalidValuesException("category is invalid parameter"),
        out);
      app_control_destroy(service);
      return;
    }
  }

  CallbackInfo* info = new CallbackInfo;
  info->instance = this;
  info->callback_id = callback_id;

  LoggerD("Try to launch...");
  ret = app_control_send_launch_request(service,
    app_control_reply_callback, reinterpret_cast<void*>(info));

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  if (ret != APP_CONTROL_ERROR_NONE) {
    switch (ret) {
      case APP_CONTROL_ERROR_INVALID_PARAMETER:
        LoggerD("launch_request is failed. ERROR_INVALID_PARAMETER");
        user_data->error_type = APP_ERROR_TYPE_MISMATCH;
        snprintf(user_data->error_msg, sizeof(user_data->error_msg),
          "launch_request is failed. INVALID_PARAMETER");
        break;
      case APP_CONTROL_ERROR_OUT_OF_MEMORY:
        LoggerD("launch_request is failed. ERROR_OUT_OF_MEMORY");
        user_data->error_type = APP_ERROR_UNKNOWN;
        snprintf(user_data->error_msg, sizeof(user_data->error_msg),
          "launch_request is failed. OUT_OF_MEMORY");
        break;
      case APP_CONTROL_ERROR_LAUNCH_REJECTED:
        LoggerD("launch_request is failed. ERROR_LAUNCH_REJECTED");
        user_data->error_type = APP_ERROR_ABORT;
        snprintf(user_data->error_msg, sizeof(user_data->error_msg),
          "launch_request is failed. LAUNCH_REJECTED");
        break;
      case APP_CONTROL_ERROR_APP_NOT_FOUND:
        LoggerD("launch_request is failed. ERROR_APP_NOT_FOUND");
        user_data->error_type = APP_ERROR_NOT_FOUND;
        snprintf(user_data->error_msg, sizeof(user_data->error_msg),
          "launch_request is failed. NOT_FOUND");
        break;
      default:
        LoggerD("launch_request is failed.");
        user_data->error_type = APP_ERROR_UNKNOWN;
        snprintf(user_data->error_msg, sizeof(user_data->error_msg),
          "launch_request is failed. UNKNOWN");
        break;
    }
    user_data->is_success = false;
  } else {
    user_data->is_success = true;
  }
  // This causes to invalidate callback. This is not expected.
  // TODO: Refactor launchAppControl
  //common::TaskQueue::GetInstance().Queue<CallbackInfo>(
  //  callback_thread, callback_thread_completed, user_data);

  ret = app_control_destroy(service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerD("app_control_destroy is failed");
  }

  ReportSuccess(out);
}

void ApplicationInstance::AppMgrFindAppControl(const picojson::value& args,
  picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  LoggerD("callbackId = %d", callback_id);

  ApplicationControlPtr app_ctr_ptr(new ApplicationControl());

  picojson::value app_control = args.get("appControl");
  char print_buf[300] = {0};
  snprintf(print_buf, sizeof(print_buf), app_control.serialize().c_str());
  LoggerD("appControl: %s", print_buf);
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

  if (app_control.contains("data")) {
    if (app_control.get("data").is<picojson::null>() == false) {
      std::vector<picojson::value> data_array =
        app_control.get("data").get<picojson::array>();
      for (int i = 0; i < data_array.size(); i++) {
        ApplicationControlDataPtr ctr_data_ptr(new ApplicationControlData());

        picojson::value each_data = data_array.at(i);
        std::string key = each_data.get("key").get<std::string>();
        LoggerD("%d: key = %s", i, key.c_str());

        ctr_data_ptr->set_ctr_key(key);

        std::vector<picojson::value> values =
          each_data.get("value").get<picojson::array>();
        for (int j = 0; j < values.size(); j++) {
          std::string val = values.at(i).to_str();

          ctr_data_ptr->add_ctr_value(val);
          LoggerD("-%d val = %s", j, val.c_str());
        }

        app_ctr_ptr->add_data_array(ctr_data_ptr);
      }
    } else {
      LoggerD("data is null.");
    }
  }

  // check parameters
  if (operation.empty()) {
    LoggerE("operation is mandatory field.");
    ReportError(InvalidValuesException("operation is mandatory field."), out);
    return;
  }

  app_control_h service;
  int ret = app_control_create(&service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    ReportError(UnknownException("Creating app_control is failed."), out);
    app_control_destroy(service);
    return;
  }

  ret = app_control_set_operation(service, operation.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    ReportError(InvalidValuesException("operation is invalid parameter"), out);
    app_control_destroy(service);
    return;
  }

  std::string uri = app_ctr_ptr->get_uri();
  if (!uri.empty()) {
    ret = app_control_set_uri(service, uri.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(InvalidValuesException("uri is invalid parameter"), out);
      app_control_destroy(service);
      return;
    }
  }

  std::string mime = app_ctr_ptr->get_mime();
  if (!mime.empty()) {
    ret = app_control_set_mime(service, mime.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(InvalidValuesException("mime is invalid parameter"), out);
      app_control_destroy(service);
      return;
    }
  }

  std::string category = app_ctr_ptr->get_category();
  if (!category.empty()) {
    ret = app_control_set_category(service, category.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      ReportError(InvalidValuesException("category is invalid parameter"),
        out);
      app_control_destroy(service);
      return;
    }
  }

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  ApplicationInformationArrayPtr app_info_array_ptr(
    new ApplicationInformationArray());

  LoggerD("Try to find...");
  ret = app_control_foreach_app_matched(service,
    app_control_app_matched_callback,
    reinterpret_cast<void*>(app_info_array_ptr.get()));

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
    LoggerD("launch_request is failed. ERROR_INVALID_PARAMETER");
    user_data->error_type = APP_ERROR_TYPE_MISMATCH;
    snprintf(user_data->error_msg, sizeof(user_data->error_msg),
      "launch_request is failed. INVALID_PARAMETER");
    user_data->is_success = false;
  } else {
    LoggerD("launch_request is failed. UNKNOWN");
    user_data->error_type = APP_ERROR_UNKNOWN;
    snprintf(user_data->error_msg, sizeof(user_data->error_msg),
      "launch_request is failed. OUT_OF_MEMORY");
    user_data->is_success = false;
  }

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(callback_thread,
    find_callback_thread_completed, user_data);

  ret = app_control_destroy(service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerD("app_control_destroy is failed");
  }

  ReportSuccess(out);
}

void ApplicationInstance::AppMgrGetAppsContext(const picojson::value& args,
  picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(
    getappsctx_callback_thread, getappsctx_callback_thread_completed,
    user_data);

  ReportSuccess(out);
}

void ApplicationInstance::AppMgrGetAppContext(const picojson::value& args,
  picojson::object& out) {
  LoggerD("ENTER");

  std::string context_id;
  if (args.contains("contextId")) {
    LoggerD("ENTER2");
    context_id = args.get("contextId").get<std::string>();
    LoggerD("contextId = %s", context_id.c_str());
  } else {
    LoggerD("contextId is null");
  }
  int ret = 0;

  std::string cur_ctx_id = context_id;
  int pid;

  if (cur_ctx_id.empty()) {
    pid = getpid();
    std::stringstream sstr;
    sstr << pid;
    cur_ctx_id = sstr.str();
  } else {
    std::stringstream(context_id) >> pid;
    if (pid <= 0) {
      LoggerE("Given context_id is wrong");
      ReportError(NotFoundException("Given context_id is wrong"), out);
      return;
    }
  }

  char *app_id = NULL;
  LoggerD("pid: %d", pid);

  ret = app_manager_get_app_id(pid, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    // Handles error case
    if (app_id) {
      free(app_id);
    }
    switch (ret) {
      case APP_MANAGER_ERROR_INVALID_PARAMETER:
        LoggerE("get_app_id error : invalid parameter");
        ReportError(NotFoundException("get_app_id error : invalid parameter"),
          out);
        return;
      case APP_MANAGER_ERROR_NO_SUCH_APP:
        LoggerE("get_app_id error : no such app");
        ReportError(NotFoundException("get_app_id error : no such app"), out);
        return;
      case APP_MANAGER_ERROR_DB_FAILED:
        LoggerE("get_app_id error : db failed");
        ReportError(NotFoundException("get_app_id error : db failed"), out);
        return;
      case APP_MANAGER_ERROR_OUT_OF_MEMORY:
        LoggerE("get_app_id error : out of memory");
        ReportError(NotFoundException("get_app_id error : out of memory"),
          out);
        return;
      default:
        LoggerE("get_app_id known error");
        ReportError(UnknownException("get_app_id error : unknown error"), out);
        return;
    }
  }

  ApplicationContextPtr app_ctx(new ApplicationContext());
  app_ctx->set_app_id(app_id);
  app_ctx->set_context_id(cur_ctx_id);

  if (app_id)
    free(app_id);

  LoggerD("appCtx: id = %s", app_ctx->get_context_id().c_str());
  LoggerD("appCtx: appId = %s", app_ctx->get_app_id().c_str());

  ReportSuccess(picojson::value(app_ctx->Value()), out);
}

void ApplicationInstance::AppMgrGetAppsInfo(const picojson::value& args,
  picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  auto user_data = std::shared_ptr<CallbackInfo>(new CallbackInfo);
  user_data->instance = this;
  user_data->callback_id = callback_id;

  common::TaskQueue::GetInstance().Queue<CallbackInfo>(
    getappsinfo_callback_thread, getappsinfo_callback_thread_completed,
    user_data);

  ReportSuccess(out);
}

void ApplicationInstance::AppMgrGetAppInfo(const picojson::value& args,
  picojson::object& out) {
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

  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get appInfo");
    ReportError(NotFoundException("Given app id is not found"), out);
    return;
  }

  ApplicationInformationPtr app_info_ptr = get_app_info(handle);
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  ReportSuccess(app_info_ptr->Value(), out);
}

void ApplicationInstance::AppMgrGetAppCerts(const picojson::value& args,
  picojson::object& out) {
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

  int ret = 0;
  char* package = getPackageByAppId(id.c_str());
  if (package == NULL) {
    LoggerE("Can not get package");
    ReportError(NotFoundException("Can not get package"), out);
    return;
  }

  // TODO(sunggyu.choi): gPkgIdMapInited
  package_info_h pkg_info;
  int result = 0;
  result = package_info_create(package, &pkg_info);
  if (result != PACKAGE_MANAGER_ERROR_NONE) {
    ReportError(UnknownException("Can not get package info"), out);
    return;
  }

  ApplicationCertificateArrayPtr cert_array_ptr(
    new ApplicationCertificateArray());

  result = package_info_foreach_cert_info(pkg_info, package_certificate_cb,
            reinterpret_cast<void*>(cert_array_ptr.get()));
  if ((result != PACKAGE_MANAGER_ERROR_NONE) &&
      (result != PACKAGE_MANAGER_ERROR_IO_ERROR)) {
    ReportError(UnknownException("Can not get package cert info"), out);
    return;
  }

  picojson::value cert_data = picojson::value(picojson::array());
  picojson::array& cert_data_array = cert_data.get<picojson::array>();

  for (int i = 0; i < cert_array_ptr->size(); i++) {
    ApplicationCertificatePtr cert_data_ptr = cert_array_ptr->at(i);
    cert_data_array.push_back(cert_data_ptr->Value());
  }

  ReportSuccess(cert_data, out);
}

void ApplicationInstance::AppMgrGetAppSharedURI(const picojson::value& args,
  picojson::object& out) {
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

#define TIZENAPIS_APP_FILE_SCHEME      "file://"
#define TIZENAPIS_APP_SLASH            "/"
#define TIZENAPIS_APP_SHARED           "shared"

  app_info_h handle;
  char* pkg_name = NULL;

  int ret = app_manager_get_app_info(id.c_str(), &handle);
  if (ret != APP_ERROR_NONE) {
    LoggerD("Fail to get appinfo");
    ReportError(NotFoundException("Fail to get appinfo"), out);
    return;
  }

  ret = app_info_get_package(handle, &pkg_name);
  if ((ret != APP_ERROR_NONE) || (pkg_name == NULL)) {
    LoggerD("Fail to get pkg_name");
    ReportError(NotFoundException("Fail to get pkg_name"), out);
    return;
  }

  app_info_destroy(handle);

  pkgmgrinfo_pkginfo_h pkginfo_h;
  char* root_path = NULL;

  ret = pkgmgrinfo_pkginfo_get_pkginfo(pkg_name, &pkginfo_h);
  if (ret != PMINFO_R_OK) {
    free(pkg_name);
    ReportError(UnknownException("Fail to get pkginfo"), out);
    return;
  }

  ret = pkgmgrinfo_pkginfo_get_root_path(pkginfo_h, &root_path);
  if ((ret != PMINFO_R_OK) || (root_path == NULL)) {
     LoggerE("Fail to get root path");
     free(pkg_name);
     ReportError(UnknownException("Fail to get root path"), out);
     return;
  }

  std::string shared_URI = TIZENAPIS_APP_FILE_SCHEME + std::string(root_path) +
    TIZENAPIS_APP_SLASH + TIZENAPIS_APP_SHARED + TIZENAPIS_APP_SLASH;
  free(pkg_name);

  pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_h);

  ReportSuccess(picojson::value(shared_URI), out);
}

void ApplicationInstance::AppMgrGetAppMetaData(const picojson::value& args,
  picojson::object& out) {
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

  int ret = 0;
  pkgmgrinfo_appinfo_h handle;
  ret = pkgmgrinfo_appinfo_get_appinfo(id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    ReportError(NotFoundException("Cannot find app with given app_id"), out);
    return;
  }

  ApplicationMetaDataArrayPtr meta_data_array_ptr(
    new ApplicationMetaDataArray());
  ret = pkgmgrinfo_appinfo_foreach_metadata(handle, app_meta_data_cb,
    reinterpret_cast<void*>(meta_data_array_ptr.get()));
  if (ret != PMINFO_R_OK) {
    LoggerE("pkgmgrinfo_appinfo_metadata_filter_foreach() failed");
    pkgmgrinfo_appinfo_destroy_appinfo(handle);
    ReportError(UnknownException("fail to get custom tag"), out);
    return;
  }
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  picojson::value meta_data = picojson::value(picojson::array());
  picojson::array& meta_data_array = meta_data.get<picojson::array>();

  for (int i = 0; i < meta_data_array_ptr->size(); i++) {
    ApplicationMetaDataPtr meta_data_ptr = meta_data_array_ptr->at(i);

    meta_data_array.push_back(meta_data_ptr->Value());
  }

  char print_buf[300] = {0};
  snprintf(print_buf, sizeof(print_buf), meta_data.serialize().c_str());
  LoggerD("GetAppMetaData:Result: %s", print_buf);

  ReportSuccess(meta_data, out);
}

void ApplicationInstance::AppMgrAddAppInfoEventListener(
  const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callback_id = static_cast<int>(args.get("callbackId").get<double>());

  if (manager_handle_ != NULL) {
    LoggerD("AppListChanged callback is already registered. watch_id_ = %d",
      watch_id_);
    ReportError(UnknownException("Listener is already registered."), out);
    return;
  }
  manager_handle_ = pkgmgr_client_new(PC_LISTENING);
  if (manager_handle_ == NULL) {
    ReportError(UnknownException("Error while registering listener"), out);
    return;
  }

  pkgmgr_client_listen_status(manager_handle_, app_list_changed_cb, this);

  callback_id_list_.push_back(callback_id);
  const double ret = static_cast<double>(get_watch_id_and_increase());
  ReportSuccess(picojson::value(ret), out);
}

void ApplicationInstance::AppMgrRemoveAppInfoEventListener(
  const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)
  int watch_id = static_cast<int>(args.get("watchId").get<double>());

  LoggerD("watch_id = %d", watch_id);

  if (manager_handle_ == NULL) {
    LoggerE("Listener is not added before.");
    ReportError(UnknownException("Listener is not added before."), out);
    return;
  }

  callback_id_list_.clear();

  if (watch_id_ != watch_id) {
    LoggerE("Invalid watch id: %d", watch_id);
    ReportError(InvalidValuesException("Watch id is invalid."), out);
    return;
  }

  pkgmgr_client_free(manager_handle_);
  manager_handle_ = NULL;

  ReportSuccess(out);
}

void ApplicationInstance::AppExit(const picojson::value& args,
  picojson::object& out) {
  LoggerD("Hide is called");

  // webkit
  // IPCSupport::Instance().Post(IPCMsg::MsgExitApp(), "" );
  // Blink
  //IPCMessageSupport::sendAsyncMessageToUiProcess(
  //  IPCMessageSupport::TIZEN_EXIT, NULL, NULL, NULL);
  ReportSuccess(out);
}

void ApplicationInstance::AppHide(const picojson::value& args,
  picojson::object& out) {
  LoggerD("Hide is called");

  // webkit
  // IPCSupport::Instance().Post(IPCMsg::MsgHideApp(), "" );
  // Blink
  //IPCMessageSupport::sendAsyncMessageToUiProcess(
  //  IPCMessageSupport::TIZEN_HIDE, NULL, NULL, NULL);
  ReportSuccess(out);
}

void ApplicationInstance::AppGetRequestedAppControl(
  const picojson::value& args, picojson::object& out) {
  std::string bundle_str =
    common::Extension::GetRuntimeVariable("encoded_bundle", 1024);
  if (bundle_str.empty()) {
    LoggerE("Getting encoded_bundle is failed");
    ReportError(UnknownException("Gettng encoded_bundle is failed"), out);
    return;
  }

  app_control_h service = NULL;
  char* tmp_str = NULL;
  bundle* request_bundle = bundle_decode((bundle_raw*)bundle_str.c_str(),
    bundle_str.length());
  if (request_bundle == NULL) {
    ReportError(UnknownException("Decoding bundle is failed"), out);
    return;
  }

  int ret = app_control_create_event(request_bundle, &service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Fail to create event");
    bundle_free(request_bundle);
    ReportError(UnknownException("Failed to create event"), out);
    return;
  }
  bundle_free(request_bundle);

  ApplicationControlPtr app_ctr_ptr(new ApplicationControl());

  ret = app_control_get_operation(service, &tmp_str);
  if (ret == APP_CONTROL_ERROR_NONE && tmp_str != NULL) {
    app_ctr_ptr->set_operation(tmp_str);
    free(tmp_str);
    tmp_str = NULL;
  }

  ret = app_control_get_uri(service, &tmp_str);
  if (ret == APP_CONTROL_ERROR_NONE && tmp_str != NULL) {
    app_ctr_ptr->set_uri(tmp_str);
    free(tmp_str);
    tmp_str = NULL;
  }

  ret = app_control_get_mime(service, &tmp_str);
  if (ret == APP_CONTROL_ERROR_NONE && tmp_str != NULL) {
    app_ctr_ptr->set_mime(tmp_str);
    free(tmp_str);
    tmp_str = NULL;
  }

  ret = app_control_get_category(service, &tmp_str);
  if (ret == APP_CONTROL_ERROR_NONE && tmp_str != NULL) {
    app_ctr_ptr->set_category(tmp_str);
    free(tmp_str);
    tmp_str = NULL;
  }

  ApplicationControlDataArrayPtr app_ctr_data_array_ptr(
    new ApplicationControlDataArray());
  ret = app_control_foreach_extra_data(service,
    app_control_extra_data_callback, app_ctr_data_array_ptr.get());
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_foreach_extra_data fail");
    ReportError(UnknownException("Getting extra data is failed"), out);
    return;
  } else {
    app_ctr_ptr->set_data_array(*(app_ctr_data_array_ptr.get()));
  }

  RequestedApplicationControlPtr req_app_ctr_ptr(
    new RequestedApplicationControl());
  req_app_ctr_ptr->set_app_control(*(app_ctr_ptr.get()));

  // add caller id
  ret = app_control_get_caller(service, &tmp_str);
  if (ret == APP_CONTROL_ERROR_NONE && tmp_str != NULL) {
    req_app_ctr_ptr->set_caller_app_id(tmp_str);
    free(tmp_str);
    tmp_str = NULL;
  } else {
    LoggerE("Failed to get caller application ID");
    ReportError(NotFoundException("Failed to get caller application ID"), out);
    return;
  }

  std::pair<std::map<std::string, app_control_h>::iterator, bool> result =
    reply_map_.insert(
      std::map<std::string, app_control_h>::value_type(
        req_app_ctr_ptr->get_caller_app_id(), service));
  if (result.second) {
    LoggerD("Adding item succeeded");
  } else {
    LoggerD("Adding item failed");
  }

  ReportSuccess(req_app_ctr_ptr->Value(), out);
}

void ApplicationInstance::RequestedAppControlReplyResult(
  const picojson::value& args, picojson::object& out) {
  ApplicationControlDataArrayPtr app_ctr_data_array_ptr(
    new ApplicationControlDataArray());
  std::string caller_app_id;
  if (args.contains("callerAppId")) {
    caller_app_id = args.get("callerAppId").get<std::string>();
  } else {
    ReportError(InvalidValuesException("unidentified caller"), out);
    return;
  }

  if (args.contains("data")) {
    if (args.get("data").is<picojson::null>() == false) {
      picojson::array data_array = args.get("data").get<picojson::array>();

      int size = data_array.size();
      LoggerD("size = %d", size);
      for (int i = 0; i < size; i++) {
        ApplicationControlData* app_ctr_data = new ApplicationControlData();
        ApplicationControlDataPtr app_ctr_data_ptr(app_ctr_data);

        picojson::value& ctr_data = data_array.at(i);
        std::string key = ctr_data.get("key").get<std::string>();
        app_ctr_data_ptr->set_ctr_key(key);

        picojson::array value_array =
          ctr_data.get("value").get<picojson::array>();
        int value_size = value_array.size();

        LoggerD("value size = %d", value_size);
        for (int j = 0; j < value_size; j++) {
          picojson::value& value_data = value_array.at(i);
          std::string value = value_data.get<std::string>();
          LoggerD("value: %s", value.c_str());
          app_ctr_data_ptr->add_ctr_value(value);
        }
        app_ctr_data_array_ptr->push_back(app_ctr_data_ptr);
      }
    } else {
      LoggerD("data is null.");
    }
  }

  app_control_h reply;
  app_control_create(&reply);

  if (!app_ctr_data_array_ptr->empty()) {
    const char** arr = NULL;

    int size = app_ctr_data_array_ptr->size();
    LoggerD("size: %d", size);
    for (size_t i = 0; i < size; i++) {
      std::vector<std::string> value_array =
        app_ctr_data_array_ptr->at(i)->get_ctr_value();
      arr = (const char**) calloc (sizeof(char*), value_array.size());

      if (arr != NULL) {
        for (size_t j = 0; j < value_array.size(); j++) {
          arr[j] = value_array.at(j).c_str();
          LoggerD("[index: %d][value: %s]", j, arr[j]);
        }
      }
      const char* key = app_ctr_data_array_ptr->at(i)->get_ctr_key().c_str();
      LoggerD("key: %s", key);
      app_control_add_extra_data_array(reply, key, arr, value_array.size());
      if (arr) {
        free(arr);
      }
    }
  } else {
    LoggerE("[replyResult] app_ctr_data_array_ptr is empty");
  }

  std::map<std::string, app_control_h>::iterator it =
    reply_map_.find(caller_app_id);
  if (it == reply_map_.end()) {
    LoggerE("caller handle is not found");
    app_control_destroy(reply);
    ReportError(NotFoundException("caller handle is not found"), out);
    return;
  }

  bool running = false;
  int ret = app_manager_is_running(caller_app_id.c_str(), &running);
  if ((ret != APP_MANAGER_ERROR_NONE) || !running) {
    LoggerE("caller is not running");
    app_control_destroy(reply);
    ReportError(NotFoundException("Cannot find caller"), out);
    return;
  }

  ret = app_control_reply_to_launch_request(reply, it->second,
    APP_CONTROL_RESULT_SUCCEEDED);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Cannot find caller");
    app_control_destroy(reply);
    ReportError(NotFoundException("Cannot find caller"), out);
    return;
  }

  reply_map_.erase(it);
  app_control_destroy(reply);

  ReportSuccess(out);
}

void ApplicationInstance::RequestedAppControlReplyFailure(
  const picojson::value& args, picojson::object& out) {
  std::string caller_app_id;
  if (args.contains("callerAppId")) {
    caller_app_id = args.get("callerAppId").get<std::string>();
  } else {
    ReportError(InvalidValuesException("unidentified caller"), out);
    return;
  }

  app_control_h reply;
  app_control_create(&reply);

  std::map<std::string, app_control_h>::iterator it =
    reply_map_.find(caller_app_id);
  if (it == reply_map_.end()) {
    LoggerE("caller handle is not found");
    ReportError(NotFoundException("caller handle is not found"), out);
    return;
  }

  bool running = false;
  int ret = app_manager_is_running(caller_app_id.c_str(), &running);
  if ((ret != APP_MANAGER_ERROR_NONE) || !running) {
    LoggerE("caller is not running");
    app_control_destroy(reply);
    ReportError(NotFoundException("Cannot find caller"), out);
    return;
  }

  ret = app_control_reply_to_launch_request(reply, it->second,
    APP_CONTROL_RESULT_FAILED);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("Cannot find caller");
    app_control_destroy(reply);
    ReportError(NotFoundException("Cannot find caller"), out);
    return;
  }

  reply_map_.erase(it);
  app_control_destroy(reply);

  ReportSuccess(out);
}

ApplicationInformationPtr ApplicationInstance::GetAppInfo(
  const std::string app_id) {
  LoggerD("app_id: %s", app_id.c_str());

  pkgmgrinfo_appinfo_h handle;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get appInfo");
    ApplicationInformationPtr app_info_ptr(new ApplicationInformation());
    return app_info_ptr;
  }

  ApplicationInformationPtr app_info_ptr = get_app_info(handle);
  pkgmgrinfo_appinfo_destroy_appinfo(handle);

  return app_info_ptr;
}

int ApplicationInstance::app_list_changed_cb(int id, const char *type,
  const char *package, const char *key, const char *val, const void *msg,
  void *data) {
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

      ret = package_info_foreach_app_from_package(package_info,
        PACKAGE_INFO_ALLAPP, app_callback, data);
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
        // ApplicationInstance* app_instance = (ApplicationInstance*)data;
        ApplicationInstance* app_instance =
          reinterpret_cast<ApplicationInstance*>(data);
        app_instance->ReplyAppListChangedCallback(event_type, package, data);
      }
    }
  }

  return APP_MANAGER_ERROR_NONE;
}

bool ApplicationInstance::app_callback(
  package_info_app_component_type_e comp_type, const char *app_id,
  void *user_data) {
  LoggerD("ENTERED");

  if (app_id == NULL) {
    LoggerE("Callback is called. but no package name is passed");
    return true;
  }

  if (user_data == NULL) {
    LoggerE("user data is not exist. skip this request");
    return true;
  }

  LoggerD("app_id = %s", app_id);
  ApplicationInstance* app_instance =
    reinterpret_cast<ApplicationInstance*>(user_data);
  app_instance->app_list_.push_back(app_id);

  return true;
}

void ApplicationInstance::ReplyAppListChangedCallback(
  app_info_event_e event_type, const char* pkg_id, void* user_data) {
  LoggerD("ENTERED");

  ApplicationInstance* app_instance =
    reinterpret_cast<ApplicationInstance*>(user_data);

  if (event_type == APP_INFO_EVENT_UNINSTALLED) {
    for (size_t i = 0; i < app_list_.size(); i++) {
      // onuninstalled
      LoggerD("onuninstalled: %d of %d", i, app_list_.size());
      std::string app_id = app_list_.at(i);

      for (size_t j = 0; j < callback_id_list_.size(); j++) {
        int callback_id = callback_id_list_.at(j);
        LoggerD("%d th callback_id(%d) of %d", j, callback_id,
          callback_id_list_.size());
        picojson::object data;
        data.insert(std::make_pair("type", picojson::value("onuninstalled")));
        data.insert(std::make_pair("id", picojson::value(app_id)));
        ReplyAsync(this, callback_id, true, &data, 0, NULL);
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
    ret = package_info_foreach_app_from_package(package_info,
      PACKAGE_INFO_ALLAPP, app_callback, user_data);
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
      switch (event_type) {
        case APP_INFO_EVENT_INSTALLED:
          {
            // oninstalled
            LoggerD("oninstalled: %d of %d", i, app_list_.size());
            std::string app_id = app_list_.at(i);

            ApplicationInformationPtr app_info_ptr = GetAppInfo(app_id);

            for (size_t j = 0; j < callback_id_list_.size(); j++) {
              int callback_id = callback_id_list_.at(j);
              LoggerD("%d th callback_id(%d) of %d", j, callback_id,
                callback_id_list_.size());
              picojson::object data;
              data.insert(std::make_pair("type",
                picojson::value("oninstalled")));
              data.insert(std::make_pair("info",
                picojson::value(app_info_ptr->Value())));
              ReplyAsync(this, callback_id, true, &data, 0, NULL);
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
              LoggerD("%d th callback_id(%d) of %d", j, callback_id,
                callback_id_list_.size());
              picojson::object data;
              data.insert(std::make_pair("type",
                picojson::value("onupdated")));
              data.insert(std::make_pair("info",
                picojson::value(app_info_ptr->Value())));
              ReplyAsync(this, callback_id, true, &data, 0, NULL);
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

int ApplicationInstance::get_watch_id_and_increase() {
  return ++watch_id_;
}
#undef CHECK_EXIST

}  // namespace application
}  // namespace extension
