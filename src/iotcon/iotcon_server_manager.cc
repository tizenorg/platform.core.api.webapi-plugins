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

IotconServerManager::IotconServerManager(IotconInstance* instance)
      : instance_(instance),
        global_id_(0) {
  ScopeLogger();
}

IotconServerManager::~IotconServerManager() {
  ScopeLogger();
}

TizenResult IotconServerManager::RestoreHandles() {
  ScopeLogger();

  for (auto it = resource_map_.begin(); it != resource_map_.end(); ++it) {
    LoggerD ("Restoring handle for resource with id: %lld", it->first);
    ResourceInfoPtr resource = it->second;

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
                                     nullptr, // user_data
                                     &(resource->handle));
    if (IOTCON_ERROR_NONE != ret || nullptr == resource->handle) {
      return LogAndCreateTizenError(UnknownError, "Unknown error occurred.",
                                    ("iotcon_resource_create failed: %d (%s)",
                                    ret, get_error_message(ret)));
    }
    LoggerD("new handle: %p", (resource->handle));
    if (old_handle) {
      LoggerD("destroy handle which is currently invalid: %p", old_handle);
      iotcon_resource_destroy(old_handle);
    }
  }
  // TODO
  // bind children (consider if it is necessary?
  //    Maybe holding children in resource_map is enough)

  return TizenSuccess();
}

void IotconServerManager::RequestHandler(iotcon_resource_h resource,
                                         iotcon_request_h request, void *user_data) {
  ScopeLogger();
  // TODO probably should be handled somehow later
}

TizenResult IotconServerManager::CreateResource(const std::string& uri_path,
                                                const picojson::array& interfaces_array,
                                                const picojson::array& types_array,
                                                bool is_discoverable,
                                                bool is_observable,
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

  // TODO consider support other properties
  int properties = ((is_discoverable ? IOTCON_RESOURCE_DISCOVERABLE : IOTCON_RESOURCE_NO_PROPERTY) |
      (is_observable ? IOTCON_RESOURCE_OBSERVABLE : IOTCON_RESOURCE_NO_PROPERTY) |
          IOTCON_RESOURCE_ACTIVE);

  // Create resource
  ret = iotcon_resource_create(uri_path.c_str(),
                               resource_types,
                               interfaces,
                               properties,
                               RequestHandler, // request_callback
                               nullptr, // user_data
                               &(res_pointer->handle));
  if (IOTCON_ERROR_NONE != ret || nullptr == res_pointer->handle) {
    return LogAndCreateTizenError(UnknownError, "Unknown error occurred.",
                                  ("iotcon_resource_create failed: %d (%s)",
                                  ret, get_error_message(ret)));
  }

  // storing ResourceInfo into map
  res_pointer->id = ++global_id_;
  resource_map_.insert(std::make_pair(res_pointer->id, res_pointer));
  return TizenSuccess();
}

TizenResult IotconServerManager::GetResourceById(long long id,
                                                 ResourceInfoPtr* res_pointer) const {
  ScopeLogger();

  auto it = resource_map_.find(id);
  if (it == resource_map_.end()) {
    LoggerE("Not found such resource");
    return LogAndCreateTizenError(NotFoundError, "Not found such resource");
  }
  LoggerE("Resource found");
  *res_pointer = it->second;

  return TizenSuccess();
}

}  // namespace iotcon
}  // namespace extension
