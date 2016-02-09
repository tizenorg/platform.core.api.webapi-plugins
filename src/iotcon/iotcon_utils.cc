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

namespace extension {
namespace iotcon {

namespace {

#define IOTCON_CONNECTIVITY_TYPE_E \
  X(IOTCON_CONNECTIVITY_IPV4, "IPV4") \
  X(IOTCON_CONNECTIVITY_IPV6, "IPV6") \
  X(IOTCON_CONNECTIVITY_BT_EDR, "BT_EDR") \
  X(IOTCON_CONNECTIVITY_BT_LE, "BT_LE") \
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

#define IOTCON_INTERFACE_E \
  X(IOTCON_INTERFACE_NONE, "NONE") \
  X(IOTCON_INTERFACE_DEFAULT, "DEFAULT") \
  X(IOTCON_INTERFACE_LINK, "LINK") \
  X(IOTCON_INTERFACE_BATCH, "BATCH") \
  X(IOTCON_INTERFACE_GROUP, "GROUP") \
  X(IOTCON_INTERFACE_READONLY, "READONLY") \
  XD(IOTCON_INTERFACE_NONE, "unknown")

#define IOTCON_QOS_E \
  X(IOTCON_QOS_LOW, "LOW") \
  X(IOTCON_QOS_HIGH, "HIGH") \
  XD(IOTCON_QOS_LOW, "unknown")

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

const std::string kHostAddress = "hostAddress";
const std::string kConnectivityType = "connectivityType";
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

using common::TizenResult;
using common::TizenSuccess;

TizenResult IotconUtils::ArrayToInterfaces(const picojson::array& interfaces, int* res) {
  ScopeLogger();

  int result_value = IOTCON_INTERFACE_NONE;

  for (auto iter = interfaces.begin(); iter != interfaces.end(); ++iter) {
    if (!iter->is<std::string>()) {
      return LogAndCreateTizenError(InvalidValuesError, "Array holds incorrect interface names");
    } else {
      iotcon_interface_e interface = ToInterface(iter->get<std::string>());
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
    res.push_back(picojson::value(FromInterface(IOTCON_INTERFACE_DEFAULT)));
  }
  if (interfaces & IOTCON_INTERFACE_LINK) {
    res.push_back(picojson::value(FromInterface(IOTCON_INTERFACE_LINK)));
  }
  if (interfaces & IOTCON_INTERFACE_BATCH) {
    res.push_back(picojson::value(FromInterface(IOTCON_INTERFACE_BATCH)));
  }
  if (interfaces & IOTCON_INTERFACE_GROUP) {
    res.push_back(picojson::value(FromInterface(IOTCON_INTERFACE_GROUP)));
  }
  if (interfaces & IOTCON_INTERFACE_READONLY) {
    res.push_back(picojson::value(FromInterface(IOTCON_INTERFACE_READONLY)));
  }
  return res;
}

TizenResult IotconUtils::ArrayToTypes(const picojson::array& types, iotcon_resource_types_h* res) {
  ScopeLogger();

  iotcon_resource_types_h resource_types = nullptr;
  auto result = ConvertIotconError(iotcon_resource_types_create(&resource_types));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_resource_types_create() failed"));
  }

  for (auto iter = types.begin(); iter != types.end(); ++iter) {
    if (!iter->is<std::string>()) {
      return LogAndCreateTizenError(InvalidValuesError, "Array holds incorrect types");
    } else {
      result = ConvertIotconError(iotcon_resource_types_add(resource_types, iter->get<std::string>().c_str()));
      if (!result) {
        iotcon_resource_types_destroy(resource_types);
        LogAndReturnTizenError(result, ("iotcon_resource_types_add() failed"));
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

  result = ConvertIotconError(iotcon_resource_get_properties(pointer->handle, properties));
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
  int ifaces = 0;
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
    ret = IotconServerManager::GetInstance().GetResourceById((*iter), &resource);
    if (ret.IsSuccess()) {
      LoggerD("Found children RESOURCE\nid: %lld\nhandle: %p", resource->id, resource->handle);

      picojson::value child = picojson::value(picojson::object());
      ret = IotconUtils::ResourceToJson(resource, &(child.get<picojson::object>()));
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

common::TizenResult IotconUtils::RequestToJson(iotcon_request_h request,
                                               picojson::object* out) {
  ScopeLogger();

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
    picojson::value v{picojson::object{}};
    result = RepresentationToJson(representation, &v.get<picojson::object>());
    if (!result) {
      LogAndReturnTizenError(result, ("RepresentationToJson() failed"));
    }
    out->insert(std::make_pair(kRepresentation, v));
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
    out->insert(std::make_pair(kRequestType, picojson::value{FromObserveType(observe_type)}));
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::RepresentationToJson(iotcon_representation_h representation,
                                                      picojson::object* out) {
  ScopeLogger();

  {
    // hostAddress
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
    int interfaces = 0;
    auto result = ConvertIotconError(iotcon_representation_get_resource_interfaces(representation, &interfaces));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_representation_get_resource_interfaces() failed"));
    }
    out->insert(std::make_pair(kResourceInterfaces, picojson::value{InterfacesToArray(interfaces)}));
  }

  {
    // states
    iotcon_state_h state = nullptr;
    auto result = ConvertIotconError(iotcon_representation_get_state(representation, &state));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_representation_get_state() failed"));
    }
    picojson::value v{picojson::object{}};
    result = StateToJson(state, &v.get<picojson::object>());
    if (!result) {
      LogAndReturnTizenError(result, ("StateToJson() failed"));
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

  return TizenSuccess();
}

common::TizenResult IotconUtils::StateToJson(iotcon_state_h state,
                                             picojson::object* out) {
  ScopeLogger();

  auto result = ConvertIotconError(iotcon_state_foreach(state, [](iotcon_state_h state, const char* key, void* user_data) -> bool {
    iotcon_type_e type = IOTCON_TYPE_NONE;
    auto result = ConvertIotconError(iotcon_state_get_type(state, key, &type));

    if (result) {
      auto out = static_cast<picojson::object*>(user_data);

      switch (type) {
        case IOTCON_TYPE_NONE:
          LoggerE("Key %s has type NONE", key);
          break;

        case IOTCON_TYPE_INT:
          {
            int value = 0;
            result = ConvertIotconError(iotcon_state_get_int(state, key, &value));
            if (result) {
              out->insert(std::make_pair(key, picojson::value{static_cast<double>(value)}));
            } else {
              LoggerE("iotcon_state_get_int() failed");
            }
          }
          break;

        case IOTCON_TYPE_BOOL:
          {
            bool value = false;
            result = ConvertIotconError(iotcon_state_get_bool(state, key, &value));
            if (result) {
              out->insert(std::make_pair(key, picojson::value{value}));
            } else {
              LoggerE("iotcon_state_get_bool() failed");
            }
          }
          break;

        case IOTCON_TYPE_DOUBLE:
          {
            double value = 0.0;
            result = ConvertIotconError(iotcon_state_get_double(state, key, &value));
            if (result) {
              out->insert(std::make_pair(key, picojson::value{value}));
            } else {
              LoggerE("iotcon_state_get_double() failed");
            }
          }
          break;

        case IOTCON_TYPE_STR:
          {
            char* value = nullptr;
            result = ConvertIotconError(iotcon_state_get_str(state, key, &value));
            if (result && value) {
              out->insert(std::make_pair(key, picojson::value{value}));
            } else {
              LoggerE("iotcon_state_get_str() failed");
            }
          }
          break;

        case IOTCON_TYPE_BYTE_STR:
          {
            unsigned char* value = nullptr;
            int length = 0;
            result = ConvertIotconError(iotcon_state_get_byte_str(state, key, &value, &length));

            if (result && length) {
              std::unique_ptr<char[]> data{new char[2 * length]};
              common::tools::BinToHex(value, length, data.get(), 2 * length);
              out->insert(std::make_pair(key, picojson::value{kHexPrefix + data.get()}));
            } else {
              LoggerE("iotcon_state_get_byte_str() failed");
            }
          }
          break;

        case IOTCON_TYPE_NULL:
          out->insert(std::make_pair(key, picojson::value{}));
          break;

        case IOTCON_TYPE_LIST:
          {
            iotcon_list_h list = nullptr;
            result = ConvertIotconError(iotcon_state_get_list(state, key, &list));
            if (result) {
              picojson::value value{picojson::array{}};

              result = StateListToJson(list, &value.get<picojson::array>());
              if (result) {
                out->insert(std::make_pair(key, picojson::value{value}));
              } else {
                LoggerE("StateListToJson() failed");
              }
            } else {
              LoggerE("iotcon_state_get_list() failed");
            }
          }
          break;

        case IOTCON_TYPE_STATE:
          {
            iotcon_state_h child = nullptr;
            result = ConvertIotconError(iotcon_state_get_state(state, key, &child));
            if (result) {
              picojson::value value{picojson::object{}};

              result = StateToJson(child, &value.get<picojson::object>());
              if (result) {
                out->insert(std::make_pair(key, picojson::value{value}));
              } else {
                LoggerE("StateToJson() failed");
              }
            } else {
              LoggerE("iotcon_state_get_state() failed");
            }
          }
          break;
      }
    } else {
      LoggerE("iotcon_state_get_type() failed");
    }

    // always continue with iteration
    return true;
  }, out));

  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_state_foreach() failed"));
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::StateListToJson(iotcon_list_h list,
                                                 picojson::array* out) {
  ScopeLogger();

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

    case IOTCON_TYPE_STATE:
      result = ConvertIotconError(iotcon_list_foreach_state(list, [](int, iotcon_state_h state, void* user_data) -> bool {
        picojson::value value{picojson::object{}};
        auto result = StateToJson(state, &value.get<picojson::object>());
        if (result) {
          auto out = static_cast<picojson::array*>(user_data);
          out->push_back(picojson::value{value});
        } else {
          LoggerE("StateToJson() failed");
        }
        // always continue with iteration
        return true;
      }, out));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_list_foreach_state() failed"));
      }
      break;
  }

  return TizenSuccess();
}

common::TizenResult IotconUtils::OptionsToJson(iotcon_options_h  options,
                                               picojson::array* out) {
  ScopeLogger();

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

  return TizenSuccess();
}

common::TizenResult IotconUtils::QueryToJson(iotcon_query_h query,
                                             picojson::object* out) {
  ScopeLogger();

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
    iotcon_interface_e interface = IOTCON_INTERFACE_NONE;
    auto result = ConvertIotconError(iotcon_query_get_interface(query, &interface));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_query_get_interface() failed"));
    }
    out->insert(std::make_pair(kResourceInterface, picojson::value{FromInterface(interface)}));
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
    int intrefaces = IOTCON_INTERFACE_NONE;
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
      iotcon_state_h state_handle = nullptr;
      result = IotconUtils::StateFromJson(state.get<picojson::object>(), &state_handle);
      if (!result) {
        LogAndReturnTizenError(result, ("StateFromJson() failed"));
      }
      SCOPE_EXIT {
        iotcon_state_destroy(state_handle);
      };
      result = IotconUtils::ConvertIotconError(iotcon_representation_set_state(representation, state_handle));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_representation_set_state() failed"));
      }
    }
  }

  {
    int children = 0;
    result = IotconUtils::ConvertIotconError(iotcon_resource_get_number_of_children(resource->handle, &children));
    if (!result) {
      LogAndReturnTizenError(result, ("iotcon_resource_get_number_of_children() failed"));
    }

    for (int i = 0; i < children; ++i) {
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
                                               iotcon_state_h* out) {
  ScopeLogger();

  iotcon_state_h state = nullptr;

  auto result = IotconUtils::ConvertIotconError(iotcon_state_create(&state));
  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_state_create() failed"));
  }

  std::unique_ptr<std::remove_pointer<iotcon_state_h>::type, void(*)(iotcon_state_h)> ptr{state, &iotcon_state_destroy};

  for (const auto& property : s) {
    const auto& key = property.first;

    if (property.second.is<picojson::null>()) {
      result = IotconUtils::ConvertIotconError(iotcon_state_add_null(state, key.c_str()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_state_add_null() failed"));
      }
    } else if (property.second.is<bool>()) {
      result = IotconUtils::ConvertIotconError(iotcon_state_add_bool(state, key.c_str(), property.second.get<bool>()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_state_add_bool() failed"));
      }
    } else if (property.second.is<double>()) {
      result = IotconUtils::ConvertIotconError(iotcon_state_add_double(state, key.c_str(), property.second.get<double>()));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_state_add_double() failed"));
      }
    } else if (property.second.is<std::string>()) {
      const auto& value = property.second.get<std::string>();

      if (0 == value.find(kHexPrefix)) {
        auto data = value.c_str() + kHexPrefix.length(); // skip prefix
        auto size = value.length() - kHexPrefix.length();
        auto length = size / 2;
        std::unique_ptr<unsigned char[]> hex{new unsigned char[length]};
        common::tools::HexToBin(data, size, hex.get(), length);
        result = IotconUtils::ConvertIotconError(iotcon_state_add_byte_str(state, key.c_str(), hex.get(), length));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_state_add_byte_str() failed"));
        }
      } else {
        result = IotconUtils::ConvertIotconError(iotcon_state_add_str(state, key.c_str(), const_cast<char*>(value.c_str())));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_state_add_str() failed"));
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
        result = IotconUtils::ConvertIotconError(iotcon_state_add_list(state, key.c_str(), list));
        if (!result) {
          LogAndReturnTizenError(result, ("iotcon_state_add_list() failed"));
        }
      }
    } else if (property.second.is<picojson::object>()) {
      iotcon_state_h sub_state = nullptr;
      result = StateFromJson(property.second.get<picojson::object>(), &sub_state);
      if (!result) {
        LogAndReturnTizenError(result, ("StateFromJson() failed"));
      }
      SCOPE_EXIT {
        iotcon_state_destroy(sub_state);
      };
      result = IotconUtils::ConvertIotconError(iotcon_state_add_state(state, key.c_str(), sub_state));
      if (!result) {
        LogAndReturnTizenError(result, ("iotcon_state_add_state() failed"));
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
      type = IOTCON_TYPE_STATE;
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

      case IOTCON_TYPE_STATE:
        if (v.is<picojson::object>()) {
          iotcon_state_h state = nullptr;
          result = StateFromJson(v.get<picojson::object>(), &state);
          if (!result) {
            LogAndReturnTizenError(result, ("StateFromJson() failed"));
          }
          SCOPE_EXIT {
            iotcon_state_destroy(state);
          };
          result = IotconUtils::ConvertIotconError(iotcon_list_add_state(list, state, position++));
          if (!result) {
            LogAndReturnTizenError(result, ("iotcon_list_add_state() failed"));
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
    case IOTCON_ERROR_DBUS:
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

std::string IotconUtils::FromInterface(iotcon_interface_e e) {
  ScopeLogger();

  switch (e) {
    IOTCON_INTERFACE_E
  }
}

#undef X
#undef XD

#define X(v, s) if (e == s) return v;
#define XD(v, s) \
  LoggerE("Unknown value: %s, returning default: %d", e.c_str(), v); \
  return v;

iotcon_interface_e IotconUtils::ToInterface(const std::string& e) {
  ScopeLogger();

  IOTCON_INTERFACE_E
}

iotcon_qos_e IotconUtils::ToQos(const std::string& e) {
  ScopeLogger();

  IOTCON_QOS_E
}

#undef X
#undef XD

} // namespace iotcon
} // namespace extension
