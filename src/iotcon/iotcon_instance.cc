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

#include <thread>

#include "common/logger.h"

#include "iotcon/iotcon_utils.h"

namespace extension {
namespace iotcon {

namespace {

#define CHECK_EXIST(args, name) \
  if (args.end() == args.find(name)) { \
    return common::TypeMismatchError(std::string(name) + " is required argument"); \
  }

long long GetId(const picojson::object& args) {
  return static_cast<long long>(args.find(kId)->second.get<double>());
}

const picojson::value& GetArg(const picojson::object& args, const std::string& name) {
  static const picojson::value kNull;

  auto it = args.find(name);
  if (args.end() == it) {
    return kNull;
  } else {
    return it->second;
  }
}

const common::ListenerToken kResourceRequestListenerToken{"ResourceRequestListener"};

}  // namespace

IotconInstance::IotconInstance() : manager_(this) {
  ScopeLogger();

  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&IotconInstance::x, this, _1))

  REGISTER_SYNC("IotconResource_getObserverIds", ResourceGetObserverIds);
  REGISTER_SYNC("IotconResource_setRequestListener", ResourceSetRequestListener);
  REGISTER_SYNC("IotconResource_unsetRequestListener", ResourceUnsetRequestListener);
  REGISTER_SYNC("IotconResponse_send", ResponseSend);
  REGISTER_SYNC("IotconRemoteResource_getCachedRepresentation", RemoteResourceGetCachedRepresentation);
  REGISTER_SYNC("IotconRemoteResource_setStateChangeListener", RemoteResourceSetStateChangeListener);
  REGISTER_SYNC("IotconRemoteResource_unsetStateChangeListener", RemoteResourceUnsetStateChangeListener);
  REGISTER_SYNC("IotconRemoteResource_startCaching", RemoteResourceStartCaching);
  REGISTER_SYNC("IotconRemoteResource_stopCaching", RemoteResourceStopCaching);
  REGISTER_SYNC("IotconRemoteResource_setConnectionChangeListener", RemoteResourceSetConnectionChangeListener);
  REGISTER_SYNC("IotconRemoteResource_unsetConnectionChangeListener", RemoteResourceUnsetConnectionChangeListener);
  REGISTER_SYNC("IotconClient_addPresenceEventListener", ClientAddPresenceEventListener);
  REGISTER_SYNC("IotconClient_removePresenceEventListener", ClientRemovePresenceEventListener);
  REGISTER_SYNC("Iotcon_getTimeout", GetTimeout);
  REGISTER_SYNC("Iotcon_setTimeout", SetTimeout);
  REGISTER_SYNC("IotconServer_createResource", ServerCreateResource);
  REGISTER_SYNC("IotconServer_removeResource", ServerRemoveResource);

#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&IotconInstance::x, this, _1, _2));

  REGISTER_ASYNC("IotconResource_notify", ResourceNotify);
  REGISTER_ASYNC("IotconResource_addResourceTypes", ResourceAddResourceTypes);
  REGISTER_ASYNC("IotconResource_addResourceInterfaces", ResourceAddResourceInterfaces);
  REGISTER_ASYNC("IotconResource_addChildResource", ResourceAddChildResource);
  REGISTER_ASYNC("IotconResource_removeChildResource", ResourceRemoveChildResource);
  REGISTER_ASYNC("IotconRemoteResource_methodGet", RemoteResourceMethodGet);
  REGISTER_ASYNC("IotconRemoteResource_methodPut", RemoteResourceMethodPut);
  REGISTER_ASYNC("IotconRemoteResource_methodPost", RemoteResourceMethodPost);
  REGISTER_ASYNC("IotconRemoteResource_methodDelete", RemoteResourceMethodDelete);
  REGISTER_ASYNC("IotconClient_findResource", ClientFindResource);
  REGISTER_ASYNC("IotconClient_getDeviceInfo", ClientGetDeviceInfo);
  REGISTER_ASYNC("IotconClient_getPlatformInfo", ClientGetPlatformInfo);

#undef REGISTER_ASYNC

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
  ScopeLogger();

  if (!is_connected) {
    LoggerD("Connection lost, need to wait for connection recovery");
  } else {
    IotconInstance* instance = static_cast<IotconInstance*>(user_data);
    if (!instance) {
      LoggerE("instance is NULL");
      return;
    }

    LoggerD("Connection recovered, restoring handles");
    auto ret = instance->manager_.RestoreHandles();
    if (!ret) {
      LoggerD("Connection recovered, but restoring handles failed");
    }
  }
}

IotconInstance::~IotconInstance() {
  ScopeLogger();

  iotcon_remove_connection_changed_cb(ConnectionChangedCallback, this);
  iotcon_disconnect();
}

common::TizenResult IotconInstance::ResourceGetObserverIds(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ResourceNotify(const picojson::object& args,
                                                   const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ResourceAddResourceTypes(const picojson::object& args,
                                                             const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ResourceAddResourceInterfaces(const picojson::object& args,
                                                                  const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ResourceAddChildResource(const picojson::object& args,
                                                             const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ResourceRemoveChildResource(const picojson::object& args,
                                                                const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ResourceSetRequestListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);

  ResourceInfoPtr resource;
  long long id = GetId(args);
  auto result = manager_.GetResourceById(id, &resource);

  if (!result) {
    return result;
  }

  if (!resource->request_listener) {
    resource->request_listener = [this, id](const common::TizenResult&, const picojson::value& v) {
      picojson::value response{picojson::object{}};
      auto& obj = response.get<picojson::object>();

      obj.insert(std::make_pair(kId, picojson::value{static_cast<double>(id)}));
      obj.insert(std::make_pair("data", v));

      Post(kResourceRequestListenerToken, common::TizenSuccess{response});
    };
  }

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ResourceUnsetRequestListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);

  ResourceInfoPtr resource;
  auto result = manager_.GetResourceById(GetId(args), &resource);

  if (!result) {
    return result;
  }

  resource->request_listener = nullptr;

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ResponseSend(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceGetCachedRepresentation(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceMethodGet(const picojson::object& args,
                                                            const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceMethodPut(const picojson::object& args,
                                                            const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceMethodPost(const picojson::object& args,
                                                             const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceMethodDelete(const picojson::object& args,
                                                               const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceSetStateChangeListener(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceUnsetStateChangeListener(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceStartCaching(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceStopCaching(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceSetConnectionChangeListener(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::RemoteResourceUnsetConnectionChangeListener(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ClientFindResource(const picojson::object& args,
                                                       const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ClientAddPresenceEventListener(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ClientRemovePresenceEventListener(const picojson::object& args) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ClientGetDeviceInfo(const picojson::object& args,
                                                        const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ClientGetPlatformInfo(const picojson::object& args,
                                                          const common::AsyncToken& token) {
  ScopeLogger();
  return common::UnknownError("Not implemented");
}

common::TizenResult IotconInstance::ServerCreateResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kUriPath);

  const std::string& uri_path = args.find(kUriPath)->second.get<std::string>();

  const auto& interfaces = GetArg(args, kResourceInterfaces);
  const auto& resource_interfaces = interfaces.is<picojson::array>() ? interfaces.get<picojson::array>() : picojson::array();

  const auto& types = GetArg(args, kResourceTypes);
  const auto& resource_types = types.is<picojson::array>() ? types.get<picojson::array>() : picojson::array();

  int properties = IOTCON_RESOURCE_NO_PROPERTY;

  const auto& observable = GetArg(args, kIsObservable);
  properties |= (observable.is<bool>() ? observable.get<bool>() : false) ? IOTCON_RESOURCE_OBSERVABLE : IOTCON_RESOURCE_NO_PROPERTY;

  const auto& discoverable = GetArg(args, kIsDiscoverable);
  properties |= (discoverable.is<bool>() ? discoverable.get<bool>() : false) ? IOTCON_RESOURCE_DISCOVERABLE : IOTCON_RESOURCE_NO_PROPERTY;

  const auto& active = GetArg(args, kIsActive);
  properties |= (active.is<bool>() ? active.get<bool>() : false) ? IOTCON_RESOURCE_ACTIVE : IOTCON_RESOURCE_NO_PROPERTY;

  const auto& slow = GetArg(args, kIsSlow);
  properties |= (slow.is<bool>() ? slow.get<bool>() : false) ? IOTCON_RESOURCE_SLOW : IOTCON_RESOURCE_NO_PROPERTY;

  const auto& secure = GetArg(args, kIsSecure);
  properties |= (secure.is<bool>() ? secure.get<bool>() : false) ? IOTCON_RESOURCE_SECURE : IOTCON_RESOURCE_NO_PROPERTY;

  const auto& explicit_discoverable = GetArg(args, kIsExplicitDiscoverable);
  properties |= (explicit_discoverable.is<bool>() ? explicit_discoverable.get<bool>() : false) ? IOTCON_RESOURCE_EXPLICIT_DISCOVERABLE : IOTCON_RESOURCE_NO_PROPERTY;

  ResourceInfoPtr resource{new ResourceInfo()};
  auto ret = manager_.CreateResource(uri_path, resource_interfaces, resource_types,
                                     properties, resource);
  if (!ret) {
    return ret;
  }

  LoggerD("RESOURCE\nid: %lld\nhandle: %p", resource->id, resource->handle);

  picojson::value result = picojson::value(picojson::object());
  ret = IotconUtils::ResourceToJson(resource, manager_, &(result.get<picojson::object>()));
  if (!ret) {
    return ret;
  }

  return common::TizenSuccess{result};
}

common::TizenResult IotconInstance::ServerRemoveResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);

  return manager_.DestroyResource(GetId(args));
}

common::TizenResult IotconInstance::GetTimeout(const picojson::object& args) {
  ScopeLogger();

  int timeout = 0;
  auto result = IotconUtils::ConvertIotconError(iotcon_get_timeout(&timeout));

  if (!result) {
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess{picojson::value{static_cast<double>(timeout)}};
}

common::TizenResult IotconInstance::SetTimeout(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, "timeout");

  int timeout = static_cast<int>(args.find("timeout")->second.get<double>());
  auto result = IotconUtils::ConvertIotconError(iotcon_set_timeout(timeout));

  if (!result) {
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess();
}

}  // namespace iotcon
}  // namespace extension
