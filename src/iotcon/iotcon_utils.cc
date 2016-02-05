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

#include "iotcon_utils.h"

#include <memory>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/scope_exit.h"

#include "iotcon/iotcon_server_manager.h"

namespace extension {
namespace iotcon {

const std::string kIsDiscoverable = "isDiscoverable";
const std::string kIsObservable = "isObservable";
const std::string kIsActive = "isActive";
const std::string kIsSlow = "isSlow";
const std::string kIsSecure = "isSecure";
const std::string kIsExplicitDiscoverable = "isExplicitDiscoverable";
const std::string kResourceTypes = "resourceTypes";
const std::string kResourceInterfaces = "resourceInterfaces";
const std::string kResourceChildren = "resources";
const std::string kUriPath = "uriPath";
const std::string kResourceId = "id";

const std::string kInterfaceDefault = "DEFAULT";
const std::string kInterfaceLink = "LINK";
const std::string kInterfaceBatch = "BATCH";
const std::string kInterfaceGroup = "GROUP";

using common::TizenResult;
using common::TizenSuccess;

TizenResult IotconUtils::StringToInterface(const std::string& interface, iotcon_interface_e* res) {
  ScopeLogger();

  if (kInterfaceDefault == interface) {
    *res = IOTCON_INTERFACE_DEFAULT;
  } else if (kInterfaceLink == interface) {
    *res = IOTCON_INTERFACE_LINK;
  } else if (kInterfaceBatch == interface) {
    *res = IOTCON_INTERFACE_BATCH;
  } else if (kInterfaceGroup == interface) {
    *res = IOTCON_INTERFACE_GROUP;
  } else {
    return LogAndCreateTizenError(InvalidValuesError, "Not supported interface name");
  }
  return TizenSuccess();
}

TizenResult IotconUtils::ArrayToInterfaces(const picojson::array& interfaces, int* res) {
  ScopeLogger();

  int result_value = IOTCON_INTERFACE_NONE;

  for (auto iter = interfaces.begin(); iter != interfaces.end(); ++iter) {
    if (!iter->is<std::string>()) {
      return LogAndCreateTizenError(InvalidValuesError, "Array holds incorrect interface names");
    } else {
      iotcon_interface_e interface = IOTCON_INTERFACE_NONE;
      auto ret = StringToInterface(iter->get<std::string>(), &interface);
      if (!ret) {
        return ret;
      }
      result_value |= interface;
    }
  }
  *res = result_value;
  return TizenSuccess();
}

picojson::array IotconUtils::InterfacesToArray(int interfaces) {
  ScopeLogger();

  picojson::array res;
  if (interfaces & IOTCON_INTERFACE_DEFAULT) {
    res.push_back(picojson::value(kInterfaceDefault));
  }
  if (interfaces & IOTCON_INTERFACE_LINK) {
    res.push_back(picojson::value(kInterfaceLink));
  }
  if (interfaces & IOTCON_INTERFACE_BATCH) {
    res.push_back(picojson::value(kInterfaceBatch));
  }
  if (interfaces & IOTCON_INTERFACE_GROUP) {
    res.push_back(picojson::value(kInterfaceGroup));
  }
  return res;
}

TizenResult IotconUtils::ArrayToTypes(const picojson::array& types, iotcon_resource_types_h* res) {
  ScopeLogger();

  iotcon_resource_types_h resource_types = nullptr;
  int ret = iotcon_resource_types_create(&resource_types);
  if (IOTCON_ERROR_NONE != ret) {
    return LogAndCreateTizenError(UnknownError, "Unknown error occurred.",
                                  ("iotcon_resource_types_create failed: %d (%s)",
                                  ret, get_error_message(ret)));
  }

  for (auto iter = types.begin(); iter != types.end(); ++iter) {
    if (!iter->is<std::string>()) {
      return LogAndCreateTizenError(InvalidValuesError, "Array holds incorrect types");
    } else {
      ret = iotcon_resource_types_add(resource_types, iter->get<std::string>().c_str());
      if (IOTCON_ERROR_NONE != ret) {
        iotcon_resource_types_destroy(resource_types);
        return LogAndCreateTizenError(UnknownError, "Unknown error occurred.",
                                      ("iotcon_resource_types_add failed: %d (%s)",
                                      ret, get_error_message(ret)));
      }
    }
  }
  *res = resource_types;
  return TizenSuccess();
}

static bool ResourceTypeIterator(const char *type, void *user_data) {
  ScopeLogger();

  picojson::array* array_data = static_cast<picojson::array*>(user_data);
  if (!array_data) {
    LoggerE("user_data is NULL");
    return false;
  }

  array_data->push_back(picojson::value(type));
  return true;
}

TizenResult IotconUtils::ExtractFromResource(const ResourceInfoPtr& pointer,
                                             char** uri_path,
                                             iotcon_resource_types_h* res_types,
                                             int* ifaces,
                                             int* properties) {
  ScopeLogger();

  int ret = iotcon_resource_get_uri_path (pointer->handle, uri_path);
  if (IOTCON_ERROR_NONE != ret) {
    LoggerD("Error %s", get_error_message(ret));
    return LogAndCreateTizenError(UnknownError, "Gathering resource uri path failed");
  }

  ret = iotcon_resource_get_types (pointer->handle, res_types);
  if (IOTCON_ERROR_NONE != ret) {
    LoggerD("Error %s", get_error_message(ret));
    return LogAndCreateTizenError(UnknownError, "Gathering resource types failed");
  }

  ret = iotcon_resource_get_interfaces (pointer->handle, ifaces);
  if (IOTCON_ERROR_NONE != ret) {
    LoggerD("Error %s", get_error_message(ret));
    return LogAndCreateTizenError(UnknownError, "Gathering resource interfaces failed");
  }

  ret = iotcon_resource_get_properties (pointer->handle, properties);
  if (IOTCON_ERROR_NONE != ret) {
    LoggerD("Error %s", get_error_message(ret));
    return LogAndCreateTizenError(UnknownError, "Gathering resource properties failed");
  }
  return TizenSuccess();
}

TizenResult IotconUtils::ResourceToJson(ResourceInfoPtr pointer,
                                        const IotconServerManager& manager,
                                        picojson::object* res) {
  ScopeLogger();

  char* uri_path = nullptr;
  iotcon_resource_types_h res_types = nullptr;
  int ifaces = 0;
  int properties = 0;
  auto ret = ExtractFromResource(pointer, &uri_path, &res_types, &ifaces, &properties);
  if (!ret){
    return ret;
  }
  res->insert(std::make_pair(kResourceId, picojson::value(static_cast<double>(pointer->id))));
  res->insert(std::make_pair(kUriPath, picojson::value(uri_path)));

  picojson::array types;
  iotcon_resource_types_foreach(res_types, ResourceTypeIterator, &types);
  res->insert(std::make_pair(kResourceTypes, picojson::value(types)));

  res->insert(std::make_pair(kResourceInterfaces,
                             picojson::value(InterfacesToArray(ifaces))));
  bool value = properties & IOTCON_RESOURCE_OBSERVABLE;
  res->insert(std::make_pair(kIsObservable, picojson::value(value)));
  value = properties & IOTCON_RESOURCE_DISCOVERABLE;
  res->insert(std::make_pair(kIsDiscoverable, picojson::value(value)));
  value = properties & IOTCON_RESOURCE_ACTIVE;
  res->insert(std::make_pair(kIsActive, picojson::value(value)));
  value = properties & IOTCON_RESOURCE_SLOW;
  res->insert(std::make_pair(kIsSlow, picojson::value(value)));
  value = properties & IOTCON_RESOURCE_SECURE;
  res->insert(std::make_pair(kIsSecure, picojson::value(value)));
  value = properties & IOTCON_RESOURCE_EXPLICIT_DISCOVERABLE;
  res->insert(std::make_pair(kIsExplicitDiscoverable, picojson::value(value)));

  picojson::array children;
  for (auto iter = pointer->children_ids.begin(); iter != pointer->children_ids.end(); ++iter) {
    if (pointer->id == (*iter)) {
      // prevent infinite recurrence
      continue;
    }
    ResourceInfoPtr resource;
    ret = manager.GetResourceById((*iter), &resource);
    if (ret.IsSuccess()) {
      LoggerD("Found children RESOURCE\nid: %lld\nhandle: %p", resource->id, resource->handle);

      picojson::value child = picojson::value(picojson::object());
      ret = IotconUtils::ResourceToJson(resource, manager, &(child.get<picojson::object>()));
      if (ret.IsSuccess()) {
        children.push_back(child);
      }
    } else {
      LoggerD("Not found such resource");
    }
  }
  res->insert(std::make_pair(kResourceChildren, picojson::value(children)));
  // observerIds would be done on demand from JS

  return TizenSuccess();
}

} // namespace iotcon
} // namespace extension
