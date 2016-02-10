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

#include "iotcon/iotcon_server_manager.h"

#include "common/logger.h"

#include "iotcon/iotcon_instance.h"

namespace extension {
namespace iotcon {

using common::TizenResult;
using common::TizenSuccess;

namespace {

long long GetNextId() {
  static long long id = 0;
  return ++id;
}

}  // namespace

IotconServerManager& IotconServerManager::GetInstance() {
  static IotconServerManager instance;
  return instance;
}

TizenResult IotconServerManager::RestoreHandles() {
  ScopeLogger();

  for (const auto& it : resource_map_) {
    LoggerD("Restoring handle for resource with id: %lld", it.first);

    ResourceInfoPtr resource = it.second;
    char* uri_path = nullptr;
    iotcon_resource_types_h res_types = nullptr;
    int ifaces = 0;
    int properties = 0;

    auto res = IotconUtils::ExtractFromResource(resource, &uri_path,
                                                &res_types, &ifaces, &properties);
    if (!res){
      return res;
    }

    const iotcon_resource_h old_handle = resource->handle;
    LoggerD("Create resource from backup data, uri: %s, res_types: %p, ifaces: %d, properties: %d",
            uri_path, res_types, ifaces, properties);

    int ret = iotcon_resource_create(uri_path, res_types, ifaces, properties,
                                     RequestHandler, // request_callback
                                     this, // user_data
                                     &(resource->handle));
    if (IOTCON_ERROR_NONE != ret || nullptr == resource->handle) {
      LogAndReturnTizenError(IotconUtils::ConvertIotconError(ret),
                             ("iotcon_resource_create() failed: %d (%s)",
                                 ret, get_error_message(ret)));
    }
    LoggerD("new handle: %p", (resource->handle));
    if (old_handle) {
      LoggerD("destroy handle which is currently invalid: %p", old_handle);
      iotcon_resource_destroy(old_handle);
    }
  }

  // rebind children
  for (const auto& it : resource_map_) {
    for (const auto& child : it.second->children) {
      auto result = IotconUtils::ConvertIotconError(iotcon_resource_bind_child_resource(it.second->handle, child->handle));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_resource_bind_child_resource() failed"));
      }
    }
  }

  return TizenSuccess();
}

void IotconServerManager::RequestHandler(iotcon_resource_h resource,
                                         iotcon_request_h request, void *user_data) {
  ScopeLogger();

  auto that = static_cast<IotconServerManager*>(user_data);
  ResourceInfoPtr r;
  auto result = that->GetResourceByHandle(resource, &r);

  if (!result) {
    LoggerE("Resource is not handled by the manager!");
    return;
  }

  // handle observer changes
  iotcon_observe_type_e type = IOTCON_OBSERVE_NO_TYPE;
  result = IotconUtils::ConvertIotconError(iotcon_request_get_observe_type(request, &type));

  if (!result) {
    LoggerE("iotcon_request_get_observe_type() failed");
    return;
  }

  int observer_id = -1;
  result = IotconUtils::ConvertIotconError(iotcon_request_get_observe_id(request, &observer_id));

  if (!result) {
    LoggerE("iotcon_request_get_observe_id() failed");
    return;
  }

  switch (type) {
    case IOTCON_OBSERVE_NO_TYPE:
      LoggerD("observer did not change");
      break;

    case IOTCON_OBSERVE_REGISTER:
      LoggerD("Observer has been registered: %d", observer_id);
      r->observers.insert(observer_id);
      break;

    case IOTCON_OBSERVE_DEREGISTER:
      LoggerD("Observer has been deregistered: %d", observer_id);
      r->observers.erase(observer_id);
      break;
  }

  if (r->request_listener) {
    // convert request to JSON
    picojson::value value{picojson::object{}};
    auto& obj = value.get<picojson::object>();

    result = IotconUtils::RequestToJson(request, &obj);

    if (!result) {
      LoggerE("RequestToJson() failed");
      return;
    }

    // create response
    iotcon_response_h response = nullptr;
    result = IotconUtils::ConvertIotconError(iotcon_response_create(request, &response));

    if (!result) {
      LoggerE("iotcon_response_create() failed");
      return;
    }

    // store data
    long long id = GetNextId();
    obj.insert(std::make_pair(kId, picojson::value{static_cast<double>(id)}));
    r->unhandled_responses.insert(std::make_pair(id, response));

    // call listener
    r->request_listener(TizenSuccess(), value);
  }
}

TizenResult IotconServerManager::CreateResource(const std::string& uri_path,
                                                const picojson::array& interfaces_array,
                                                const picojson::array& types_array,
                                                int properties,
                                                ResourceInfoPtr res_pointer) {
  ScopeLogger();

  int ret;
  int interfaces = IOTCON_INTERFACE_NONE;
  auto res = IotconUtils::ArrayToInterfaces(interfaces_array, &interfaces);
  if (!res) {
    return res;
  }

  iotcon_resource_types_h resource_types = nullptr;
  res = IotconUtils::ArrayToTypes(types_array, &resource_types);
  if (!res) {
    return res;
  }

  // Create resource
  ret = iotcon_resource_create(uri_path.c_str(),
                               resource_types,
                               interfaces,
                               properties,
                               RequestHandler, // request_callback
                               this, // user_data
                               &(res_pointer->handle));
  if (IOTCON_ERROR_NONE != ret || nullptr == res_pointer->handle) {
    LogAndReturnTizenError(IotconUtils::ConvertIotconError(ret),
                           ("iotcon_resource_create() failed: %d (%s)",
                               ret, get_error_message(ret)));
  }

  // storing ResourceInfo into map
  res_pointer->id = GetNextId();
  resource_map_.insert(std::make_pair(res_pointer->id, res_pointer));
  return TizenSuccess();
}

TizenResult IotconServerManager::GetResourceById(long long id,
                                                 ResourceInfoPtr* res_pointer) const {
  ScopeLogger();

  auto it = resource_map_.find(id);
  if (it == resource_map_.end()) {
    return LogAndCreateTizenError(NotFoundError, "Resource with specified ID does not exist");
  }
  LoggerE("Resource found");
  *res_pointer = it->second;

  return TizenSuccess();
}

common::TizenResult IotconServerManager::DestroyResource(long long id) {
  ScopeLogger();

  ResourceInfoPtr resource;
  auto result = GetResourceById(id, &resource);
  if (!result) {
    LogAndReturnTizenError(result, ("GetResourceById() failed"));
  }

  // do not allow to destroy a resource which has a parent resource
  if (resource->parents.size() > 0) {
    return LogAndCreateTizenError(InvalidStateError, "Cannot destroy child resource, remove it from parent first");
  }

  // notify children they've lost a parent :(
  for (const auto& child : resource->children) {
    child->parents.erase(resource);
  }

  resource_map_.erase(id);

  return TizenSuccess();
}

common::TizenResult IotconServerManager::GetResourceByHandle(
    iotcon_resource_h resource, ResourceInfoPtr* res_pointer) const {
  ScopeLogger();

  auto it = std::find_if(resource_map_.begin(), resource_map_.end(), [resource](const ResourceInfoMap::value_type& p) -> bool {
    return p.second->handle == resource;
  });

  if (it == resource_map_.end()) {
    return LogAndCreateTizenError(NotFoundError, "Resource with specified handle does not exist");
  }

  LoggerE("Resource found");
  *res_pointer = it->second;

  return TizenSuccess();
}

}  // namespace iotcon
}  // namespace extension
