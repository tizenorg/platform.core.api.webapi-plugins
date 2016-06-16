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
#include "common/tools.h"

#include "iotcon/iotcon_server_manager.h"
#include "iotcon/iotcon_client_manager.h"

namespace extension {
namespace iotcon {

namespace {

#define IOTCON_CONNECTIVITY_TYPE_E \
  X(IOTCON_CONNECTIVITY_IPV4, "IPV4") \
  X(IOTCON_CONNECTIVITY_IPV6, "IPV6") \
  X(IOTCON_CONNECTIVITY_ALL, "ALL") \
  XD(IOTCON_CONNECTIVITY_ALL, "unknown")

#define IOTCON_REQUEST_TYPE_E \
  X(IOTCON_REQUEST_UNKNOWN, "unknown") \
  X(IOTCON_REQUEST_GET, "GET") \
  X(IOTCON_REQUEST_PUT, "PUT") \
  X(IOTCON_REQUEST_POST, "POST") \
  X(IOTCON_REQUEST_DELETE, "DELETE") \
  XD(IOTCON_REQUEST_UNKNOWN, "unknown")

#define IOTCON_OBSERVE_TYPE_E \
  X(IOTCON_OBSERVE_NO_TYPE, "NO_TYPE") \
  X(IOTCON_OBSERVE_REGISTER, "REGISTER") \
  X(IOTCON_OBSERVE_DEREGISTER, "DEREGISTER") \
  XD(IOTCON_OBSERVE_NO_TYPE, "unknown")

#define IOTCON_QOS_E \
  X(IOTCON_QOS_LOW, "LOW") \
  X(IOTCON_QOS_HIGH, "HIGH") \
  XD(IOTCON_QOS_LOW, "unknown")

#define IOTCON_PRESENCE_RESULT_E \
  X(IOTCON_PRESENCE_OK, "OK") \
  X(IOTCON_PRESENCE_STOPPED, "STOPPED") \
  XD(IOTCON_PRESENCE_TIMEOUT, "TIMEOUT")

#define IOTCON_PRESENCE_TRIGGER_E \
  X(IOTCON_PRESENCE_RESOURCE_CREATED, "CREATED") \
  X(IOTCON_PRESENCE_RESOURCE_UPDATED, "UPDATED") \
  XD(IOTCON_PRESENCE_RESOURCE_DESTROYED, "DESTROYED")

#define IOTCON_RESPONSE_RESULT_E \
  X(IOTCON_RESPONSE_OK, "SUCCESS") \
  X(IOTCON_RESPONSE_ERROR, "ERROR") \
  X(IOTCON_RESPONSE_RESOURCE_CREATED, "RESOURCE_CREATED") \
  X(IOTCON_RESPONSE_RESOURCE_DELETED, "RESOURCE_DELETED") \
  X(IOTCON_RESPONSE_SLOW, "SLOW") \
  X(IOTCON_RESPONSE_FORBIDDEN, "FORBIDDEN") \
  XD(IOTCON_RESPONSE_ERROR, "unknown")

#define IOTCON_OBSERVE_POLICY_E \
  X(IOTCON_OBSERVE_IGNORE_OUT_OF_ORDER, "IGNORE_OUT_OF_ORDER") \
  X(IOTCON_OBSERVE_ACCEPT_OUT_OF_ORDER, "ACCEPT_OUT_OF_ORDER") \
  XD(IOTCON_OBSERVE_ACCEPT_OUT_OF_ORDER, "unknown")

}  // namespace

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
const std::string kStates = "states";
const std::string kId = "id";
const std::string kKeepId = "keepId";
const std::string kDeviceId = "deviceId";
const std::string kHostAddress = "hostAddress";
const std::string kConnectivityType = "connectivityType";
const std::string kObservePolicy = "observePolicy";

const std::string kRepresentation = "representation";
const std::string kRepresentations = "representations";
const std::string kRequestType = "type";
const std::string kOptions = "options";
const std::string kQuery = "query";
const std::string kObserverId = "observerId";
const std::string kObserveType = "observeType";

const std::string kOptionsId = "id";
const std::string kOptionsData = "data";

const std::string kResourceType = "resourceType";
const std::string kResourceInterface = "resourceInterface";
const std::string kFilter = "filter";

const std::string kHexPrefix = "0x";

const std::string kPlatformId = "platformId";
const std::string kManufacturerName = "manufacturerName";
const std::string kManufacturerUrl = "manufacturerUrl";
const std::string kModelNumber = "modelNumber";
const std::string kManufactureDate = "manufactureDate";
const std::string kPlatformVersion = "platformVersion";
const std::string kOperatingSystemVersion = "operatingSystemVersion";
const std::string kHardwareVersion = "hardwareVersion";
const std::string kFirmwareVersion = "firmwareVersion";
const std::string kSupportUrl = "supportUrl";
const std::string kSystemTime = "systemTime";

const std::string kDeviceName = "deviceName";
const std::string kSpecVersion = "specVersion";
const std::string kOicDeviceId = "oicDeviceId";
const std::string kDataModelVersion = "dataModelVersion";

const std::string kResultType = "resultType";
const std::string kTriggerType = "triggerType";

const std::string kResult = "result";

using common::TizenResult;
using common::TizenSuccess;

const picojson::value& IotconUtils::GetArg(const picojson::object& args, const std::string& name) {
  static const picojson::value kNull;

  auto it = args.find(name);
  if (args.end() == it) {
    return kNull;
  } else {
    return it->second;
  }
}

int IotconUtils::GetProperties(const picojson::object& args) {
  int properties = IOTCON_RESOURCE_NO_POLICY;

  const auto& observable = IotconUtils::GetArg(args, kIsObservable);
  properties |= (observable.is<bool>() ? observable.get<bool>() : false) ? IOTCON_RESOURCE_OBSERVABLE : IOTCON_RESOURCE_NO_POLICY;

  const auto& discoverable = IotconUtils::GetArg(args, kIsDiscoverable);
  properties |= (discoverable.is<bool>() ? discoverable.get<bool>() : true) ? IOTCON_RESOURCE_DISCOVERABLE : IOTCON_RESOURCE_NO_POLICY;

  const auto& active = IotconUtils::GetArg(args, kIsActive);
  properties |= (active.is<bool>() ? active.get<bool>() : false) ? IOTCON_RESOURCE_ACTIVE : IOTCON_RESOURCE_NO_POLICY;

  const auto& slow = IotconUtils::GetArg(args, kIsSlow);
  properties |= (slow.is<bool>() ? slow.get<bool>() : false) ? IOTCON_RESOURCE_SLOW : IOTCON_RESOURCE_NO_POLICY;

  const auto& secure = IotconUtils::GetArg(args, kIsSecure);
  properties |= (secure.is<bool>() ? secure.get<bool>() : false) ? IOTCON_RESOURCE_SECURE : IOTCON_RESOURCE_NO_POLICY;

  const auto& explicit_discoverable = IotconUtils::GetArg(args, kIsExplicitDiscoverable);
  properties |= (explicit_discoverable.is<bool>() ? explicit_discoverable.get<bool>() : false) ? IOTCON_RESOURCE_EXPLICIT_DISCOVERABLE : IOTCON_RESOURCE_NO_POLICY;

  return properties;
}

void IotconUtils::PropertiesToJson(int properties, picojson::object* res) {
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
}

TizenResult IotconUtils::ArrayToInterfaces(const picojson::array& i, iotcon_resource_interfaces_h* out) {
  ScopeLogger();

  iotcon_resource_interfaces_h interfaces = nullptr;

  auto result = ConvertIotconError(iotcon_resource_interfaces_create(&interfaces));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_interfaces_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_resource_interfaces_h>::type, void(*)(iotcon_resource_interfaces_h)> ptr{interfaces, &iotcon_resource_interfaces_destroy};

  for (const auto& iter : i) {
    if (!iter.is<std::string>()) {
      return LogAndCreateTizenError(InvalidValuesError, "Interface name should be a string");
    } else {
      result = ConvertIotconError(iotcon_resource_interfaces_add(interfaces, iter.get<std::string>().c_str()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_resource_interfaces_add() failed"));
      }
    }
  }

  *out = ptr.release();
  return TizenSuccess();
}

TizenResult IotconUtils::InterfacesToArray(iotcon_resource_interfaces_h interfaces, picojson::array* arr) {
  ScopeLogger();

  if (interfaces) {
    auto result = ConvertIotconError(iotcon_resource_interfaces_foreach(interfaces, [](const char* iface, void* user_data) -> bool {
      ScopeLogger("iotcon_resource_interfaces_foreach");

      if (iface) {
        auto arr = static_cast<picojson::array*>(user_data);
        arr->push_back(picojson::value(iface));
      }

      // always continue with iteration
      return true;
    }, arr));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_interfaces_foreach() failed"));
    }
  } else {
    LoggerW("Interface handle is null, ignoring");
  }
  return TizenSuccess();
}

TizenResult IotconUtils::ArrayToTypes(const picojson::array& types, iotcon_resource_types_h* res) {
  ScopeLogger();

  iotcon_resource_types_h resource_types = nullptr;
  auto result = ConvertIotconError(iotcon_resource_types_create(&resource_types));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_types_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_resource_types_h>::type, void(*)(iotcon_resource_types_h)> ptr{resource_types, &iotcon_resource_types_destroy};

  for (const auto& iter : types) {
    if (!iter.is<std::string>()) {
      return LogAndCreateTizenError(InvalidValuesError, "Resource type should be a string");
    } else {
      result = ConvertIotconError(iotcon_resource_types_add(resource_types, iter.get<std::string>().c_str()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_resource_types_add() failed"));
      }
    }
  }

  *res = ptr.release();
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
                                             iotcon_resource_interfaces_h* ifaces,
                                             int* properties) {
  ScopeLogger();

  auto result = ConvertIotconError(iotcon_resource_get_uri_path(pointer->handle, uri_path));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering resource uri path failed"));
  }

  result = ConvertIotconError(iotcon_resource_get_types(pointer->handle, res_types));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering resource types failed"));
  }

  result = ConvertIotconError(iotcon_resource_get_interfaces(pointer->handle, ifaces));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering resource interfaces failed"));
  }

  result = ConvertIotconError(iotcon_resource_get_policies(pointer->handle, properties));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering resource properties failed"));
  }
  return TizenSuccess();
}

TizenResult IotconUtils::ResourceToJson(ResourceInfoPtr pointer,
                                        picojson::object* res) {
  ScopeLogger();

  char* uri_path = nullptr;
  iotcon_resource_types_h res_types = nullptr;
  iotcon_resource_interfaces_h ifaces = nullptr;
  int properties = 0;
  auto ret = ExtractFromResource(pointer, &uri_path, &res_types, &ifaces, &properties);
  if (!ret){
    return ret;
  }
  res->insert(std::make_pair(kId, picojson::value(static_cast<double>(pointer->id))));
  res->insert(std::make_pair(kUriPath, picojson::value(uri_path)));

  picojson::array types;
  iotcon_resource_types_foreach(res_types, ResourceTypeIterator, &types);
  res->insert(std::make_pair(kResourceTypes, picojson::value(types)));

  picojson::array interfaces;
  ret = InterfacesToArray(ifaces, &interfaces);
  if (!ret) {
    LogAndReturnTizenError(ret, ("InterfacesToArray() failed"));
  }
  res->insert(std::make_pair(kResourceInterfaces, picojson::value(interfaces)));
  IotconUtils::PropertiesToJson(properties, res);

  picojson::array children;
  for (const auto& child_resource : pointer->children) {
    picojson::value child = picojson::value(picojson::object());
    ret = IotconUtils::ResourceToJson(child_resource, &(child.get<picojson::object>()));
    if (ret.IsSuccess()) {
      children.push_back(child);
    }
  }
  res->insert(std::make_pair(kResourceChildren, picojson::value(children)));

  // observerIds would be done on demand from JS

  return TizenSuccess();
}

TizenResult IotconUtils::ExtractFromRemoteResource(RemoteResourceInfo* resource) {
  ScopeLogger();

  auto result = ConvertIotconError(
      iotcon_remote_resource_get_uri_path(resource->resource, &resource->uri_path));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering uri path failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_connectivity_type(resource->resource, &resource->connectivity_type));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering connectivity type failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_host_address(resource->resource, &resource->host_address));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering host address failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_device_id(resource->resource, &resource->device_id));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering host address failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_types(resource->resource, &resource->types));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering types failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_interfaces(resource->resource, &resource->ifaces));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering interfaces failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_policies(resource->resource, &resource->properties));
  if (!result) {
    LogAndReturnTizenError(result, ("Gathering properties failed"));
  }

  result = ConvertIotconError(
      iotcon_remote_resource_get_cached_representation(resource->resource, &resource->representation));
  if (!result) {
    LoggerD("Gathering cached representation failed");
    //TODO check: native method returns error here, now ignoring fail instead of returning error
    //LogAndReturnTizenError(result, ("Gathering cached representation failed"));
  }

  return TizenSuccess();
}

TizenResult IotconUtils::RemoteResourceToJson(iotcon_remote_resource_h handle,
                                              picojson::object* res) {
  ScopeLogger();

  RemoteResourceInfo remote_res;
  remote_res.resource = handle;
  auto result = ExtractFromRemoteResource(&remote_res);
  if (!result){
    return result;
  }
  res->insert(std::make_pair(kUriPath, picojson::value(remote_res.uri_path)));
  res->insert(std::make_pair(kConnectivityType, picojson::value(
      FromConnectivityType(remote_res.connectivity_type))));
  res->insert(std::make_pair(kHostAddress, picojson::value(remote_res.host_address)));
  res->insert(std::make_pair(kDeviceId, picojson::value(remote_res.device_id)));

  if (remote_res.types) {
    picojson::array types;
    iotcon_resource_types_foreach(remote_res.types, ResourceTypeIterator, &types);
    res->insert(std::make_pair(kResourceTypes, picojson::value(types)));
  }

  if (remote_res.ifaces) {
    picojson::array interfaces;
    result = InterfacesToArray(remote_res.ifaces, &interfaces);
    if (!result) {
      LogAndReturnTizenError(result, ("InterfacesToArray() failed"));
    }
    res->insert(std::make_pair(kResourceInterfaces, picojson::value(interfaces)));
  }

  IotconUtils::PropertiesToJson(remote_res.properties, res);

  if (remote_res.representation) {
    picojson::value repr_json{picojson::object{}};
    result = RepresentationToJson(remote_res.representation, &repr_json.get<picojson::object>());
    if (!result) {
      LogAndReturnTizenError(result, ("RepresentationToJson() failed"));
    }
    res->insert(std::make_pair(kRepresentation, repr_json));
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::RemoteResourceFromJson(const picojson::object& source,
                                                        FoundRemoteInfoPtr* ptr) {
  ScopeLogger();
  //checking if resource has id
  long long id = 0;
  if (source.find(kId)->second.is<double>()) {
    id = static_cast<long long>(source.find(kId)->second.get<double>());
  }
  if (id > 0) {
    LoggerD("Resource stored, getting it from IotconClientManager");
    return IotconClientManager::GetInstance().GetRemoteById(id, ptr);
  } else {
    LoggerD("Id is not defined, creating handle using resource info");
  }

  (*ptr).reset(new FoundRemoteInfo{});
  CHECK_EXIST(source, kHostAddress);
  char* host_address = nullptr;
  if (source.find(kHostAddress)->second.is<std::string>()) {
    host_address = const_cast<char*>(source.find(kHostAddress)->second.get<std::string>().c_str());
  }

  CHECK_EXIST(source, kConnectivityType);
  iotcon_connectivity_type_e connectivity_type = IotconUtils::ToConnectivityType(
      source.find(kConnectivityType)->second.get<std::string>());

  CHECK_EXIST(source, kUriPath);
  char* uri_path = nullptr;
  if (source.find(kUriPath)->second.is<std::string>()) {
    uri_path = const_cast<char*>(source.find(kUriPath)->second.get<std::string>().c_str());
  }

  int properties = IotconUtils::GetProperties(source);

  CHECK_EXIST(source, kResourceTypes);
  const auto& types_array = source.find(kResourceTypes)->second.get<picojson::array>();
  iotcon_resource_types_h resource_types = nullptr;
  auto res = IotconUtils::ArrayToTypes(types_array, &resource_types);
  if (!res) {
    return res;
  }
  SCOPE_EXIT {
    iotcon_resource_types_destroy(resource_types);
  };

  CHECK_EXIST(source, kResourceInterfaces);
  const auto& interfaces_array = source.find(kResourceInterfaces)->second.get<picojson::array>();
  iotcon_resource_interfaces_h interfaces = nullptr;
  res = IotconUtils::ArrayToInterfaces(interfaces_array, &interfaces);
  if (!res) {
    return res;
  }
  SCOPE_EXIT {
    iotcon_resource_interfaces_destroy(interfaces);
  };

  res = IotconUtils::ConvertIotconError(
      iotcon_remote_resource_create(host_address, connectivity_type, uri_path,
                                    properties, resource_types, interfaces, &((*ptr)->handle)));
  if (!res) {
    LogAndReturnTizenError(res, ("creating handle failed"));
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::RequestToJson(iotcon_request_h request,
                                               picojson::object* out) {
  ScopeLogger();

  if (request) {
    {
      // hostAddress
      char* host_address = nullptr;
      auto result = ConvertIotconError(iotcon_request_get_host_address(request, &host_address));
      if (!result || !host_address) {
        LogAndReturnTizenError(result, ("iotcon_request_get_host_address() failed"));
      }
      out->insert(std::make_pair(kHostAddress, picojson::value{host_address}));
    }

    {
      // connectivityType
      iotcon_connectivity_type_e connectivity_type = IOTCON_CONNECTIVITY_ALL;
      auto result = ConvertIotconError(iotcon_request_get_connectivity_type(request, &connectivity_type));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_connectivity_type() failed"));
      }
      out->insert(std::make_pair(kConnectivityType, picojson::value{FromConnectivityType(connectivity_type)}));
    }

    {
      // representation
      iotcon_representation_h representation = nullptr;
      auto result = ConvertIotconError(iotcon_request_get_representation(request, &representation));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_representation() failed"));
      }
      if (representation) {
        picojson::value v{picojson::object{}};
        result = RepresentationToJson(representation, &v.get<picojson::object>());
        if (!result) {
          LogAndReturnTizenError(result, ("RepresentationToJson() failed"));
        }
        out->insert(std::make_pair(kRepresentation, v));
      } else {
        LoggerD("Request doesn't have representation.");
      }
    }

    {
      // requestType
      iotcon_request_type_e request_type = IOTCON_REQUEST_UNKNOWN;
      auto result = ConvertIotconError(iotcon_request_get_request_type(request, &request_type));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_request_type() failed"));
      }
      out->insert(std::make_pair(kRequestType, picojson::value{FromRequestType(request_type)}));
    }

    {
      // options
      iotcon_options_h options = nullptr;
      auto result = ConvertIotconError(iotcon_request_get_options(request, &options));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_options() failed"));
      }
      picojson::value v{picojson::array{}};
      result = OptionsToJson(options, &v.get<picojson::array>());
      if (!result) {
        LogAndReturnTizenError(result, ("OptionsToJson() failed"));
      }
      out->insert(std::make_pair(kOptions, v));
    }

    {
      // query
      iotcon_query_h query = nullptr;
      auto result = ConvertIotconError(iotcon_request_get_query(request, &query));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_query() failed"));
      }
      picojson::value v{picojson::object{}};
      result = QueryToJson(query, &v.get<picojson::object>());
      if (!result) {
        LogAndReturnTizenError(result, ("QueryToJson() failed"));
      }
      out->insert(std::make_pair(kQuery, v));
    }

    {
      // observerId
      int observer_id = -1;
      auto result = ConvertIotconError(iotcon_request_get_observe_id(request, &observer_id));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_observe_id() failed"));
      }
      out->insert(std::make_pair(kObserverId, picojson::value{static_cast<double>(observer_id)}));
    }

    {
      // observeType
      iotcon_observe_type_e observe_type = IOTCON_OBSERVE_NO_TYPE;
      auto result = ConvertIotconError(iotcon_request_get_observe_type(request, &observe_type));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_request_get_observe_type() failed"));
      }
      out->insert(std::make_pair(kObserveType, picojson::value{FromObserveType(observe_type)}));
    }
  } else {
    LoggerW("Request handle is null, ignoring");
  }
  return TizenSuccess();
}

common::TizenResult IotconUtils::RepresentationToJson(iotcon_representation_h representation,
                                                      picojson::object* out) {
  ScopeLogger();

  if (representation) {
    {
      // uriPath
      char* uri_path = nullptr;
      auto result = ConvertIotconError(iotcon_representation_get_uri_path(representation, &uri_path));
      if (!result || !uri_path) {
        LogAndReturnTizenError(result, ("iotcon_representation_get_uri_path() failed"));
      }
      out->insert(std::make_pair(kUriPath, picojson::value{uri_path}));
    }

    {
      // resourceTypes
      iotcon_resource_types_h resource_types = nullptr;
      auto result = ConvertIotconError(iotcon_representation_get_resource_types(representation, &resource_types));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_get_resource_types() failed"));
      }
      picojson::value v{picojson::array{}};
      iotcon_resource_types_foreach(resource_types, ResourceTypeIterator, &v.get<picojson::array>());
      out->insert(std::make_pair(kResourceTypes, v));
    }

    {
      // resourceInterfaces
      iotcon_resource_interfaces_h interfaces = nullptr;
      auto result = ConvertIotconError(iotcon_representation_get_resource_interfaces(representation, &interfaces));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_get_resource_interfaces() failed"));
      }
      picojson::array js_interfaces;
      result = InterfacesToArray(interfaces, &js_interfaces);
      if (!result) {
        LogAndReturnTizenError(result, ("InterfacesToArray() failed"));
      }
      out->insert(std::make_pair(kResourceInterfaces, picojson::value{js_interfaces}));
    }

    {
      // states
      iotcon_attributes_h attributes = nullptr;
      auto result = ConvertIotconError(iotcon_representation_get_attributes(representation, &attributes));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_get_attributes() failed"));
      }
      picojson::value v{picojson::object{}};
      result = AttributesToJson(attributes, &v.get<picojson::object>());
      if (!result) {
        LogAndReturnTizenError(result, ("AttributesToJson() failed"));
      }
      out->insert(std::make_pair(kStates, v));
    }

    {
      // representations
      picojson::value v{picojson::array{}};
      auto result = ConvertIotconError(iotcon_representation_foreach_children(representation, [](iotcon_representation_h child, void* user_data) -> bool {
        auto arr = static_cast<picojson::array*>(user_data);
        arr->push_back(picojson::value{picojson::object{}});
        auto result = RepresentationToJson(child, &arr->back().get<picojson::object>());
        if (!result) {
          LoggerE("Failed to convert child representation");
        }
        // always continue with iteration
        return true;
      }, &v.get<picojson::array>()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_foreach_children() failed"));
      }
      out->insert(std::make_pair(kRepresentations, v));
    }
  } else {
    LoggerW("Representation handle is null, ignoring");
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::AttributesToJson(iotcon_attributes_h attributes,
                                             picojson::object* out) {
  ScopeLogger();

  if (attributes) {
    auto result = ConvertIotconError(iotcon_attributes_foreach(attributes, [](iotcon_attributes_h attributes, const char* key, void* user_data) -> bool {
      iotcon_type_e type = IOTCON_TYPE_NONE;
      auto result = ConvertIotconError(iotcon_attributes_get_type(attributes, key, &type));

      if (result) {
        auto out = static_cast<picojson::object*>(user_data);

        switch (type) {
          case IOTCON_TYPE_NONE:
            LoggerE("Key %s has type NONE", key);
            break;

          case IOTCON_TYPE_INT:
          {
            int value = 0;
            result = ConvertIotconError(iotcon_attributes_get_int(attributes, key, &value));
            if (result) {
              out->insert(std::make_pair(key, picojson::value{static_cast<double>(value)}));
            } else {
              LoggerE("iotcon_attributes_get_int() failed");
            }
          }
          break;

          case IOTCON_TYPE_BOOL:
          {
            bool value = false;
            result = ConvertIotconError(iotcon_attributes_get_bool(attributes, key, &value));
            if (result) {
              out->insert(std::make_pair(key, picojson::value{value}));
            } else {
              LoggerE("iotcon_attributes_get_bool() failed");
            }
          }
          break;

          case IOTCON_TYPE_DOUBLE:
          {
            double value = 0.0;
            result = ConvertIotconError(iotcon_attributes_get_double(attributes, key, &value));
            if (result) {
              out->insert(std::make_pair(key, picojson::value{value}));
            } else {
              LoggerE("iotcon_attributes_get_double() failed");
            }
          }
          break;

          case IOTCON_TYPE_STR:
          {
            char* value = nullptr;
            result = ConvertIotconError(iotcon_attributes_get_str(attributes, key, &value));
            if (result && value) {
              out->insert(std::make_pair(key, picojson::value{value}));
            } else {
              LoggerE("iotcon_attributes_get_str() failed");
            }
          }
          break;

          case IOTCON_TYPE_BYTE_STR:
          {
            unsigned char* value = nullptr;
            int length = 0;
            result = ConvertIotconError(iotcon_attributes_get_byte_str(attributes, key, &value, &length));

            if (result && length) {
              std::unique_ptr<char[]> data{new char[2 * length]};
              common::tools::BinToHex(value, length, data.get(), 2 * length);
              out->insert(std::make_pair(key, picojson::value{kHexPrefix + data.get()}));
            } else {
              LoggerE("iotcon_attributes_get_byte_str() failed");
            }
          }
          break;

          case IOTCON_TYPE_NULL:
            out->insert(std::make_pair(key, picojson::value{}));
            break;

          case IOTCON_TYPE_LIST:
          {
            iotcon_list_h list = nullptr;
            result = ConvertIotconError(iotcon_attributes_get_list(attributes, key, &list));
            if (result) {
              picojson::value value{picojson::array{}};

              result = StateListToJson(list, &value.get<picojson::array>());
              if (result) {
                out->insert(std::make_pair(key, picojson::value{value}));
              } else {
                LoggerE("StateListToJson() failed");
              }
            } else {
              LoggerE("iotcon_attributes_get_list() failed");
            }
          }
          break;

          case IOTCON_TYPE_ATTRIBUTES:
          {
            iotcon_attributes_h child = nullptr;
            result = ConvertIotconError(iotcon_attributes_get_attributes(attributes, key, &child));
            if (result) {
              picojson::value value{picojson::object{}};

              result = AttributesToJson(child, &value.get<picojson::object>());
              if (result) {
                out->insert(std::make_pair(key, picojson::value{value}));
              } else {
                LoggerE("AttributesToJson() failed");
              }
            } else {
              LoggerE("iotcon_attributes_get_attributes() failed");
            }
          }
          break;
        }
      } else {
        LoggerE("iotcon_attributes_get_type() failed");
      }

      // always continue with iteration
      return true;
    }, out));

    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_attributes_foreach() failed"));
    }
  } else {
    LoggerW("attributes handle is null, ignoring");
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::StateListToJson(iotcon_list_h list,
                                                 picojson::array* out) {
  ScopeLogger();

  if (list) {
    iotcon_type_e type = IOTCON_TYPE_NONE;
    auto result = ConvertIotconError(iotcon_list_get_type(list, &type));

    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_list_get_type() failed"));
    }

    switch (type) {
      case IOTCON_TYPE_NONE:
        LoggerE("List has type NONE");
        break;

      case IOTCON_TYPE_INT:
        result = ConvertIotconError(iotcon_list_foreach_int(list, [](int, int value, void* user_data) -> bool {
          auto out = static_cast<picojson::array*>(user_data);
          out->push_back(picojson::value{static_cast<double>(value)});
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_int() failed"));
        }
        break;

      case IOTCON_TYPE_BOOL:
        result = ConvertIotconError(iotcon_list_foreach_bool(list, [](int, bool value, void* user_data) -> bool {
          auto out = static_cast<picojson::array*>(user_data);
          out->push_back(picojson::value{value});
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_bool() failed"));
        }
        break;

      case IOTCON_TYPE_DOUBLE:
        result = ConvertIotconError(iotcon_list_foreach_double(list, [](int, double value, void* user_data) -> bool {
          auto out = static_cast<picojson::array*>(user_data);
          out->push_back(picojson::value{value});
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_double() failed"));
        }
        break;

      case IOTCON_TYPE_STR:
        result = ConvertIotconError(iotcon_list_foreach_str(list, [](int, const char* value, void* user_data) -> bool {
          if (value) {
            auto out = static_cast<picojson::array*>(user_data);
            out->push_back(picojson::value{value});
          }
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_str() failed"));
        }
        break;

      case IOTCON_TYPE_BYTE_STR:
        result = ConvertIotconError(iotcon_list_foreach_byte_str(list, [](int, const unsigned char* value, int length, void* user_data) -> bool {
          if (length) {
            std::unique_ptr<char[]> data{new char[2 * length]};
            common::tools::BinToHex(value, length, data.get(), 2 * length);

            auto out = static_cast<picojson::array*>(user_data);
            out->push_back(picojson::value{kHexPrefix + data.get()});
          }
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_str() failed"));
        }
        break;

      case IOTCON_TYPE_NULL:
        LoggerE("List has type NULL");
        break;

      case IOTCON_TYPE_LIST:
        result = ConvertIotconError(iotcon_list_foreach_list(list, [](int, iotcon_list_h list, void* user_data) -> bool {
          picojson::value value{picojson::array{}};
          auto result = StateListToJson(list, &value.get<picojson::array>());
          if (result) {
            auto out = static_cast<picojson::array*>(user_data);
            out->push_back(picojson::value{value});
          } else {
            LoggerE("StateListToJson() failed");
          }
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_list() failed"));
        }
        break;

      case IOTCON_TYPE_ATTRIBUTES:
        result = ConvertIotconError(iotcon_list_foreach_attributes(list, [](int, iotcon_attributes_h attributes, void* user_data) -> bool {
          picojson::value value{picojson::object{}};
          auto result = AttributesToJson(attributes, &value.get<picojson::object>());
          if (result) {
            auto out = static_cast<picojson::array*>(user_data);
            out->push_back(picojson::value{value});
          } else {
            LoggerE("AttributesToJson() failed");
          }
          // always continue with iteration
          return true;
        }, out));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_list_foreach_attributes() failed"));
        }
        break;
    }
  } else {
    LoggerW("List handle is null, ignoring");
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::OptionsToJson(iotcon_options_h  options,
                                               picojson::array* out) {
  ScopeLogger();

  if (options) {
    auto result = ConvertIotconError(iotcon_options_foreach(options, [](unsigned short id, const char *data, void* user_data) -> bool {
      if (data) {
        picojson::value v{picojson::object{}};
        auto& obj = v.get<picojson::object>();

        obj.insert(std::make_pair(kOptionsId, picojson::value{static_cast<double>(id)}));
        obj.insert(std::make_pair(kOptionsData, picojson::value{data}));

        auto out = static_cast<picojson::array*>(user_data);
        out->push_back(v);
      }
      // always continue with iteration
      return true;
    }, out));

    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_options_foreach() failed"));
    }
  } else {
    LoggerW("Options handle is null, ignoring");
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::QueryToJson(iotcon_query_h query,
                                             picojson::object* out) {
  ScopeLogger();

  if (query) {
    {
      // resourceType
      char* resource_type = nullptr;
      auto result = ConvertIotconError(iotcon_query_get_resource_type(query, &resource_type));
      if (!result || !resource_type) {
        LogAndReturnTizenError(result, ("iotcon_query_get_resource_type() failed"));
      }
      out->insert(std::make_pair(kResourceType, picojson::value{resource_type}));
    }

    {
      // resourceInterface
      char* interface = nullptr;
      auto result = ConvertIotconError(iotcon_query_get_interface(query, &interface));
      if (!result || !interface) {
        LogAndReturnTizenError(result, ("iotcon_query_get_interface() failed"));
      }
      out->insert(std::make_pair(kResourceInterface, picojson::value{interface}));
    }

    {
      // filter
      picojson::value v{picojson::object{}};
      auto result = ConvertIotconError(iotcon_query_foreach(query, [](const char* key, const char* value, void* user_data) -> bool {
        if (key && value) {
          auto obj = static_cast<picojson::object*>(user_data);
          obj->insert(std::make_pair(key, picojson::value{value}));
        }
        // always continue with iteration
        return true;
      }, &v.get<picojson::object>()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_query_foreach() failed"));
      }
      out->insert(std::make_pair(kFilter, v));
    }
  } else {
    LoggerW("Query handle is null, ignoring");
  }
  return TizenSuccess();
}

common::TizenResult IotconUtils::QueryFromJson(const picojson::object& source, iotcon_query_h* res) {
  ScopeLogger();
  iotcon_query_h query = nullptr;
  auto result = ConvertIotconError(iotcon_query_create(&query));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_query_create() failed"));
  }
  std::unique_ptr<std::remove_pointer<iotcon_query_h>::type, void(*)(iotcon_query_h)>
  query_ptr(query, &iotcon_query_destroy); // automatically release the memory
  {
    // resourceType
    auto it = source.find(kResourceType);
    if (source.end() != it && it->second.is<std::string>()) {
      const char* resource_type = it->second.get<std::string>().c_str();
      auto result = ConvertIotconError(iotcon_query_set_resource_type(query, resource_type));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_query_set_resource_type() failed"));
      }
    }
  }

  {
    // resourceInterface
    auto it = source.find(kResourceInterface);
    if (source.end() != it && it->second.is<std::string>()) {
      auto& interface = it->second.get<std::string>();
      auto result = ConvertIotconError(iotcon_query_set_interface(query, interface.c_str()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_query_set_interface() failed"));
      }
    }
  }

  {
    // filter
    auto it = source.find(kFilter);
    if (source.end() != it) {
      const auto& filter = IotconUtils::GetArg(source, kFilter);
      if (filter.is<picojson::object>()) {
        const auto& filter_obj = filter.get<picojson::object>();
        for (const auto it : filter_obj) {
          if (it.second.is<std::string>()){
            const std::string& key = it.first;
            const std::string& value = it.second.get<std::string>();
            LoggerD("key: %s  ----  value: %s", key.c_str(), value.c_str());

            auto result = ConvertIotconError(iotcon_query_add(query, key.c_str(), value.c_str()));
            if (!result) {
              LogAndReturnTizenError(result, ("iotcon_query_add() failed"));
            }
          }
        }
      }
    }
  }

  *res = query_ptr.release();
  return TizenSuccess();
}

common::TizenResult IotconUtils::ResponseToJson(iotcon_response_h handle,
                                                picojson::object* res) {
  ScopeLogger();

  if (handle) {
    {
      // ResponseResult result
      iotcon_response_result_e response = IOTCON_RESPONSE_ERROR;
      auto result = ConvertIotconError(iotcon_response_get_result(handle, &response));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_response_get_result() failed"));
      }
      std::string result_str = FromResponseResultType(response);
      res->insert(std::make_pair(kResult, picojson::value{result_str}));
    }

    {
      // Representation representation
      iotcon_representation_h repr = nullptr;
      auto result = ConvertIotconError(iotcon_response_get_representation(handle, &repr));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_response_get_representation() failed"));
      }
      picojson::value repr_json{picojson::object{}};
      result = RepresentationToJson(repr, &repr_json.get<picojson::object>());
      if (!result) {
        LogAndReturnTizenError(result, ("RepresentationToJson() failed"));
      }
      res->insert(std::make_pair(kRepresentation, repr_json));
    }

    {
      // IotconOption[]? options
      iotcon_options_h options = nullptr;
      int native_ret = iotcon_response_get_options(handle, &options);
      auto result = ConvertIotconError(native_ret);
      if (!result && IOTCON_ERROR_NO_DATA != native_ret) {
        LogAndReturnTizenError(result, ("iotcon_response_get_options() failed"));
      }
      if (options) {
        picojson::value opt_json{picojson::array{}};
        result = OptionsToJson(options, &opt_json.get<picojson::array>());
        if (!result) {
          LogAndReturnTizenError(result, ("OptionsToJson() failed"));
        }
        res->insert(std::make_pair(kOptions, opt_json));
      } else {
        res->insert(std::make_pair(kOptions, picojson::value{}));
      }
    }
  } else {
    LoggerW("Response handle is null, ignoring");
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::RepresentationFromResource(const ResourceInfoPtr& resource,
                                                            const picojson::value& states,
                                                            iotcon_representation_h* out) {
  ScopeLogger();

  iotcon_representation_h representation = nullptr;

  auto result = IotconUtils::ConvertIotconError(iotcon_representation_create(&representation));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_representation_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_representation_h>::type, void(*)(iotcon_representation_h)> ptr{representation, &iotcon_representation_destroy};

  {
    char* uri_path = nullptr;
    result = IotconUtils::ConvertIotconError(iotcon_resource_get_uri_path(resource->handle, &uri_path));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_get_uri_path() failed"));
    }
    result = IotconUtils::ConvertIotconError(iotcon_representation_set_uri_path(representation, uri_path));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_representation_set_uri_path() failed"));
    }
  }

  {
    iotcon_resource_types_h types = nullptr;
    result = IotconUtils::ConvertIotconError(iotcon_resource_get_types(resource->handle, &types));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_get_types() failed"));
    }
    result = IotconUtils::ConvertIotconError(iotcon_representation_set_resource_types(representation, types));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_representation_set_resource_types() failed"));
    }
  }

  {
    iotcon_resource_interfaces_h intrefaces = nullptr;
    result = IotconUtils::ConvertIotconError(iotcon_resource_get_interfaces(resource->handle, &intrefaces));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_get_interfaces() failed"));
    }
    result = IotconUtils::ConvertIotconError(iotcon_representation_set_resource_interfaces(representation, intrefaces));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_representation_set_resource_interfaces() failed"));
    }
  }

  {
    auto& state = states.get(std::to_string(resource->id));
    if (state.is<picojson::object>()) {
      iotcon_attributes_h attributes_handle = nullptr;
      result = IotconUtils::StateFromJson(state.get<picojson::object>(), &attributes_handle);
      if (!result) {
        LogAndReturnTizenError(result, ("StateFromJson() failed"));
      }
      SCOPE_EXIT {
        iotcon_attributes_destroy(attributes_handle);
      };
      result = IotconUtils::ConvertIotconError(iotcon_representation_set_attributes(representation, attributes_handle));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_set_attributes() failed"));
      }
    }
  }

  {
    unsigned int children = 0;
    result = IotconUtils::ConvertIotconError(iotcon_resource_get_child_count(resource->handle, &children));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_get_child_count() failed"));
    }

    for (unsigned int i = 0; i < children; ++i) {
      iotcon_resource_h child = nullptr;
      result = IotconUtils::ConvertIotconError(iotcon_resource_get_nth_child(resource->handle, i, &child));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_resource_get_nth_child() failed"));
      }

      ResourceInfoPtr child_resource;
      result = IotconServerManager::GetInstance().GetResourceByHandle(child, &child_resource);
      if (!result) {
        LogAndReturnTizenError(result, ("GetResourceByHandle() failed"));
      }

      iotcon_representation_h child_representation = nullptr;
      result = RepresentationFromResource(child_resource, states, &child_representation);
      if (!result) {
        LogAndReturnTizenError(result, ("RepresentationFromResource() failed"));
      }
      SCOPE_EXIT {
        iotcon_representation_destroy(child_representation);
      };
      result = IotconUtils::ConvertIotconError(iotcon_representation_add_child(representation, child_representation));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_add_child() failed"));
      }
    }
  }

  *out = ptr.release();
  return TizenSuccess();
}

common::TizenResult IotconUtils::StateFromJson(const picojson::object& s,
                                               iotcon_attributes_h* out) {
  ScopeLogger();

  iotcon_attributes_h attributes = nullptr;

  auto result = IotconUtils::ConvertIotconError(iotcon_attributes_create(&attributes));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_attributes_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_attributes_h>::type, void(*)(iotcon_attributes_h)> ptr{attributes, &iotcon_attributes_destroy};

  for (const auto& property : s) {
    const auto& key = property.first;

    if (property.second.is<picojson::null>()) {
      result = IotconUtils::ConvertIotconError(iotcon_attributes_add_null(attributes, key.c_str()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_attributes_add_null() failed"));
      }
    } else if (property.second.is<bool>()) {
      result = IotconUtils::ConvertIotconError(iotcon_attributes_add_bool(attributes, key.c_str(), property.second.get<bool>()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_attributes_add_bool() failed"));
      }
    } else if (property.second.is<double>()) {
      result = IotconUtils::ConvertIotconError(iotcon_attributes_add_double(attributes, key.c_str(), property.second.get<double>()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_attributes_attributes_double() failed"));
      }
    } else if (property.second.is<std::string>()) {
      const auto& value = property.second.get<std::string>();

      if (0 == value.find(kHexPrefix)) {
        auto data = value.c_str() + kHexPrefix.length(); // skip prefix
        auto size = value.length() - kHexPrefix.length();
        auto length = size / 2;
        std::unique_ptr<unsigned char[]> hex{new unsigned char[length]};
        common::tools::HexToBin(data, size, hex.get(), length);
        result = IotconUtils::ConvertIotconError(iotcon_attributes_add_byte_str(attributes, key.c_str(), hex.get(), length));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_attributes_add_byte_str() failed"));
        }
      } else {
        result = IotconUtils::ConvertIotconError(iotcon_attributes_add_str(attributes, key.c_str(), const_cast<char*>(value.c_str())));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_attributes_add_str() failed"));
        }
      }
    } else if (property.second.is<picojson::array>()) {
      iotcon_list_h list = nullptr;
      result = StateListFromJson(property.second.get<picojson::array>(), &list);
      if (!result) {
        LogAndReturnTizenError(result, ("StateListFromJson() failed"));
      }
      if (list) {
        SCOPE_EXIT {
          iotcon_list_destroy(list);
        };
        result = IotconUtils::ConvertIotconError(iotcon_attributes_add_list(attributes, key.c_str(), list));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_attributes_add_list() failed"));
        }
      }
    } else if (property.second.is<picojson::object>()) {
      iotcon_attributes_h sub_attributes = nullptr;
      result = StateFromJson(property.second.get<picojson::object>(), &sub_attributes);
      if (!result) {
        LogAndReturnTizenError(result, ("StateFromJson() failed"));
      }
      SCOPE_EXIT {
        iotcon_attributes_destroy(sub_attributes);
      };
      result = IotconUtils::ConvertIotconError(iotcon_attributes_add_attributes(attributes, key.c_str(), sub_attributes));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_attributes_add_attributes() failed"));
      }
    }
  }

  *out = ptr.release();
  return TizenSuccess();
}

common::TizenResult IotconUtils::StateListFromJson(const picojson::array& l,
                                                   iotcon_list_h* out) {
  ScopeLogger();

  iotcon_list_h list = nullptr;
  iotcon_type_e type = IOTCON_TYPE_NONE;

  // check first element in array for type
  if (l.size() > 0) {
    if (l[0].is<bool>()) {
      type = IOTCON_TYPE_BOOL;
    } else if (l[0].is<double>()) {
      type = IOTCON_TYPE_DOUBLE;
    } else if (l[0].is<std::string>()) {
      if (0 == l[0].get<std::string>().find(kHexPrefix)) {
        type = IOTCON_TYPE_BYTE_STR;
      } else {
        type = IOTCON_TYPE_STR;
      }
    } else if (l[0].is<picojson::array>()) {
      type = IOTCON_TYPE_LIST;
    } else if (l[0].is<picojson::object>()) {
      type = IOTCON_TYPE_ATTRIBUTES;
    }
  }

  if (IOTCON_TYPE_NONE == type) {
    LoggerD("Empty list");
    return TizenSuccess();
  }

  auto result = IotconUtils::ConvertIotconError(iotcon_list_create(type, &list));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_list_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_list_h>::type, void(*)(iotcon_list_h)> ptr{list, &iotcon_list_destroy};

  int position = 0;

  // we're ignoring values with wrong type
  for (const auto& v : l) {
    switch (type) {
      case IOTCON_TYPE_BOOL:
        if (v.is<bool>()) {
          result = IotconUtils::ConvertIotconError(iotcon_list_add_bool(list, v.get<bool>(), position++));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_list_add_bool() failed"));
          }
        }
        break;

      case IOTCON_TYPE_DOUBLE:
        if (v.is<double>()) {
          result = IotconUtils::ConvertIotconError(iotcon_list_add_double(list, v.get<double>(), position++));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_list_add_double() failed"));
          }
        }
        break;

      case IOTCON_TYPE_BYTE_STR:
        if (v.is<std::string>()) {
          const auto& str = v.get<std::string>();
          if (0 == str.find(kHexPrefix)) {
            auto data = str.c_str() + kHexPrefix.length(); // skip prefix
            auto size = str.length() - kHexPrefix.length();
            auto length = size / 2;
            std::unique_ptr<unsigned char[]> hex{new unsigned char[length]};
            common::tools::HexToBin(data, size, hex.get(), length);
            result = IotconUtils::ConvertIotconError(iotcon_list_add_byte_str(list, hex.get(), length, position++));
            if (!result) {
              LogAndReturnTizenError(result, ("iotcon_list_add_byte_str() failed"));
            }
          }
        }
        break;

      case IOTCON_TYPE_STR:
        if (v.is<std::string>()) {
          result = IotconUtils::ConvertIotconError(iotcon_list_add_str(list, const_cast<char*>(v.get<std::string>().c_str()), position++));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_list_add_str() failed"));
          }
        }
        break;

      case IOTCON_TYPE_LIST:
        if (v.is<picojson::array>()) {
          iotcon_list_h sub_list = nullptr;
          result = StateListFromJson(v.get<picojson::array>(), &sub_list);
          if (!result) {
            LogAndReturnTizenError(result, ("StateListFromJson() failed"));
          }
          SCOPE_EXIT {
            iotcon_list_destroy(sub_list);
          };
          result = IotconUtils::ConvertIotconError(iotcon_list_add_list(list, sub_list, position++));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_list_add_list() failed"));
          }
        }
        break;

      case IOTCON_TYPE_ATTRIBUTES:
        if (v.is<picojson::object>()) {
          iotcon_attributes_h attributes = nullptr;
          result = StateFromJson(v.get<picojson::object>(), &attributes);
          if (!result) {
            LogAndReturnTizenError(result, ("StateFromJson() failed"));
          }
          SCOPE_EXIT {
            iotcon_attributes_destroy(attributes);
          };
          result = IotconUtils::ConvertIotconError(iotcon_list_add_attributes(list, attributes, position++));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_list_add_attributes() failed"));
          }
        }
        break;

      default:
        // should not happen
        LoggerE("Unexpected type: %d", type);
        return common::UnknownError("Unexpected list type");
    }
  }

  *out = ptr.release();
  return TizenSuccess();
}

common::TizenResult IotconUtils::PresenceResponseToJson(
    iotcon_presence_response_h presence, picojson::object* out) {
  ScopeLogger();

  if (presence) {
    {
      // hostAddress
      char* host = nullptr;
      auto result = ConvertIotconError(iotcon_presence_response_get_host_address(presence,
                                                                                 &host));
      if (!result || !host) {
        LogAndReturnTizenError(result, ("iotcon_presence_response_get_host_address() failed"));
      }
      out->insert(std::make_pair(kHostAddress, picojson::value{std::string(host)}));
    }

    {
      // connectivityType
      iotcon_connectivity_type_e con_type = IOTCON_CONNECTIVITY_IPV4;
      auto result = ConvertIotconError(iotcon_presence_response_get_connectivity_type(presence,
                                                                                      &con_type));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_presence_response_get_connectivity_type() failed"));
      }
      out->insert(std::make_pair(kConnectivityType, picojson::value{
        FromConnectivityType(con_type)}));
    }

    {
      // resourceType
      char* resource_type = nullptr;
      auto result = ConvertIotconError(iotcon_presence_response_get_resource_type(presence,
                                                                                  &resource_type));
      if (!result || !resource_type) {
        LoggerE("iotcon_presence_response_get_resource_type() failed");
        out->insert(std::make_pair(kResourceType, picojson::value()));
      } else {
        out->insert(std::make_pair(kResourceType, picojson::value{std::string(resource_type)}));
      }
    }

    // resultType
    iotcon_presence_result_e result_type = IOTCON_PRESENCE_OK;
    {
      auto result = ConvertIotconError(iotcon_presence_response_get_result(presence,
                                                                           &result_type));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_presence_response_get_result() failed"));
      }

      out->insert(std::make_pair(kResultType, picojson::value{
        FromPresenceResponseResultType(result_type)}));
    }

    {
      // triggerType
      iotcon_presence_trigger_e trigger_type = IOTCON_PRESENCE_RESOURCE_CREATED;
      if (IOTCON_PRESENCE_OK == result_type) {
        auto result = ConvertIotconError(iotcon_presence_response_get_trigger(presence,
                                                                              &trigger_type));
        if (!result) {
          LoggerE("iotcon_presence_response_get_trigger() failed");
          out->insert(std::make_pair(kTriggerType, picojson::value()));
        } else {
          out->insert(std::make_pair(kTriggerType, picojson::value{FromPresenceTriggerType(
              trigger_type)}));
        }
      } else {
        out->insert(std::make_pair(kTriggerType, picojson::value()));
      }
    }
  } else {
    LoggerW("Presence handle is null, ignoring");
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::ExtractFromPresenceEvent(const PresenceEventPtr& pointer,
                                                          char** host,
                                                          iotcon_connectivity_type_e* con_type,
                                                          char** resource_type) {
  ScopeLogger();

  auto result = ConvertIotconError(iotcon_presence_get_host_address(pointer->handle,
                                                                    host));
  if (!result) {
   LogAndReturnTizenError(result, ("Gathering presence host address failed"));
  }

  result = ConvertIotconError(iotcon_presence_get_connectivity_type(pointer->handle,
                                                                    con_type));
  if (!result) {
   LogAndReturnTizenError(result, ("Gathering presence connectivity type failed"));
  }

  result = ConvertIotconError(iotcon_presence_get_resource_type(pointer->handle,
                                                                resource_type));
  if (!result) {
   LogAndReturnTizenError(result, ("Gathering presence resource type failed"));
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::OptionsFromJson(const picojson::array& o,
                                                 iotcon_options_h* out) {
  ScopeLogger();

  iotcon_options_h options = nullptr;

  auto result = ConvertIotconError(iotcon_options_create(&options));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_options_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_options_h>::type, void(*)(iotcon_options_h)> ptr{options, &iotcon_options_destroy};

  // we ignore values with incorrect types
  // TODO: should we convert them in JS?
  for (const auto& option : o) {
    if (option.is<picojson::object>()) {
      const auto& js_id = option.get(kOptionsId);
      const auto& js_data = option.get(kOptionsData);

      if (js_id.is<double>() && js_data.is<std::string>()) {
        result = ConvertIotconError(iotcon_options_add(options, js_id.get<double>(), js_data.get<std::string>().c_str()));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_options_add() failed"));
        }
      }
    }
  }

  *out = ptr.release();
  return TizenSuccess();
}

common::TizenResult IotconUtils::RepresentationFromJson(const picojson::object& r,
                                                        iotcon_representation_h* out) {
  ScopeLogger();

  iotcon_representation_h representation = nullptr;

  auto result = ConvertIotconError(iotcon_representation_create(&representation));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_representation_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_representation_h>::type, void(*)(iotcon_representation_h)> ptr{representation, &iotcon_representation_destroy};

  {
    const auto& uri_path = r.find(kUriPath);
    if (r.end() != uri_path && uri_path->second.is<std::string>()) {
      result = ConvertIotconError(iotcon_representation_set_uri_path(representation, uri_path->second.get<std::string>().c_str()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_set_uri_path() failed"));
      }
    } else {
      return LogAndCreateTizenError(TypeMismatchError, "Representation object needs to have an uriPath attribute which is a string.");
    }
  }

  {
    const auto& resource_types = r.find(kResourceTypes);
    if (r.end() != resource_types && resource_types->second.is<picojson::array>()) {
      iotcon_resource_types_h types = nullptr;

      result = ArrayToTypes(resource_types->second.get<picojson::array>(), &types);
      if (!result) {
        LogAndReturnTizenError(result, ("ArrayToTypes() failed"));
      }
      SCOPE_EXIT {
        iotcon_resource_types_destroy(types);
      };

      result = ConvertIotconError(iotcon_representation_set_resource_types(representation, types));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_set_resource_types() failed"));
      }
    } else {
      return LogAndCreateTizenError(TypeMismatchError, "Representation object needs to have a resourceTypes attribute which is an array.");
    }
  }

  {
    const auto& resource_interfaces = r.find(kResourceInterfaces);
    if (r.end() != resource_interfaces && resource_interfaces->second.is<picojson::array>()) {
      iotcon_resource_interfaces_h interfaces = nullptr;

      result = ArrayToInterfaces(resource_interfaces->second.get<picojson::array>(), &interfaces);
      if (!result) {
        LogAndReturnTizenError(result, ("ArrayToInterfaces() failed"));
      }
      SCOPE_EXIT {
        iotcon_resource_interfaces_destroy(interfaces);
      };
      result = ConvertIotconError(iotcon_representation_set_resource_interfaces(representation, interfaces));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_set_resource_interfaces() failed"));
      }
    } else {
      return LogAndCreateTizenError(TypeMismatchError, "Representation object needs to have a resourceInterfaces attribute which is an array.");
    }
  }

  {
    const auto& states = r.find(kStates);
    if (r.end() != states && states->second.is<picojson::object>()) {
      iotcon_attributes_h s = nullptr;

      result = StateFromJson(states->second.get<picojson::object>(), &s);
      if (!result) {
        LogAndReturnTizenError(result, ("StateFromJson() failed"));
      }
      SCOPE_EXIT {
        iotcon_attributes_destroy(s);
      };

      result = ConvertIotconError(iotcon_representation_set_attributes(representation, s));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_set_attributes() failed"));
      }
    }
  }

  {
    const auto& representations = r.find(kRepresentations);
    if (r.end() != representations && representations->second.is<picojson::array>()) {
      for (const auto& js_child : representations->second.get<picojson::array>()) {
        if (js_child.is<picojson::object>()) {
          iotcon_representation_h child = nullptr;

          result = RepresentationFromJson(js_child.get<picojson::object>(), &child);
          if (!result) {
            LogAndReturnTizenError(result, ("RepresentationFromJson() failed"));
          }
          SCOPE_EXIT {
            iotcon_representation_destroy(child);
          };

          result = ConvertIotconError(iotcon_representation_add_child(representation, child));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_representation_add_child() failed"));
          }
        } else {
          return LogAndCreateTizenError(TypeMismatchError, "The Representation.representations attribute needs to be an array of Representation objects.");
        }
      }
    }
  }

  *out = ptr.release();
  return TizenSuccess();
}

common::TizenResult IotconUtils::PlatformInfoGetProperty(iotcon_platform_info_h platform,
                                                         iotcon_platform_info_e property_e,
                                                         const std::string& name,
                                                         picojson::object* out) {
  ScopeLogger();

  char* property = nullptr;
  auto result = ConvertIotconError(iotcon_platform_info_get_property(platform,
                                                                     property_e,
                                                                     &property));
  if (!result || !property) {
    LogAndReturnTizenError(result, ("iotcon_platform_info_get_property() failed"));
  }
  out->insert(std::make_pair(name, picojson::value{property}));

  return TizenSuccess();
}

common::TizenResult IotconUtils::PlatformInfoToJson(iotcon_platform_info_h platform,
                                                    picojson::object* out) {
  ScopeLogger();

  {
    // platformId
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_ID,
                                          kPlatformId, out);
    if (!result) {
      return result;
    }
  }

  {
    // manufacturerName
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_MANUF_NAME,
                                          kManufacturerName, out);
    if (!result) {
      return result;
    }
  }

  {
    // manufacturerUrl
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_MANUF_URL,
                                          kManufacturerUrl, out);
    if (!result) {
      return result;
    }
  }

  {
    // modelNumber
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_MODEL_NUMBER,
                                          kModelNumber, out);
    if (!result) {
      return result;
    }
  }

  {
    // manufactureDate
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_DATE_OF_MANUF,
                                          kManufactureDate, out);
    if (!result) {
      return result;
    }
  }

  {
    // platformVersion
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_PLATFORM_VER,
                                          kPlatformVersion, out);
    if (!result) {
      return result;
    }
  }

  {
    // operatingSystemVersion
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_OS_VER,
                                          kOperatingSystemVersion, out);
    if (!result) {
      return result;
    }
  }

  {
    // hardwareVersion
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_HARDWARE_VER,
                                          kHardwareVersion, out);
    if (!result) {
      return result;
    }
  }

  {
    // firmwareVersion
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_FIRMWARE_VER,
                                          kFirmwareVersion, out);
    if (!result) {
      return result;
    }
  }

  {
    // supportUrl
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_SUPPORT_URL,
                                          kSupportUrl, out);
    if (!result) {
      return result;
    }
  }

  {
    // systemTime
    auto result = PlatformInfoGetProperty(platform, IOTCON_PLATFORM_INFO_SYSTEM_TIME,
                                          kSystemTime, out);
    if (!result) {
      return result;
    }
  }
  return TizenSuccess();
}

common::TizenResult IotconUtils::DeviceInfoGetProperty(iotcon_device_info_h device,
                                                       iotcon_device_info_e property_e,
                                                       const std::string& name,
                                                       picojson::object* out) {
  ScopeLogger();

  char* property = nullptr;
  auto result = ConvertIotconError(iotcon_device_info_get_property(device,
                                                                   property_e,
                                                                   &property));
  if (!result || !property) {
    LogAndReturnTizenError(result, ("iotcon_device_info_get_property() failed"));
  }
  out->insert(std::make_pair(name, picojson::value{property}));

  return TizenSuccess();
}

common::TizenResult IotconUtils::DeviceInfoToJson(iotcon_device_info_h device,
                                                  picojson::object* out) {
  ScopeLogger();

  {
    // deviceName
    auto result = DeviceInfoGetProperty(device, IOTCON_DEVICE_INFO_NAME,
                                        kDeviceName, out);
    if (!result) {
      return result;
    }
  }

  {
    // specVersion
    auto result = DeviceInfoGetProperty(device, IOTCON_DEVICE_INFO_SPEC_VER,
                                        kSpecVersion, out);
    if (!result) {
      return result;
    }
  }

  {
    // oicDeviceId
    auto result = DeviceInfoGetProperty(device, IOTCON_DEVICE_INFO_ID,
                                        kOicDeviceId, out);
    if (!result) {
      return result;
    }
  }

  {
    // dataModelVersion
    auto result = DeviceInfoGetProperty(device, IOTCON_DEVICE_INFO_DATA_MODEL_VER,
                                        kDataModelVersion, out);
    if (!result) {
      return result;
    }
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::ConvertIotconError(int error) {
  switch (error) {
    case IOTCON_ERROR_NONE:
      return common::TizenSuccess();

    case IOTCON_ERROR_IO_ERROR:
      return common::IoError(error);

    case IOTCON_ERROR_PERMISSION_DENIED:
      return common::SecurityError(error);

    case IOTCON_ERROR_NOT_SUPPORTED:
      return common::NotSupportedError(error);

    case IOTCON_ERROR_INVALID_PARAMETER:
      return common::InvalidValuesError(error);

    case IOTCON_ERROR_NO_DATA:
      return common::NotFoundError(error);

    case IOTCON_ERROR_TIMEOUT:
      return common::TimeoutError(error);

    case IOTCON_ERROR_INVALID_TYPE:
      return common::TypeMismatchError(error);

    case IOTCON_ERROR_ALREADY:
      return common::InvalidStateError(error);

    case IOTCON_ERROR_OUT_OF_MEMORY:
    case IOTCON_ERROR_IOTIVITY:
    case IOTCON_ERROR_REPRESENTATION:
    case IOTCON_ERROR_SYSTEM:
    default:
      return common::AbortError(error);
  }
}

#define X(v, s) case v: return s;
#define XD(v, s) \
  default: \
    LoggerE("Unknown value: %d, returning default: %s", e, s); \
    return s;

std::string IotconUtils::FromConnectivityType(iotcon_connectivity_type_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_CONNECTIVITY_TYPE_E
  }
}

std::string IotconUtils::FromRequestType(iotcon_request_type_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_REQUEST_TYPE_E
  }
}

std::string IotconUtils::FromObserveType(iotcon_observe_type_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_OBSERVE_TYPE_E
  }
}

std::string IotconUtils::FromPresenceResponseResultType(iotcon_presence_result_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_PRESENCE_RESULT_E
  }
}

std::string IotconUtils::FromPresenceTriggerType(iotcon_presence_trigger_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_PRESENCE_TRIGGER_E
  }
}

std::string IotconUtils::FromResponseResultType(iotcon_response_result_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_RESPONSE_RESULT_E
  }
}

#undef X
#undef XD

#define X(v, s) if (e == s) return v;
#define XD(v, s) \
  LoggerE("Unknown value: %s, returning default: %d", e.c_str(), v); \
  return v;

iotcon_qos_e IotconUtils::ToQos(const std::string& e) {
  ScopeLogger();

  IOTCON_QOS_E
}

iotcon_connectivity_type_e IotconUtils::ToConnectivityType(const std::string& e) {
  ScopeLogger();

  IOTCON_CONNECTIVITY_TYPE_E
}

iotcon_observe_policy_e IotconUtils::ToObservePolicy(const std::string& e) {
  ScopeLogger();

  IOTCON_OBSERVE_POLICY_E
}

iotcon_response_result_e IotconUtils::ToResponseResult(const std::string& e) {
  IOTCON_RESPONSE_RESULT_E
}

#undef X
#undef XD

} // namespace iotcon
} // namespace extension
