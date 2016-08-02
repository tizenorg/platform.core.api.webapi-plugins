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
#include "common/tools.h"

#include "iotcon/iotcon_utils.h"
#include "iotcon/iotcon_server_manager.h"
#include "iotcon/iotcon_client_manager.h"

namespace extension {
namespace iotcon {

namespace {
const std::string kPrivilegeIotcon = "http://tizen.org/privilege/d2d.datasharing";

struct CallbackData {
  common::PostCallback fun;
};

long long GetId(const picojson::object& args) {
  return static_cast<long long>(args.find(kId)->second.get<double>());
}

void RemoteResourceResponseCallback(iotcon_remote_resource_h resource,
                                    iotcon_error_e err,
                                    iotcon_request_type_e request_type,
                                    iotcon_response_h response,
                                    void* user_data) {
  ScopeLogger();

  std::unique_ptr<CallbackData> data{static_cast<CallbackData*>(user_data)};

  if (data) {
    picojson::value value{picojson::object{}};
    auto result = IotconUtils::ConvertIotconError(err);

    if (result) {
      result = IotconUtils::ResponseToJson(response, &value.get<picojson::object>());
      if (!result) {
        LoggerE("ResponseToJson() failed");
      }
    } else {
      LoggerE("RemoteResourceResponseCallback() reports error");
    }

    data->fun(result, value);
  } else {
    LoggerE("Native callback data is null");
  }
}

const common::ListenerToken kResourceRequestListenerToken{"ResourceRequestListener"};
const common::ListenerToken kFindResourceListenerToken{"FindResourceListener"};
const common::ListenerToken kPresenceEventListenerToken{"PresenceEventListener"};
const common::ListenerToken kRemoteResourceConnectionChangeListener
                                {"RemoteResourceConnectionChangeListener"};
const common::ListenerToken kRemoteResourceStateChangeListener
                                {"RemoteResourceStateChangeListener"};

const std::string kObserverIds = "observerIds";
const std::string kQos = "qos";
const std::string kChildId = "childId";
const std::string kTypes = "types";
const std::string kInterface = "iface";
const std::string kResult = "result";
const std::string kTimeout = "timeout";
const std::string kData = "data";

const std::string kVirtualResourcesHandlingPath = "/home/owner/share/tmp_file_iotcon.dat";

}  // namespace

IotconInstance::IotconInstance() {
  ScopeLogger();

  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&IotconInstance::x, this, _1))

  REGISTER_SYNC("IotconResource_getObserverIds", ResourceGetObserverIds);
  REGISTER_SYNC("IotconResource_notify", ResourceNotify);
  REGISTER_SYNC("IotconResource_addResourceTypes", ResourceAddResourceTypes);
  REGISTER_SYNC("IotconResource_addResourceInterface", ResourceAddResourceInterface);
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
  REGISTER_SYNC("IotconClient_findResource", ClientFindResource);

#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&IotconInstance::x, this, _1, _2));

  REGISTER_ASYNC("IotconRemoteResource_methodGet", RemoteResourceMethodGet);
  REGISTER_ASYNC("IotconRemoteResource_methodPut", RemoteResourceMethodPut);
  REGISTER_ASYNC("IotconRemoteResource_methodPost", RemoteResourceMethodPost);
  REGISTER_ASYNC("IotconRemoteResource_methodDelete", RemoteResourceMethodDelete);
  REGISTER_ASYNC("IotconClient_getDeviceInfo", ClientGetDeviceInfo);
  REGISTER_ASYNC("IotconClient_getPlatformInfo", ClientGetPlatformInfo);

#undef REGISTER_ASYNC

  // initialize connection to iotcon service
  int ret = iotcon_initialize(kVirtualResourcesHandlingPath.c_str());
  if (IOTCON_ERROR_NONE != ret) {
    LoggerE("Could not connnect to iotcon service: %s", get_error_message(ret));
  } else {
    LoggerD("Iotcon service connected");

    ret = iotcon_start_presence(0);
    if (IOTCON_ERROR_NONE != ret) {
      LoggerE("Could not start presence: %s",
                  get_error_message(ret));
    } else {
      LoggerD("Iotcon iotcon_start_presence");
    }
  }
}

IotconInstance::~IotconInstance() {
  ScopeLogger();

  iotcon_stop_presence();
  iotcon_deinitialize();
}

common::TizenResult IotconInstance::ResourceGetObserverIds(const picojson::object& args) {
  ScopeLogger();

  CHECK_EXIST(args, kId);

  ResourceInfoPtr resource;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &resource);

  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  picojson::value value{picojson::array{}};
  auto& arr = value.get<picojson::array>();

  for (auto id : resource->observers) {
    arr.push_back(picojson::value{static_cast<double>(id)});
  }

  return common::TizenSuccess(value);
}

common::TizenResult IotconInstance::ResourceNotify(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kQos);
  CHECK_EXIST(args, kStates);

  ResourceInfoPtr resource;
  long long id = GetId(args);
  auto result = IotconServerManager::GetInstance().GetResourceById(id, &resource);

  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  auto& qos = IotconUtils::GetArg(args, kQos);
  if (!qos.is<std::string>()) {
    return common::TypeMismatchError("QOS needs to be a string");
  }

  // create observers to notify
  auto& observer_ids = IotconUtils::GetArg(args, kObserverIds);

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

  result = IotconUtils::RepresentationFromResource(resource, IotconUtils::GetArg(args, kStates), &representation);
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

common::TizenResult IotconInstance::ResourceAddResourceTypes(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kTypes);

  ResourceInfoPtr resource;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  const auto& types = IotconUtils::GetArg(args, kTypes).get<picojson::array>();

  for (const auto& type : types) {
    result = IotconUtils::ConvertIotconError(iotcon_resource_bind_type(resource->handle, type.get<std::string>().c_str()));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_bind_type() failed"));
    }
  }

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ResourceAddResourceInterface(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kInterface);

  ResourceInfoPtr resource;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  result = IotconUtils::ConvertIotconError(iotcon_resource_bind_interface(resource->handle, IotconUtils::GetArg(args, kInterface).get<std::string>().c_str()));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_bind_interface() failed"));
  }

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ResourceAddChildResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kChildId);

  ResourceInfoPtr parent;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &parent);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() parent failed"));
  }

  long long child_id = static_cast<long long>(IotconUtils::GetArg(args, kChildId).get<double>());
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

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kChildId);

  ResourceInfoPtr parent;
  auto result = IotconServerManager::GetInstance().GetResourceById(GetId(args), &parent);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() parent failed"));
  }

  long long child_id = static_cast<long long>(IotconUtils::GetArg(args, kChildId).get<double>());
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
      obj.insert(std::make_pair(kData, v));

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

  CHECK_EXIST(args, kId);
  CHECK_EXIST(args, kResult);
  CHECK_EXIST(args, kRepresentation);
  CHECK_EXIST(args, kOptions);
  CHECK_EXIST(args, kInterface);

  ResponsePtr response = nullptr;
  auto result = IotconServerManager::GetInstance().GetResponseById(GetId(args), &response);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResponseById() failed"));
  }

  {
    const auto& js_response_result = IotconUtils::GetArg(args, kResult);
    if (!js_response_result.is<std::string>()) {
      return LogAndCreateTizenError(TypeMismatchError, "ResponseResult should be a string");
    }
    iotcon_response_result_e response_result = IotconUtils::ToResponseResult(js_response_result.get<std::string>());

    result = IotconUtils::ConvertIotconError(iotcon_response_set_result(response.get(), response_result));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_result() failed"));
    }
  }

  {
    const auto& js_representation = IotconUtils::GetArg(args, kRepresentation);
    if (!js_representation.is<picojson::object>()) {
      return LogAndCreateTizenError(TypeMismatchError, "Representation should be an object");
    }
    iotcon_representation_h representation = nullptr;
    result = IotconUtils::RepresentationFromJson(js_representation.get<picojson::object>(), &representation);
    if (!result) {
      LogAndReturnTizenError(result, ("RepresentationFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_representation_destroy(representation);
    };

    result = IotconUtils::ConvertIotconError(
        iotcon_response_set_representation(
            response.get(),
            representation));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_representation() failed"));
    }
  }

  {
    const auto& js_options = IotconUtils::GetArg(args, kOptions);

    if (js_options.is<picojson::array>()) {
      iotcon_options_h options = nullptr;

      result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
      if (!result) {
        LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
      }
      SCOPE_EXIT {
        iotcon_options_destroy(options);
      };

      result = IotconUtils::ConvertIotconError(iotcon_response_set_options(response.get(), options));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
      }
    }
  }

  result = IotconUtils::ConvertIotconError(iotcon_response_send(response.get()));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_response_send() failed"));
  }

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::RemoteResourceGetCachedRepresentation(const picojson::object& args) {
  ScopeLogger();

  FoundRemoteInfoPtr ptr;
  auto res = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!res) {
    LogAndReturnTizenError(res, ("Failed to build resource using json data"));
  }
  iotcon_representation_h representation = nullptr;
  res = IotconUtils::ConvertIotconError(
      iotcon_remote_resource_get_cached_representation(ptr->handle, &representation));
  if (!res) {
    LogAndReturnTizenError(res, ("Gathering cached representation failed"));
  }
  if (representation) {
    picojson::value repr_json{picojson::object{}};
    res = IotconUtils::RepresentationToJson(representation, &repr_json.get<picojson::object>());
    if (!res) {
      LogAndReturnTizenError(res, ("RepresentationToJson() failed"));
    }
    return common::TizenSuccess{repr_json};
  }
  return common::UnknownError("Failed to gather cached representation");
}

common::TizenResult IotconInstance::RemoteResourceMethodGet(const picojson::object& args,
                                                            const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kQuery);

  FoundRemoteInfoPtr resource;
  auto result = IotconUtils::RemoteResourceFromJson(args, &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("RemoteResourceFromJson() failed"));
  }

  iotcon_query_h query = nullptr;
  result = IotconUtils::QueryFromJson(IotconUtils::GetArg(args, kQuery).get<picojson::object>(), &query);
  if (!result) {
    LogAndReturnTizenError(result, ("QueryFromJson() failed"));
  }
  SCOPE_EXIT {
    iotcon_query_destroy(query);
  };

  std::unique_ptr<CallbackData> data{new CallbackData{PostForMethodCall(token, resource)}};

  // set options to the remote resource
  const auto& js_options = IotconUtils::GetArg(args, kOptions);

  if (js_options.is<picojson::array>()) {
    iotcon_options_h options = nullptr;

    result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
    if (!result) {
      LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_options_destroy(options);
    };

    result = IotconUtils::ConvertIotconError(iotcon_remote_resource_set_options(resource->handle, options));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
    }
  }

  result = IotconUtils::ConvertIotconError(iotcon_remote_resource_get(resource->handle, query, RemoteResourceResponseCallback, data.get()));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_remote_resource_get() failed"));
  }

  // release memory ownership
  data.release();

  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(resource)};
}

common::TizenResult IotconInstance::RemoteResourceMethodPut(const picojson::object& args,
                                                            const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kRepresentation);
  CHECK_EXIST(args, kQuery);

  FoundRemoteInfoPtr resource;
  auto result = IotconUtils::RemoteResourceFromJson(args, &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("RemoteResourceFromJson() failed"));
  }

  iotcon_representation_h representation = nullptr;
  result = IotconUtils::RepresentationFromJson(IotconUtils::GetArg(args, kRepresentation).get<picojson::object>(), &representation);
  if (!result) {
    LogAndReturnTizenError(result, ("RepresentationFromJson() failed"));
  }
  SCOPE_EXIT {
    iotcon_representation_destroy(representation);
  };

  iotcon_query_h query = nullptr;
  result = IotconUtils::QueryFromJson(IotconUtils::GetArg(args, kQuery).get<picojson::object>(), &query);
  if (!result) {
    LogAndReturnTizenError(result, ("QueryFromJson() failed"));
  }
  SCOPE_EXIT {
    iotcon_query_destroy(query);
  };

  std::unique_ptr<CallbackData> data{new CallbackData{PostForMethodCall(token, resource)}};

  // set options to the remote resource
  const auto& js_options = IotconUtils::GetArg(args, kOptions);

  if (js_options.is<picojson::array>()) {
    iotcon_options_h options = nullptr;

    result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
    if (!result) {
      LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_options_destroy(options);
    };

    result = IotconUtils::ConvertIotconError(iotcon_remote_resource_set_options(resource->handle, options));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
    }
  }

  result = IotconUtils::ConvertIotconError(iotcon_remote_resource_put(resource->handle, representation, query, RemoteResourceResponseCallback, data.get()));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_remote_resource_put() failed"));
  }

  // release memory ownership
  data.release();

  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(resource)};
}

common::TizenResult IotconInstance::RemoteResourceMethodPost(const picojson::object& args,
                                                             const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kRepresentation);
  CHECK_EXIST(args, kQuery);

  FoundRemoteInfoPtr resource;
  auto result = IotconUtils::RemoteResourceFromJson(args, &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("RemoteResourceFromJson() failed"));
  }

  iotcon_representation_h representation = nullptr;
  result = IotconUtils::RepresentationFromJson(IotconUtils::GetArg(args, kRepresentation).get<picojson::object>(), &representation);
  if (!result) {
    LogAndReturnTizenError(result, ("RepresentationFromJson() failed"));
  }
  SCOPE_EXIT {
    iotcon_representation_destroy(representation);
  };

  iotcon_query_h query = nullptr;
  result = IotconUtils::QueryFromJson(IotconUtils::GetArg(args, kQuery).get<picojson::object>(), &query);
  if (!result) {
    LogAndReturnTizenError(result, ("QueryFromJson() failed"));
  }
  SCOPE_EXIT {
    iotcon_query_destroy(query);
  };

  std::unique_ptr<CallbackData> data{new CallbackData{PostForMethodCall(token, resource)}};

  // set options to the remote resource
  const auto& js_options = IotconUtils::GetArg(args, kOptions);

  if (js_options.is<picojson::array>()) {
    iotcon_options_h options = nullptr;

    result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
    if (!result) {
      LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_options_destroy(options);
    };

    result = IotconUtils::ConvertIotconError(iotcon_remote_resource_set_options(resource->handle, options));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
    }
  }

  result = IotconUtils::ConvertIotconError(iotcon_remote_resource_post(resource->handle, representation, query, RemoteResourceResponseCallback, data.get()));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_remote_resource_post() failed"));
  }

  // release memory ownership
  data.release();

  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(resource)};
}

common::TizenResult IotconInstance::RemoteResourceMethodDelete(const picojson::object& args,
                                                               const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  FoundRemoteInfoPtr resource;
  auto result = IotconUtils::RemoteResourceFromJson(args, &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("RemoteResourceFromJson() failed"));
  }

  std::unique_ptr<CallbackData> data{new CallbackData{PostForMethodCall(token, resource)}};

  // set options to the remote resource
  const auto& js_options = IotconUtils::GetArg(args, kOptions);

  if (js_options.is<picojson::array>()) {
    iotcon_options_h options = nullptr;

    result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
    if (!result) {
      LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_options_destroy(options);
    };

    result = IotconUtils::ConvertIotconError(iotcon_remote_resource_set_options(resource->handle, options));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
    }
  }

  result = IotconUtils::ConvertIotconError(iotcon_remote_resource_delete(resource->handle, RemoteResourceResponseCallback, data.get()));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_remote_resource_delete() failed"));
  }

  // release memory ownership
  data.release();

  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(resource)};
}

static void ObserveCallback(iotcon_remote_resource_h resource, iotcon_error_e err,
                            int sequence_number, iotcon_response_h response, void *user_data) {
  ScopeLogger();
  FoundRemoteInfo* ptr = static_cast<FoundRemoteInfo*>(user_data);
  if (ptr->observe_listener) {
    picojson::value json_result = picojson::value(picojson::object());

    auto result = IotconUtils::ResponseToJson(response, &json_result.get<picojson::object>());
    if (result) {
      ptr->observe_listener(common::TizenSuccess(), json_result);
    } else {
      LoggerD("Ignoring callback");
    }
  }
}

common::TizenResult IotconInstance::RemoteResourceSetStateChangeListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kQuery);
  CHECK_EXIST(args, kObservePolicy);
  FoundRemoteInfoPtr ptr;
  auto result = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!result) {
    LogAndReturnTizenError(result, ("Failed to create remote resource handle"));
  }

  iotcon_query_h query = nullptr;
  auto query_obj = args.find(kQuery)->second.get<picojson::object>();
  result = IotconUtils::QueryFromJson(query_obj, &query);
  if (!result){
    return result;
  }
  SCOPE_EXIT {
    iotcon_query_destroy(query);
  };

  iotcon_observe_policy_e observe_policy = IotconUtils::ToObservePolicy(
      args.find(kObservePolicy)->second.get<std::string>().c_str());

  ptr->observe_listener = [this, ptr](const common::TizenResult& res, const picojson::value& v) {
    picojson::value response{picojson::object{}};
    auto& obj = response.get<picojson::object>();

    obj.insert(std::make_pair(kId, picojson::value{static_cast<double>(ptr->id)}));
    obj.insert(std::make_pair(kData, v));

    Post(kRemoteResourceStateChangeListener, common::TizenSuccess{response});
  };

  // set options to the remote resource
  const auto& js_options = IotconUtils::GetArg(args, kOptions);

  if (js_options.is<picojson::array>()) {
    iotcon_options_h options = nullptr;

    result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
    if (!result) {
      LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_options_destroy(options);
    };

    result = IotconUtils::ConvertIotconError(iotcon_remote_resource_set_options(ptr->handle, options));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
    }
  }

  result = IotconUtils::ConvertIotconError(
      iotcon_remote_resource_observe_register(ptr->handle, observe_policy, query,
                                              ObserveCallback, ptr.get()));
  if (!result) {
    return result;
  }
  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(ptr)};
}

common::TizenResult IotconInstance::RemoteResourceUnsetStateChangeListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  FoundRemoteInfoPtr ptr;
  auto result = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!result) {
    LogAndReturnTizenError(result, ("Failed to create remote resource handle"));
  }

  // set options to the remote resource
  const auto& js_options = IotconUtils::GetArg(args, kOptions);

  if (js_options.is<picojson::array>()) {
    iotcon_options_h options = nullptr;

    result = IotconUtils::OptionsFromJson(js_options.get<picojson::array>(), &options);
    if (!result) {
      LogAndReturnTizenError(result, ("OptionsFromJson() failed"));
    }
    SCOPE_EXIT {
      iotcon_options_destroy(options);
    };

    result = IotconUtils::ConvertIotconError(iotcon_remote_resource_set_options(ptr->handle, options));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_response_set_options() failed"));
    }
  }

  result = IotconUtils::ConvertIotconError(iotcon_remote_resource_observe_deregister(ptr->handle));
  if (!result) {
    return result;
  }
  ptr->observe_listener = nullptr;
  return common::TizenSuccess{IotconClientManager::GetInstance().RemoveRemoteResource(ptr)};
}

static void RepresentationChangedCallback(iotcon_remote_resource_h resource,
                                          iotcon_representation_h representation,
                                          void *user_data) {
  LoggerD("Entered");
  //TODO probably should be handled
}

common::TizenResult IotconInstance::RemoteResourceStartCaching(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  FoundRemoteInfoPtr ptr;
  auto result = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!result) {
    LogAndReturnTizenError(result, ("Failed to create remote resource handle"));
  }
  result = IotconUtils::ConvertIotconError(
      iotcon_remote_resource_start_caching(ptr->handle, RepresentationChangedCallback, nullptr));
  if (!result) {
    return result;
  }
  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(ptr)};
}

common::TizenResult IotconInstance::RemoteResourceStopCaching(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  FoundRemoteInfoPtr ptr;
  auto result = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!result) {
    LogAndReturnTizenError(result, ("Failed to create remote resource handle"));
  }
  result = IotconUtils::ConvertIotconError(
      iotcon_remote_resource_stop_caching(ptr->handle));
  if (!result) {
    return result;
  }
  return common::TizenSuccess{IotconClientManager::GetInstance().RemoveRemoteResource(ptr)};
}

static void MonitoringCallback(iotcon_remote_resource_h resource,
                                 iotcon_remote_resource_state_e state, void *user_data) {
  ScopeLogger();
  FoundRemoteInfo* ptr = static_cast<FoundRemoteInfo*>(user_data);
  if (ptr->connection_listener) {
    picojson::value json_result = picojson::value(IOTCON_REMOTE_RESOURCE_ALIVE == state);
    ptr->connection_listener(common::TizenSuccess(), json_result);
  } else {
    LoggerD("Post function not present, just ignoring");
  }
}

common::TizenResult IotconInstance::RemoteResourceSetConnectionChangeListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  FoundRemoteInfoPtr ptr;
  auto result = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!result) {
    LogAndReturnTizenError(result, ("Failed to create remote resource handle"));
  }
  result = IotconUtils::ConvertIotconError(
      iotcon_remote_resource_start_monitoring(ptr->handle, MonitoringCallback, ptr.get()));
  if (!result) {
    return result;
  }
  ptr->connection_listener = [this, ptr](const common::TizenResult& res, const picojson::value& v) {
    picojson::value response{picojson::object{}};
    auto& obj = response.get<picojson::object>();

    obj.insert(std::make_pair(kId, picojson::value{static_cast<double>(ptr->id)}));
    obj.insert(std::make_pair(kData, v));

    Post(kRemoteResourceConnectionChangeListener, common::TizenSuccess{response});
  };

  return common::TizenSuccess{IotconClientManager::GetInstance().StoreRemoteResource(ptr)};
}

common::TizenResult IotconInstance::RemoteResourceUnsetConnectionChangeListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  FoundRemoteInfoPtr ptr;
  auto result = IotconUtils::RemoteResourceFromJson(args, &ptr);
  if (!result) {
    LogAndReturnTizenError(result, ("Failed to create remote resource handle"));
  }
  result = IotconUtils::ConvertIotconError(iotcon_remote_resource_stop_monitoring(ptr->handle));
  if (!result) {
    return result;
  }
  ptr->connection_listener = nullptr;
  return common::TizenSuccess{IotconClientManager::GetInstance().RemoveRemoteResource(ptr)};
}

bool IotconInstance::ResourceFoundCallback(iotcon_remote_resource_h resource,
                                           iotcon_error_e result, void *user_data) {
  ScopeLogger();
  CallbackData* data = static_cast<CallbackData*>(user_data);
  auto ret = IotconUtils::ConvertIotconError(result);
  if (!ret) {
    data->fun(ret, picojson::value{});
    return IOTCON_FUNC_STOP;
  }

  picojson::value json_result = picojson::value(picojson::object());

  ret = IotconUtils::RemoteResourceToJson(resource, &(json_result.get<picojson::object>()));
  if (!ret) {
    data->fun(ret, picojson::value{});
    return IOTCON_FUNC_STOP;
  }
  data->fun(ret, json_result);

  return IOTCON_FUNC_STOP;
}

common::TizenResult IotconInstance::ClientFindResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kHostAddress);
  char* host_address = nullptr;
  if (args.find(kHostAddress)->second.is<std::string>()) {
    host_address = const_cast<char*>(args.find(kHostAddress)->second.get<std::string>().c_str());
  }

  CHECK_EXIST(args, kResourceType);
  char* resource_type = nullptr;
  if (args.find(kResourceType)->second.is<std::string>()) {
    resource_type = const_cast<char*>(args.find(kResourceType)->second.get<std::string>().c_str());
  }

  CHECK_EXIST(args, kConnectivityType);
  iotcon_connectivity_type_e connectivity_type = IotconUtils::ToConnectivityType(
      args.find(kConnectivityType)->second.get<std::string>());
  CHECK_EXIST(args, kIsSecure);
  bool is_secure = args.find(kIsSecure)->second.get<bool>();

  long long id = GetId(args);
  auto response = [this, id](const common::TizenResult& res, const picojson::value& v) {
    picojson::value response{picojson::object{}};
    auto& obj = response.get<picojson::object>();

    obj.insert(std::make_pair(kId, picojson::value{static_cast<double>(id)}));
    if(res) {
      common::tools::ReportSuccess(v, obj);
    } else {
      common::tools::ReportError(res, &obj);
    }

    Post(kFindResourceListenerToken, common::TizenSuccess{response});
  };
  CallbackData* data = new CallbackData{response};

  LoggerD("Running find with:\nhost_address: %s,\nconnectivity_type: %d,\nresource_type: %s,\nis_secure: %d",
          host_address, connectivity_type, resource_type, is_secure);
  auto result = IotconUtils::ConvertIotconError(
      iotcon_find_resource(host_address, connectivity_type, resource_type,
                           is_secure, ResourceFoundCallback, data));
  if (!result) {
    delete data;
    LogAndReturnTizenError(result);
  } else {
    int timeout = 60; //default value set much bigger than default value for iotcon = 30s
    auto result = IotconUtils::ConvertIotconError(iotcon_get_timeout(&timeout));
    if (!result) {
      LoggerE("iotcon_get_timeout - function call failed, using default value %d", timeout);
    } else {
      timeout = timeout + 1; //add one extra second to prevent too fast delete
    }
    // adding listener to delete data, when find would be finished
    std::thread([data, timeout]() {
      std::this_thread::sleep_for(std::chrono::seconds(timeout));
      LoggerD("Deleting resource find data: %p", data);
      delete data;
    }).detach();
  }

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ClientAddPresenceEventListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kHostAddress);
  CHECK_EXIST(args, kResourceType);
  CHECK_EXIST(args, kConnectivityType);

  char* host = nullptr;
  if (args.find(kHostAddress)->second.is<std::string>()) {
    host = const_cast<char*>(args.find(kHostAddress)->second.get<std::string>().c_str());
  }

  char* resource_type = nullptr;
  if (args.find(kResourceType)->second.is<std::string>()) {
    resource_type = const_cast<char*>(args.find(kResourceType)->second.get<std::string>().c_str());
  }

  auto& con_type = IotconUtils::GetArg(args, kConnectivityType);
  if (!con_type.is<std::string>()) {
    return common::TypeMismatchError("connectivityType needs to be a string");
  }
  iotcon_connectivity_type_e con_type_e = IotconUtils::ToConnectivityType(
      con_type.get<std::string>());

  PresenceEventPtr presence{new PresenceEvent()};
  auto ret = IotconClientManager::GetInstance().AddPresenceEventListener(
      host, con_type_e, resource_type, presence);
  if (!ret) {
    return ret;
  }

  long long id = presence->id;

  presence->presence_listener = [this, id](const common::TizenResult&, const picojson::value& v) {
    picojson::value response{picojson::object{}};
    auto& obj = response.get<picojson::object>();

    obj.insert(std::make_pair(kId, picojson::value{static_cast<double>(id)}));
    obj.insert(std::make_pair(kData, v));

    Post(kPresenceEventListenerToken, common::TizenSuccess{response});
  };

  return common::TizenSuccess(picojson::value{static_cast<double>(id)});
}

common::TizenResult IotconInstance::ClientRemovePresenceEventListener(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kId);

  auto ret = IotconClientManager::GetInstance().RemovePresenceEventListener(GetId(args));

  if (!ret) {
    return ret;
  }

  return common::TizenSuccess();
}

bool IotconDeviceInfoCb(iotcon_device_info_h device_info,
                          iotcon_error_e result, void *user_data) {
  ScopeLogger();

  CallbackData* data = static_cast<CallbackData*>(user_data);
  picojson::value v{picojson::object{}};
  common::TizenResult ret = common::TizenSuccess();

  if (IOTCON_ERROR_NONE != result) {
    ret = IotconUtils::ConvertIotconError(result);
  } else {
    ret = IotconUtils::DeviceInfoToJson(device_info,&v.get<picojson::object>());
  }

  data->fun(ret, v);
  delete data;

  return IOTCON_FUNC_STOP;
}

common::TizenResult IotconInstance::ClientGetDeviceInfo(const picojson::object& args,
                                                        const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kHostAddress);
  CHECK_EXIST(args, kConnectivityType);

  std::string host = args.find(kHostAddress)->second.get<std::string>();
  std::string con_type = args.find(kConnectivityType)->second.get<std::string>();
  iotcon_connectivity_type_e con_type_e = IotconUtils::ToConnectivityType(con_type);

  CallbackData* data = new CallbackData{SimplePost(token)};

  auto result = IotconUtils::ConvertIotconError(
       iotcon_find_device_info(host.c_str(), con_type_e, IotconDeviceInfoCb,
                                data));

  if (!result) {
    delete data;
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess();
}

bool IotconPlatformInfoCb(iotcon_platform_info_h platform_info,
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

  return IOTCON_FUNC_STOP;
}

common::TizenResult IotconInstance::ClientGetPlatformInfo(const picojson::object& args,
                                                          const common::AsyncToken& token) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kHostAddress);
  CHECK_EXIST(args, kConnectivityType);

  std::string host = args.find(kHostAddress)->second.get<std::string>();
  std::string con_type = args.find(kConnectivityType)->second.get<std::string>();
  iotcon_connectivity_type_e con_type_e = IotconUtils::ToConnectivityType(con_type);

  CallbackData* data = new CallbackData{SimplePost(token)};

  auto result = IotconUtils::ConvertIotconError(
       iotcon_find_platform_info(host.c_str(), con_type_e, IotconPlatformInfoCb,
                                data));

  if (!result) {
    delete data;
    LogAndReturnTizenError(result);
  }

  return common::TizenSuccess();
}

common::TizenResult IotconInstance::ServerCreateResource(const picojson::object& args) {
  ScopeLogger();

  CHECK_PRIVILEGE(kPrivilegeIotcon);

  CHECK_EXIST(args, kUriPath);

  const std::string& uri_path = args.find(kUriPath)->second.get<std::string>();

  const auto& interfaces = IotconUtils::GetArg(args, kResourceInterfaces);
  const auto& resource_interfaces = interfaces.is<picojson::array>() ? interfaces.get<picojson::array>() : picojson::array();

  const auto& types = IotconUtils::GetArg(args, kResourceTypes);
  const auto& resource_types = types.is<picojson::array>() ? types.get<picojson::array>() : picojson::array();

  int properties = IotconUtils::GetProperties(args);

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

  CHECK_PRIVILEGE(kPrivilegeIotcon);

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

common::PostCallback IotconInstance::PostForMethodCall(const common::AsyncToken& token, const FoundRemoteInfoPtr& resource) {
  ScopeLogger();

  return [this, token, resource](const common::TizenResult& result, const picojson::value& v) {
    auto value = IotconClientManager::GetInstance().RemoveRemoteResource(resource);
    auto& obj = value.get<picojson::object>();

    if (result) {
      obj.insert(std::make_pair(kData, v));
    } else {
      result.ToJson(&obj);
    }

    Post(token, common::TizenSuccess{value});
  };
}

}  // namespace iotcon
}  // namespace extension
