/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "iotcon/iotcon_instance.h"

#include "common/logger.h"
#include "common/task-queue.h"
#include "common/tools.h"

#include "iotcon/iotcon_utils.h"

using common::PlatformResult;
using common::ErrorCode;
using common::TypeMismatchException;
using common::tools::ReportError;
using common::tools::ReportSuccess;
using common::Instance;
using common::TaskQueue;

namespace extension {
namespace iotcon {

namespace {
#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    std::string message = std::string(name) + " is required argument";\
    common::tools::ReportError(\
        common::PlatformResult(common::ErrorCode::TYPE_MISMATCH_ERR, message), &out);\
    return;\
  }
}

IotconInstance::IotconInstance() : manager_(this) {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_ASYNC(c, func) \
      RegisterSyncHandler(c, func);
  #define REGISTER_SYNC(c, func) \
      RegisterSyncHandler(c, func);

  REGISTER_SYNC("IotconResource_getObserverIds",
      std::bind(&IotconInstance::IotconResourceGetObserverIds, this, _1, _2));
  REGISTER_ASYNC("IotconResource_notify",
      std::bind(&IotconInstance::IotconResourceNotify, this, _1, _2));
  REGISTER_ASYNC("IotconResource_addResourceTypes",
      std::bind(&IotconInstance::IotconResourceAddResourceTypes, this, _1, _2));
  REGISTER_ASYNC("IotconResource_addResourceInterfaces",
      std::bind(&IotconInstance::IotconResourceAddResourceInterfaces, this, _1, _2));
  REGISTER_ASYNC("IotconResource_addChildResource",
      std::bind(&IotconInstance::IotconResourceAddChildResource, this, _1, _2));
  REGISTER_ASYNC("IotconResource_removeChildResource",
      std::bind(&IotconInstance::IotconResourceRemoveChildResource, this, _1, _2));
  REGISTER_SYNC("IotconResource_setRequestListener",
      std::bind(&IotconInstance::IotconResourceSetRequestListener, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_unsetRequestListener",
      std::bind(&IotconInstance::IotconRemoteResourceUnsetRequestListener, this, _1, _2));
  REGISTER_SYNC("IotconResponse_send",
      std::bind(&IotconInstance::IotconResponseSend, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_getCachedRepresentation",
      std::bind(&IotconInstance::IotconRemoteResourceGetCachedRepresentation, this, _1, _2));
  REGISTER_ASYNC("IotconRemoteResource_methodGet",
      std::bind(&IotconInstance::IotconRemoteResourceMethodGet, this, _1, _2));
  REGISTER_ASYNC("IotconRemoteResource_methodPut",
      std::bind(&IotconInstance::IotconRemoteResourceMethodPut, this, _1, _2));
  REGISTER_ASYNC("IotconRemoteResource_methodPost",
      std::bind(&IotconInstance::IotconRemoteResourceMethodPost, this, _1, _2));
  REGISTER_ASYNC("IotconRemoteResource_methodDelete",
      std::bind(&IotconInstance::IotconRemoteResourceMethodDelete, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_setStateChangeListener",
      std::bind(&IotconInstance::IotconRemoteResourceSetStateChangeListener, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_unsetStateChangeListener",
      std::bind(&IotconInstance::IotconRemoteResourceUnsetStateChangeListener, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_startCaching",
      std::bind(&IotconInstance::IotconRemoteResourceStartCaching, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_stopCaching",
      std::bind(&IotconInstance::IotconRemoteResourceStopCaching, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_setConnectionChangeListener",
      std::bind(&IotconInstance::IotconRemoteResourceSetConnectionChangeListener, this, _1, _2));
  REGISTER_SYNC("IotconRemoteResource_unsetConnectionChangeListener",
      std::bind(&IotconInstance::IotconRemoteResourceUnsetConnectionChangeListener, this, _1, _2));
  REGISTER_ASYNC("IotconClient_findResource",
      std::bind(&IotconInstance::IotconClientFindResource, this, _1, _2));
  REGISTER_SYNC("IotconClient_addPresenceEventListener",
      std::bind(&IotconInstance::IotconClientAddPresenceEventListener, this, _1, _2));
  REGISTER_SYNC("IotconClient_removePresenceEventListener",
      std::bind(&IotconInstance::IotconClientRemovePresenceEventListener, this, _1, _2));
  REGISTER_ASYNC("IotconClient_getDeviceInfo",
      std::bind(&IotconInstance::IotconClientGetDeviceInfo, this, _1, _2));
  REGISTER_ASYNC("IotconClient_getPlatformInfo",
      std::bind(&IotconInstance::IotconClientGetPlatformInfo, this, _1, _2));
  REGISTER_ASYNC("IotconServer_createResource",
      std::bind(&IotconInstance::IotconServerCreateResource, this, _1, _2));
  REGISTER_ASYNC("IotconServer_removeResource",
      std::bind(&IotconInstance::IotconServerRemoveResource, this, _1, _2));
  REGISTER_ASYNC("IotconServer_updateResource",
      std::bind(&IotconInstance::IotconServerUpdateResource, this, _1, _2));
  REGISTER_SYNC("Iotcon_getTimeout",
      std::bind(&IotconInstance::IotconGetTimeout, this, _1, _2));
  REGISTER_SYNC("Iotcon_setTimeout",
      std::bind(&IotconInstance::IotconSetTimeout, this, _1, _2));
  #undef REGISTER_ASYNC
  #undef REGISTER_SYNC

  // initialize connection to iotcon service
  int ret = iotcon_connect();
  if (IOTCON_ERROR_NONE != ret) {
    LoggerE("Could not connnect to iotcon service: %s", get_error_message(ret));
  } else {
    LoggerD("Iotcon service connected");
    ret = iotcon_add_connection_changed_cb(ConnectionChangedCallback, this);
    if (IOTCON_ERROR_NONE != ret) {
      LoggerE("Could not add connection changed callback for iotcon service: %s",
              get_error_message(ret));
    } else {
      LoggerD("Iotcon connection changed callback is registered");
    }
  }
}

void IotconInstance::ConnectionChangedCallback(bool is_connected, void* user_data) {
  LoggerD("Enter");
  if (!is_connected) {
    LoggerD("Connection lost, need to wait for connection recovery");
  } else {
    IotconInstance* instance = static_cast<IotconInstance*>(user_data);
    if (!instance) {
      LoggerE("instance is NULL");
      return;
    }

    LoggerD("Connection recovered, restoring handles");
    PlatformResult ret = instance->manager_.RestoreHandles();
    if (ret.IsError()) {
      LoggerD("Connection recovered, but restoring handles failed");
    }
  }
}

IotconInstance::~IotconInstance() {
  LoggerD("Enter");

  iotcon_remove_connection_changed_cb(ConnectionChangedCallback, this);
  iotcon_disconnect();
}

void IotconInstance::IotconResourceGetObserverIds(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResourceNotify(const picojson::value& args,
                                          picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResourceAddResourceTypes(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResourceAddResourceInterfaces(const picojson::value& args,
                                                         picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResourceAddChildResource(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResourceRemoveChildResource(const picojson::value& args,
                                                       picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResourceSetRequestListener(const picojson::value& args,
                                                      picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceUnsetRequestListener(const picojson::value& args,
                                                              picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconResponseSend(const picojson::value& args,
                                        picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceGetCachedRepresentation(const picojson::value& args,
                                                                 picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceMethodGet(const picojson::value& args,
                                                   picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceMethodPut(const picojson::value& args,
                                                   picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceMethodPost(const picojson::value& args,
                                                    picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceMethodDelete(const picojson::value& args,
                                                      picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceSetStateChangeListener(const picojson::value& args,
                                                                picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceUnsetStateChangeListener(const picojson::value& args,
                                                                  picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceStartCaching(const picojson::value& args,
                                                      picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceStopCaching(const picojson::value& args,
                                                     picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceSetConnectionChangeListener(const picojson::value& args,
                                                                     picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconRemoteResourceUnsetConnectionChangeListener(const picojson::value& args,
                                                                       picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconClientFindResource(const picojson::value& args,
                                              picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconClientAddPresenceEventListener(const picojson::value& args,
                                                          picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconClientRemovePresenceEventListener(const picojson::value& args,
                                                             picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconClientGetDeviceInfo(const picojson::value& args,
                                               picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconClientGetPlatformInfo(const picojson::value& args,
                                                 picojson::object& out) {
  LoggerD("Enter");

}

void IotconInstance::IotconServerCreateResource(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Enter");
  LoggerD("args: %s", args.serialize().c_str());
  CHECK_EXIST(args, kCallbackId, out)
  CHECK_EXIST(args, kIsDiscoverable, out)
  CHECK_EXIST(args, kIsObservable, out)
  CHECK_EXIST(args, kResourceTypes, out)
  CHECK_EXIST(args, kResourceInterfaces, out)
  CHECK_EXIST(args, kUriPath, out)

  const double callback_id = args.get(kCallbackId).get<double>();
  // TODO consider support other properties
  const bool is_discoverable = args.get(kIsDiscoverable).get<bool>();
  const bool is_observable = args.get(kIsObservable).get<bool>();
  const auto& resource_types = args.get(kResourceTypes).get<picojson::array>();
  const auto& resource_interfaces = args.get(kResourceInterfaces).get<picojson::array>();
  const std::string& uri_path = args.get(kUriPath).get<std::string>();

  auto create = [this, callback_id, is_discoverable, is_observable,
                 resource_types, uri_path, resource_interfaces]
                 (const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Create resource");

    picojson::value result = picojson::value(picojson::object());
    ResourceInfoPtr resource{new ResourceInfo()};
    PlatformResult ret = manager_.CreateResource(uri_path, resource_interfaces, resource_types,
                                                 is_discoverable, is_observable, resource);
    if (ret.IsError()) {
      LogAndReportError(ret,&(response->get<picojson::object>()));
      return;
    }
    LoggerD("RESOURCE\nid: %lld\nhandle: %p", resource->id, resource->handle);

    ret = IotconUtils::ResourceToJson(resource, manager_, &(result.get<picojson::object>()));
    if (ret.IsError()) {
      LogAndReportError(ret,&(response->get<picojson::object>()));
      return;
    }
    ReportSuccess(result, response->get<picojson::object>());
  };

  auto create_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value{static_cast<double>(callback_id)}));
    LoggerD("message: %s", response->serialize().c_str());
    Instance::PostMessage(this, response->serialize().c_str());
  };

  auto data = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

  TaskQueue::GetInstance().Queue<picojson::value>(create, create_response, data);
}

void IotconInstance::IotconServerRemoveResource(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Enter");
}

void IotconInstance::IotconServerUpdateResource(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Enter");
}

void IotconInstance::IotconGetTimeout(const picojson::value& args,
                                      picojson::object& out) {
  LoggerD("Enter");
}

void IotconInstance::IotconSetTimeout(const picojson::value& args,
                                      picojson::object& out) {
  LoggerD("Enter");
}

}  // namespace iotcon
}  // namespace extension
