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

#include "package/package_instance.h"

#include <functional>
#include <string>

#include "package/package_info_provider.h"
#include "common/logger.h"
#include "common/task-queue.h"
#include "common/picojson.h"

namespace extension {
namespace package {

using common::TaskQueue;
using common::PlatformException;
using common::UnknownException;
using common::NotFoundException;
using common::TypeMismatchException;
using common::SecurityException;

using common::ErrorCode;
using common::PlatformResult;

typedef enum _PackageThreadWorkType {
  PackageThreadWorkNone = 0,
  PackageThreadWorkGetPackagesInfo,
} PackageThreadWorkType;

class PackageUserData {
 public:
  PackageUserData(PackageInstance* ins,
      int id,
      PackageThreadWorkType task) {
    instance_ = ins;
    callback_id_ = id;
    work_ = task;
  }

  PackageInstance* instance_;
  int callback_id_;
  PackageThreadWorkType work_;
  picojson::object data_;
};
typedef std::shared_ptr<PackageUserData> PackageUserDataPtr;

static void* PackageThreadWork(
    const PackageUserDataPtr& userData) {
  LoggerD("Enter");

  switch ( userData->work_ ) {
    case PackageThreadWorkGetPackagesInfo: {
      picojson::object output;
      PackageInfoProvider::GetPackagesInfo(output);
      userData->data_ = output;
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
    }
  }

  return NULL;
}

static gboolean PackageAfterWork(
    const PackageUserDataPtr& userData) {
  LoggerD("Enter");

  userData->data_["callbackId"] =
      picojson::value(static_cast<double>(userData->callback_id_));
  picojson::value result = picojson::value(userData->data_);
  common::Instance::PostMessage(userData->instance_, result.serialize().c_str());

  return FALSE;
}

static void PackageRequestCb(
    int id, const char *type, const char *package,
    package_manager_event_type_e event_type,
    package_manager_event_state_e event_state, int progress,
    package_manager_error_e error, void *user_data) {
  LoggerD("Enter");

  PackageInstance* instance = static_cast<PackageInstance*>(user_data);
  if ( !instance ) {
    LoggerE("instance is NULL");
    return;
  }

  picojson::object param;
  LoggerD("Request type: %d, state: %d, progress: %d", event_type, event_state, progress);
  if (PACKAGE_MANAGER_EVENT_STATE_FAILED == event_state) {
    LoggerE("[Failed]");
    param["status"] = picojson::value("error");
    param["error"] = UnknownException(
        "It is not allowed to install the package by the platform or " \
        "any other platform error occurs").ToJSON();
  } else if (PACKAGE_MANAGER_EVENT_STATE_STARTED == event_state ||
      PACKAGE_MANAGER_EVENT_STATE_PROCESSING == event_state) {
    // this 'or' condition is needed to handle onprogress callback even on uninstall process,
    // with this additional check manual TCT uninstall/onprogress pass
    param["status"] = picojson::value("progress");
    param["progress"] = picojson::value(static_cast<double>(progress));
    param["id"] = picojson::value(std::string(package));
  } else if (PACKAGE_MANAGER_EVENT_STATE_COMPLETED == event_state) {
    param["status"] = picojson::value("complete");
    param["id"] = picojson::value(std::string(package));
  }

  instance->InvokeCallback(id, param);
  if ( event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED
      || event_state == PACKAGE_MANAGER_EVENT_STATE_FAILED ) {
    LoggerD("Request has been completed");
    instance->DeregisterCallback(id);
  }
}

static void PackageListenerCb(
    const char *type, const char *package,
    package_manager_event_type_e event_type,
    package_manager_event_state_e event_state, int progress,
    package_manager_error_e error, void *user_data) {
  LoggerD("Enter");

  PackageInstance* instance = static_cast<PackageInstance*>(user_data);
  if ( !instance ) {
    LoggerE("instance is NULL");
    return;
  }

  if ( error != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed");
    return;
  }

  picojson::object param;
  param["listener"] = picojson::value("infoEvent");

  LoggerD("Listener type: %d , state: %d, progress: %d",
          event_type, event_state, progress);
  if ( event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL
      && event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED ) {
    LoggerD("[Installed]");
    param["status"] = picojson::value("installed");
    picojson::object info;
    PackageInfoProvider::GetPackageInfo(package, info);
    param["info"] = picojson::value(info["result"]);
    instance->InvokeListener(param);
  } else if ( event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL
      && event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED ) {
    LoggerD("[Uninstalled]");
    param["status"] = picojson::value("uninstalled");
    param["id"] = picojson::value(std::string(package));
    instance->InvokeListener(param);
  } else if ( event_type == PACKAGE_MANAGER_EVENT_TYPE_UPDATE
      && event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED ) {
    LoggerD("[Updated]");
    param["status"] = picojson::value("updated");
    picojson::object info;
    PackageInfoProvider::GetPackageInfo(package, info);
    param["info"] = picojson::value(info["result"]);
    instance->InvokeListener(param);
  }
}

static std::string ltrim(const std::string& s) {
    std::string str = s;
    std::string::iterator i;
    for (i = str.begin(); i != str.end(); ++i) {
        if ( !isspace(*i) ) {
            break;
        }
    }
    if ( i == str.end() ) {
        str.clear();
    } else {
        str.erase(str.begin(), i);
    }
    return str;
}

static std::string convertUriToPath(const std::string& uri) {
    std::string result;
    std::string schema("file://");
    std::string str = ltrim(uri);

    std::string _schema = str.substr(0, schema.size());
    if ( _schema == schema ) {
        result = str.substr(schema.size());
    } else {
        result = str;
    }

    return result;
}

PackageInstance::PackageInstance() {
  LoggerD("Enter");

  if ( package_manager_request_create(&request_)
      != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to created package manager request");
    request_ = NULL;
  }

  if ( package_manager_request_set_event_cb(request_, PackageRequestCb,
      static_cast<void*>(this)) != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to set request event callback");
  }

  if ( package_manager_create(&manager_) != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to created package manager");
    manager_ = NULL;
  }

  is_package_info_listener_set_ = false;

  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&PackageInstance::x, this, _1, _2));
  REGISTER_SYNC("PackageManager_setPackageInfoEventListener",
      PackageManagerSetpackageinfoeventlistener);
  REGISTER_SYNC("PackageManager_install",
      PackageManagerInstall);
  REGISTER_SYNC("PackageManager_getPackagesInfo",
      PackageManagerGetpackagesinfo);
  REGISTER_SYNC("PackageManager_uninstall",
      PackageManagerUninstall);
  REGISTER_SYNC("PackageManager_unsetPackageInfoEventListener",
      PackageManagerUnsetpackageinfoeventlistener);
  REGISTER_SYNC("PackageManager_getPackageInfo",
      PackageManagerGetpackageinfo);
  REGISTER_SYNC("PackageManager_getTotalSize",
      PackageManagerGetTotalSize);
  REGISTER_SYNC("PackageManager_getDataSize",
      PackageManagerGetDataSize);
  #undef REGISTER_SYNC
}

PackageInstance::~PackageInstance() {
  LoggerD("Enter");
  package_manager_request_unset_event_cb(request_);
  package_manager_request_destroy(request_);
  package_manager_destroy(manager_);
}

#define CHECK_EXIST(args, name, out) \
    if (  !args.contains(name) ) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void PackageInstance::RegisterCallback(
    int request_id, int callback_id) {
  LoggerD("Enter");
  callbacks_map_[request_id] = callback_id;
}

void PackageInstance::DeregisterCallback(int request_id) {
  LoggerD("Enter");
  callbacks_map_.erase(request_id);
}

void PackageInstance::InvokeCallback(
    int request_id, picojson::object& param) {
  LoggerD("Enter");

  int callback_id = callbacks_map_[request_id];

  param["callbackId"] = picojson::value(
      static_cast<double>(callback_id));
  picojson::value result = picojson::value(param);
  Instance::PostMessage(this, result.serialize().c_str());
}

void PackageInstance::PackageManagerInstall(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "packageFileURI", out)

  int callback_id = static_cast<int>(
      args.get("callbackId").get<double>());
  const std::string& packageFileURI =
      convertUriToPath(args.get("packageFileURI").get<std::string>());

  if ( !request_ ) {
    LoggerE("package_manager_request_h is NULL");
    InvokeErrorCallbackAsync(callback_id,
      UnknownException("It is not allowed to install the package by " \
          "the platform or any other platform error occurs"));
    return;
  }

  int request_id = 0;
  int ret = package_manager_request_install(
      request_, packageFileURI.c_str(), &request_id);
  if ( ret != PACKAGE_MANAGER_ERROR_NONE ) {
    if ( ret == PACKAGE_MANAGER_ERROR_INVALID_PARAMETER ) {
      LoggerE("The package is not found at the specified location");
      InvokeErrorCallbackAsync(callback_id,
          NotFoundException(
          "The package is not found at the specified location"));
    } else {
      LoggerE("It is not allowed to install the package by " \
          "the platform or any other platform error occurs");
      InvokeErrorCallbackAsync(callback_id,
          UnknownException("It is not allowed to install the package by " \
          "the platform or any other platform error occurs"));
    }
  } else {
    RegisterCallback(request_id, callback_id);
  }

  ReportSuccess(out);
}

void PackageInstance::PackageManagerUninstall(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "id", out)

  int callback_id =
      static_cast<int>(args.get("callbackId").get<double>());
  const std::string& id = args.get("id").get<std::string>();

  if ( !request_ ) {
    LoggerE("package_manager_request_h is NULL");
    InvokeErrorCallbackAsync(callback_id,
        UnknownException("It is not allowed to install the package by " \
        "the platform or any other platform error occurs"));
    return;
  }

  int request_id = 0;
  int ret = package_manager_request_uninstall(request_, id.c_str(), &request_id);
  if ( ret != PACKAGE_MANAGER_ERROR_NONE ) {
    if ( ret == PACKAGE_MANAGER_ERROR_INVALID_PARAMETER ) {
      LoggerE("The package is not found at the specified location");
      InvokeErrorCallbackAsync(callback_id,
        NotFoundException(
            "The package is not found at the specified location"));
    } else {
      LoggerE("It is not allowed to install the package by the " \
          "platform or any other platform error occurs");
      InvokeErrorCallbackAsync(callback_id, UnknownException(
          "It is not allowed to install the package by the platform or " \
          "any other platform error occurs"));
    }
  } else {
    RegisterCallback(request_id, callback_id);
  }

  ReportSuccess(out);
}
void PackageInstance::PackageManagerGetpackagesinfo(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)
  int callback_id =
      static_cast<int>(args.get("callbackId").get<double>());

  PackageUserDataPtr userData(new PackageUserData(
      this, callback_id, PackageThreadWorkGetPackagesInfo));
  TaskQueue::GetInstance().Queue<PackageUserData>(
      PackageThreadWork, PackageAfterWork, userData);
  ReportSuccess(out);
}
void PackageInstance::PackageManagerGetpackageinfo(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  if ( args.contains("id") ) {
    std::string id = args.get("id").get<std::string>();
    PackageInfoProvider::GetPackageInfo(id.c_str(), out);
  } else {
    PackageInfoProvider::GetPackageInfo(out);
  }
}

void PackageInstance::PackageManagerGetTotalSize(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");

  const auto& id = args.get("id");

  if (id.is<std::string>()) {
    PackageInfoProvider::GetTotalSize(id.get<std::string>(), &out);
  } else {
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Missing id parameter"),
                &out);
  }
}

void PackageInstance::PackageManagerGetDataSize(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Enter");

  const auto& id = args.get("id");

  if (id.is<std::string>()) {
    PackageInfoProvider::GetDataSize(id.get<std::string>(), &out);
  } else {
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Missing id parameter"),
                &out);
  }
}

void PackageInstance::InvokeListener(picojson::object& param) {
  LoggerD("Enter");
  picojson::value result = picojson::value(param);
  Instance::PostMessage(this, result.serialize().c_str());
}

void PackageInstance::
    PackageManagerSetpackageinfoeventlistener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)

  if ( is_package_info_listener_set_ ) {
    LoggerD("Already set");
    ReportSuccess(out);
    return;
  }

  if ( !manager_ ) {
    LoggerE("package_manager_h is NULL");
    ReportError(
        UnknownException("The package list change event cannot be " \
        "generated because of a platform error"),
        out);
    return;
  }

  if ( package_manager_set_event_cb(
      manager_, PackageListenerCb, static_cast<void*>(this))
      != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to set event callback");
    ReportError(
        UnknownException("The package list change event cannot be " \
        "generated because of a platform error"),
        out);
    return;
  }

  is_package_info_listener_set_ = true;
  ReportSuccess(out);
}

void PackageInstance::
    PackageManagerUnsetpackageinfoeventlistener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  if ( !is_package_info_listener_set_ ) {
    LoggerD("Listener is not set");
    ReportSuccess(out);
    return;
  }

  if ( !manager_ ) {
    LoggerE("package_manager_h is NULL");
    ReportError(
        UnknownException("The listener removal request fails" \
        "because of a platform error"),
        out);
    return;
  }

  if ( package_manager_unset_event_cb(manager_)
      != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to unset event callback");
    ReportError(
        UnknownException("The listener removal request fails" \
        "because of a platform error"),
        out);
    return;
  }

  is_package_info_listener_set_ = false;
  ReportSuccess(out);
}

void PackageInstance::InvokeErrorCallbackAsync(
    int callback_id, const PlatformException& ex) {
  LoggerD("Enter");

  picojson::object param;
  ReportError(ex, param);
  PackageUserDataPtr userData(new PackageUserData(
      this, callback_id, PackageThreadWorkNone));
  userData->data_ = param;
  TaskQueue::GetInstance().Async
    <PackageUserData>(PackageAfterWork, userData);
}

#undef CHECK_EXIST

}  // namespace package
}  // namespace extension
