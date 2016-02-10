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
#include "common/scope_exit.h"

#include "iotcon/iotcon_utils.h"

namespace extension {
namespace iotcon {

namespace {

typedef struct {
  common::PostCallback fun;
} CallbackData;

const std::string kTimeout = "timeout";

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

const std::string kObserverIds = "observerIds";
const std::string kQos = "qos";
const std::string kChildId = "childId";

}  // namespace

IotconInstance::IotconInstance() {
  ScopeLogger();

  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&IotconInstance::x, this, _1))

  REGISTER_SYNC("IotconResource_getObserverIds", ResourceGetObserverIds);
  REGISTER_SYNC("IotconResource_notify", ResourceNotify);
  REGISTER_SYNC("IotconResource_addChildResource", ResourceAddChildResource);
  REGISTER_SYNC("IotconResource_removeChildResource", ResourceRemoveChildResource);
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

  REGISTER_ASYNC("IotconResource_addResourceTypes", ResourceAddResourceTypes);
  REGISTER_ASYNC("IotconResource_addResourceInterfaces", ResourceAddResourceInterfaces);
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
    auto ret = IotconServerManager::GetInstance().RestoreHandles();
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

common::TizenResult IotconInstance::ResourceNotify(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kQos);
  CHECK_EXIST(args, kStates);

  ResourceInfoPtr resource;
  long long id = GetId(args);
  auto result = IotconServerManager::GetInstance().GetResourceById(id, &resource);

  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  auto& qos = GetArg(args, kQos);
  if (!qos.is<std::string>()) {
    return common::TypeMismatchError("QOS needs to be a string");
  }

  // create observers to notify
  auto& observer_ids = GetArg(args, kObserverIds);

  std::vector<int> observers;

  if (observer_ids.is<picojson::array>()) {
    // use provided list, make sure that observer exists
    for (auto& id : observer_ids.get<picojson::array>()) {
      if (id.is<double>()) {
        auto v = static_cast<int>(id.get<double>());
        if (resource->observers.end() != resource->observers.find(v)) {
          observers.push_back(v);
        }
      }
    }
  } else {
    // use own list
    observers.assign(resource->observers.begin(), resource->observers.end());
  }

  // create & initialize platform object
  iotcon_observers_h observers_handle = nullptr;
  result = IotconUtils::ConvertIotconError(iotcon_observers_create(&observers_handle));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_observers_create() failed"));
  }

  SCOPE_EXIT {
    iotcon_observers_destroy(observers_handle);
  };

  for (auto& id : observers) {
    result = IotconUtils::ConvertIotconError(iotcon_observers_add(observers_handle, id));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_observers_add() failed"));
    }
  }

  // create representation from resource and states
  iotcon_representation_h representation = nullptr;

  result = IotconUtils::RepresentationFromResource(resource, GetArg(args, kStates), &representation);
  if (!result) {
    LogAndReturnTizenError(result, ("RepresentationFromResource() failed"));
  }

  SCOPE_EXIT {
    iotcon_representation_destroy(representation);
  };

  result = IotconUtils::ConvertIotconError(iotcon_resource_notify(resource->handle, representation, observers_handle, IotconUtils::ToQos(qos.get<std::string>())));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_notify() failed"));
  }

  return common::TizenSuccess();
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

common::TizenResult IotconInstance::ResourceAddChildResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kChildId);

  ResourceInfoPtr parent;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &parent);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() parent failed"));
  }

  long long child_id = static_cast<long long>(GetArg(args, kChildId).get<double>());
  ResourceInfoPtr child;

  result = IotconServerManager::GetInstance().GetResourceById(child_id, &child);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  result = IotconUtils::ConvertIotconError(iotcon_resource_bind_child_resource(parent->handle, child->handle));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_bind_child_resource() failed"));
  }

  parent->children.insert(child);
  child->parents.insert(parent);

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ResourceRemoveChildResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kChildId);

  ResourceInfoPtr parent;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &parent);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() parent failed"));
  }

  long long child_id = static_cast<long long>(GetArg(args, kChildId).get<double>());
  ResourceInfoPtr child;

  result = IotconServerManager::GetInstance().GetResourceById(child_id, &child);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  result = IotconUtils::ConvertIotconError(iotcon_resource_unbind_child_resource(parent->handle, child->handle));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_unbind_child_resource() failed"));
  }

  parent->children.erase(child);
  child->parents.erase(parent);

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ResourceSetRequestListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);

  ResourceInfoPtr resource;
  long long id = GetId(args);
  auto result = IotconServerManager::GetInstance().GetResourceById(id, &resource);

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
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &resource);

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

void IotconPDeviceInfoCb(iotcon_device_info_h device_info,
                          iotcon_error_e result, void *user_data) {
  ScopeLogger();

  CallbackData* data = static_cast<CallbackData*>(user_data);
  picojson::value v{picojson::object{}};
  common::TizenResult ret = common::TizenSuccess();

  if (IOTCON_ERROR_NONE != result) {
    ret = IotconUtils::ConvertIotconError(result);
  } else {
    auto ret = IotconUtils::DeviceInfoToJson(device_info,&v.get<picojson::object>());
  }

  data->fun(ret, v);
  delete data;
}

common::TizenResult IotconInstance::ClientGetDeviceInfo(const picojson::object& args,
                                                        const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_EXIST(args, kHostAddress);
  CHECK_EXIST(args, kConnectivityType);

  std::string host = args.find(kHostAddress)->second.get<std::string>();
  std::string con_type = args.find(kConnectivityType)->second.get<std::string>();
  iotcon_connectivity_type_e con_type_e = IotconUtils::ToConnectivityType(con_type);

  CallbackData* data = new CallbackData{SimplePost(token)};

  auto result = IotconUtils::ConvertIotconError(
       iotcon_get_device_info(host.c_str(), con_type_e, IotconPDeviceInfoCb,
                                data));

  if (!result) {
    delete data;
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess();
}

void IotconPlatformInfoCb(iotcon_platform_info_h platform_info,
                          iotcon_error_e result, void *user_data) {
  ScopeLogger();

  CallbackData* data = static_cast<CallbackData*>(user_data);
  picojson::value v{picojson::object{}};
  common::TizenResult ret = common::TizenSuccess();

  if (IOTCON_ERROR_NONE != result) {
    ret = IotconUtils::ConvertIotconError(result);
  } else {
    ret = IotconUtils::PlatformInfoToJson(platform_info,&v.get<picojson::object>());
  }

  data->fun(ret, v);
  delete data;
}

common::TizenResult IotconInstance::ClientGetPlatformInfo(const picojson::object& args,
                                                          const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_EXIST(args, kHostAddress);
  CHECK_EXIST(args, kConnectivityType);

  std::string host = args.find(kHostAddress)->second.get<std::string>();
  std::string con_type = args.find(kConnectivityType)->second.get<std::string>();
  iotcon_connectivity_type_e con_type_e = IotconUtils::ToConnectivityType(con_type);

  CallbackData* data = new CallbackData{SimplePost(token)};

  auto result = IotconUtils::ConvertIotconError(
       iotcon_get_platform_info(host.c_str(), con_type_e, IotconPlatformInfoCb,
                                data));

  if (!result) {
    delete data;
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess();
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
  auto ret = IotconServerManager::GetInstance().CreateResource(uri_path, resource_interfaces, resource_types,
                                                               properties, resource);
  if (!ret) {
    return ret;
  }

  LoggerD("RESOURCE\nid: %lld\nhandle: %p", resource->id, resource->handle);

  picojson::value result = picojson::value(picojson::object());
  ret = IotconUtils::ResourceToJson(resource, &(result.get<picojson::object>()));
  if (!ret) {
    return ret;
  }

  return common::TizenSuccess{result};
}

common::TizenResult IotconInstance::ServerRemoveResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);

  return IotconServerManager::GetInstance().DestroyResource(GetId(args));
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

  CHECK_EXIST(args, kTimeout);

  int timeout = static_cast<int>(args.find(kTimeout)->second.get<double>());
  auto result = IotconUtils::ConvertIotconError(iotcon_set_timeout(timeout));

  if (!result) {
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess();
}

}  // namespace iotcon
}  // namespace extension
