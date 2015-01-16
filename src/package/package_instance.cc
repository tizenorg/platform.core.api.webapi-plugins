// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "package/package_instance.h"

#include <glib.h>

#include <functional>

#include "package/package_info_provider.h"
#include "common/logger.h"
#include "common/task-queue.h"
#include "common/picojson.h"

namespace extension {
namespace package {

namespace {
// The privileges that required in Package API
const std::string kPrivilegePackageInstall = "http://tizen.org/privilege/packagemanager.install";
const std::string kPrivilegePackageInfo = "http://tizen.org/privilege/package.info";
} // namespace

using namespace common;
using namespace extension::package;

typedef enum _PackageThreadWorkType {
  PackageThreadWorkNone = 0, 
  PackageThreadWorkGetPackagesInfo, 
} PackageThreadWorkType;

class PackageUserData {
 public:
  PackageUserData(PackageInstance* ins, int id, PackageThreadWorkType task) {
    instance = ins;
    callbackId = id;
    work = task;
  }  
  
  PackageInstance* instance;
  int callbackId;  
  PackageThreadWorkType work;
  picojson::object data;
};
typedef std::shared_ptr<PackageUserData> PackageUserDataPtr;

static void* PackageThreadWork(const PackageUserDataPtr& userData) {
  LoggerD("Enter");

  PackageInstance* instance = userData->instance;

  switch(userData->work) {
    case PackageThreadWorkGetPackagesInfo: {
      LoggerD("Start PackageThreadWorkGetPackagesInfo");      
      picojson::object output;
      PackageInfoProvider::GetPackagesInfo(output);
      userData->data = output;
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
    }
  }

  return NULL;
}

static gboolean PackageAfterWork(const PackageUserDataPtr& userData) {
  LoggerD("Enter");
  
  userData->data["callbackId"] = picojson::value(static_cast<double>(userData->callbackId));  
  picojson::value result = picojson::value(userData->data);
  userData->instance->PostMessage(result.serialize().c_str());

  return FALSE;
}

static void PackageRequestCb(
    int id, const char *type, const char *package, package_manager_event_type_e event_type, 
    package_manager_event_state_e event_state, int progress, package_manager_error_e error, void *user_data) {
  LoggerD("Enter [%s]", package);

  PackageInstance* instance = static_cast<PackageInstance*>(user_data);
  if(!instance) {
    LoggerE("instance is NULL");
    return;
  }

  if(event_state == PACKAGE_MANAGER_EVENT_STATE_STARTED) {
    LoggerD("[Started] Do not invoke JS callback");
    return;
  }

  picojson::object param;
  if(event_state == PACKAGE_MANAGER_EVENT_STATE_FAILED) {
    LoggerD("[Failed]");
    param["status"] = picojson::value("error");
    param["error"] = UnknownException(
      "It is not allowed to install the package by the platform or any other platform error occurs").ToJSON();
  } else if(event_state == PACKAGE_MANAGER_EVENT_STATE_PROCESSING) {
    LoggerD("[Onprogress] %d %", progress);
    param["status"] = picojson::value("progress");
    param["progress"] = picojson::value(static_cast<double>(progress));
    param["id"] = picojson::value(std::string(package));
  } else if(event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
    LoggerD("[Oncomplete]");
    param["status"] = picojson::value("complete");
    param["id"] = picojson::value(std::string(package));
  }

  instance->InvokeCallback(id, param);
  if(event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
    LoggerD("Request has been completed");
    instance->DeregisterCallback(id);
  }
}

static void PackageListenerCb(
    const char *type, const char *package, package_manager_event_type_e event_type, 
    package_manager_event_state_e event_state, int progress, package_manager_error_e error, void *user_data) {
  LoggerD("Enter");

  PackageInstance* instance = static_cast<PackageInstance*>(user_data);
  if(!instance) {
    LoggerE("instance is NULL");
    return;
  }

  if(error != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Failed");
    return;    
  }

  picojson::object param;
  if(event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL && event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
    LoggerD("[Installed]");
    param["status"] = picojson::value("installed");
    picojson::object info;
    PackageInfoProvider::GetPackageInfo(package, info);
    param["info"] = picojson::value(info["result"]);
    instance->InvokeListener(param);
  } else if(event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL && event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
    LoggerD("[Uninstalled]");
    param["status"] = picojson::value("uninstalled");
    param["id"] = picojson::value(std::string(package));
    instance->InvokeListener(param);
  } else if(event_type == PACKAGE_MANAGER_EVENT_TYPE_UPDATE && event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED) {
    LoggerD("[Updated]");
    param["status"] = picojson::value("updated");
    picojson::object info;
    PackageInfoProvider::GetPackageInfo(package, info);
    param["info"] = picojson::value(info["result"]);
    instance->InvokeListener(param);
  }
}

PackageInstance::PackageInstance() {
  LoggerD("Enter");

  if(package_manager_request_create(&pRequest) != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Failed to created package manager request");
    pRequest = NULL;
  } 

  if(package_manager_request_set_event_cb(pRequest, PackageRequestCb, static_cast<void*>(this))
      != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Failed to set request event callback");
  }

  if(package_manager_create(&pManager) != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Failed to created package manager");
    pManager = NULL;
  }

  listenerId = -1;
  
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&PackageInstance::x, this, _1, _2));
  REGISTER_SYNC("PackageManager_setPackageInfoEventListener", PackageManagerSetpackageinfoeventlistener);
  REGISTER_SYNC("PackageManager_install", PackageManagerInstall);
  REGISTER_SYNC("PackageManager_getPackagesInfo", PackageManagerGetpackagesinfo);
  REGISTER_SYNC("PackageManager_uninstall", PackageManagerUninstall);
  REGISTER_SYNC("PackageManager_unsetPackageInfoEventListener", PackageManagerUnsetpackageinfoeventlistener);
  REGISTER_SYNC("PackageManager_getPackageInfo", PackageManagerGetpackageinfo);
  #undef REGISTER_SYNC
}

PackageInstance::~PackageInstance() {
  LoggerD("Enter");
  package_manager_request_unset_event_cb(pRequest);
  package_manager_request_destroy(pRequest);
  package_manager_destroy(pManager);
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void PackageInstance::RegisterCallback(int requestId, int callbackId) {
  LoggerD("Enter");
  callbacksMap[requestId] = callbackId;
}

void PackageInstance::DeregisterCallback(int requestId) {
  LoggerD("Enter [%d]", requestId);
  callbacksMap.erase(requestId);
}

void PackageInstance::InvokeCallback(int requestId, picojson::object& param) {
  LoggerD("Enter [%d]", requestId);

  int callbackId = callbacksMap[requestId];
  LoggerD("callbackId: %d", callbackId);

  param["callbackId"] = picojson::value(static_cast<double>(callbackId));  
  picojson::value result = picojson::value(param);
  PostMessage(result.serialize().c_str());
}

void PackageInstance::PackageManagerInstall(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "packageFileURI", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  LoggerD("callbackId: %d", callbackId);
  const std::string& packageFileURI = args.get("packageFileURI").get<std::string>();
  LoggerD("packageFileURI: %s", packageFileURI.c_str());
  
  // Need to check privilege
  // throw new SecurityException("This application does not have the privilege to call this method");

  if(!pRequest) {
    LoggerE("package_manager_request_h is NULL");
    InvokeErrorCallbackAsync(callbackId, 
      UnknownException("It is not allowed to install the package by the platform or any other platform error occurs"));
    return;
  }

  int requestId = 0;
  int ret = package_manager_request_install(pRequest, packageFileURI.c_str(), &requestId);
  if(ret != PACKAGE_MANAGER_ERROR_NONE) {
    if(ret == PACKAGE_MANAGER_ERROR_INVALID_PARAMETER) {
      LoggerE("The package is not found at the specified location");
      InvokeErrorCallbackAsync(callbackId, 
        NotFoundException("The package is not found at the specified location"));
    } else {
      LoggerE("It is not allowed to install the package by the platform or any other platform error occurs");
      InvokeErrorCallbackAsync(callbackId, 
        UnknownException("It is not allowed to install the package by the platform or any other platform error occurs"));
    }
  } else {
    RegisterCallback(requestId, callbackId);
  }
  
  ReportSuccess(out);
}

void PackageInstance::PackageManagerUninstall(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "id", out)
  
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  LoggerD("callbackId: %d", callbackId);
  const std::string& id = args.get("id").get<std::string>();
  LoggerD("id: %s", id.c_str());

  // Need to check privilege
  // throw new SecurityException("This application does not have the privilege to call this method");

  if(!pRequest) {
    LoggerE("package_manager_request_h is NULL");
    InvokeErrorCallbackAsync(callbackId, 
      UnknownException("It is not allowed to install the package by the platform or any other platform error occurs"));
    return;
  }

  int requestId = 0;
  int ret = package_manager_request_uninstall(pRequest, id.c_str(), &requestId);
  if(ret != PACKAGE_MANAGER_ERROR_NONE) {
    if(ret == PACKAGE_MANAGER_ERROR_INVALID_PARAMETER) {
      LoggerE("The package is not found at the specified location");
      InvokeErrorCallbackAsync(callbackId, 
        NotFoundException("The package is not found at the specified location"));
    } else {
      LoggerE("It is not allowed to install the package by the platform or any other platform error occurs");
      InvokeErrorCallbackAsync(callbackId, 
        UnknownException("It is not allowed to install the package by the platform or any other platform error occurs"));
    }
  } else {
    RegisterCallback(requestId, callbackId);
  }
  
  ReportSuccess(out);
}
void PackageInstance::PackageManagerGetpackagesinfo(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_EXIST(args, "callbackId", out)
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  LoggerD("callbackId: %d", callbackId);

  // Need to check privilege
  // throw new SecurityException("This application does not have the privilege to call this method");

  PackageUserDataPtr userData(new PackageUserData(this, callbackId, PackageThreadWorkGetPackagesInfo));
  TaskQueue::GetInstance().Queue<PackageUserData>(PackageThreadWork, PackageAfterWork, userData);
  ReportSuccess(out);
}
void PackageInstance::PackageManagerGetpackageinfo(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  // Need to check privilege
  // throw new SecurityException("This application does not have the privilege to call this method");

  if(args.contains("id")) {
    std::string id = args.get("id").get<std::string>();
    LoggerD("package id : [%s]", id.c_str());
    PackageInfoProvider::GetPackageInfo(id.c_str(), out);
  } else {
    PackageInfoProvider::GetPackageInfo(out);
  }
}

void PackageInstance::InvokeListener(picojson::object& param) {
  LoggerD("Enter");

  param["callbackId"] = picojson::value(static_cast<double>(listenerId));  
  picojson::value result = picojson::value(param);
  PostMessage(result.serialize().c_str());
}

void PackageInstance::PackageManagerSetpackageinfoeventlistener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  
  CHECK_EXIST(args, "callbackId", out)
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  LoggerD("callbackId: %d", callbackId);

  // Need to check privilege
  // throw new SecurityException("This application does not have the privilege to call this method");

  if(!pManager) {
    LoggerE("package_manager_h is NULL");
    throw new UnknownException(
      "The package list change event cannot be generated because of a platform error");
  }

  if(package_manager_set_event_cb(pManager, PackageListenerCb, static_cast<void*>(this))
      != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Failed to set event callback");
    throw new UnknownException(
      "The package list change event cannot be generated because of a platform error");    
  }

  listenerId = callbackId;  
  ReportSuccess(out);
}

void PackageInstance::PackageManagerUnsetpackageinfoeventlistener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  // Need to check privilege
  // throw new SecurityException("This application does not have the privilege to call this method");

  if(listenerId == -1) {
    LoggerD("Listener is not set");
    ReportSuccess(out);
    return;
  }

  if(!pManager) {
    LoggerE("package_manager_h is NULL");
    throw new UnknownException(
      "Tthe listener removal request fails because of a platform error");
  }

  if(package_manager_unset_event_cb(pManager)
      != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Failed to unset event callback");
    throw new UnknownException(
      "Tthe listener removal request fails because of a platform error");    
  }

  ReportSuccess(picojson::value(static_cast<double>(listenerId)), out);
  listenerId = -1;
}

void PackageInstance::InvokeErrorCallbackAsync(int callbackId, const PlatformException& ex) {
  LoggerD("Enter");

  picojson::object param;
  ReportError(ex, param);
  PackageUserDataPtr userData(new PackageUserData(this, callbackId, PackageThreadWorkNone));
  userData->data = param;
  TaskQueue::GetInstance().Async<PackageUserData>(PackageAfterWork, userData);
}

#undef CHECK_EXIST

} // namespace package
} // namespace extension
