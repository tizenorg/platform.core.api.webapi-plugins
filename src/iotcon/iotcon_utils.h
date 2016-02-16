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

#ifndef WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__
#define WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__

#include <map>
#include <memory>
#include <set>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <iotcon.h>

#include "common/tizen_instance.h"
#include "common/tizen_result.h"

namespace extension {
namespace iotcon {

extern const std::string kIsDiscoverable;
extern const std::string kIsObservable;
extern const std::string kIsActive;
extern const std::string kIsSlow;
extern const std::string kIsSecure;
extern const std::string kIsExplicitDiscoverable;
extern const std::string kResourceTypes;
extern const std::string kResourceType;
extern const std::string kResourceInterfaces;
extern const std::string kResourceChildren;
extern const std::string kUriPath;
extern const std::string kStates;
extern const std::string kId;
extern const std::string kDeviceId;
extern const std::string kHostAddress;
extern const std::string kConnectivityType;
extern const std::string kResourceType;

class ResourceInfo;
class PresenceEvent;

typedef std::shared_ptr<ResourceInfo> ResourceInfoPtr;
typedef std::map<long long, ResourceInfoPtr> ResourceInfoMap;
typedef std::shared_ptr<PresenceEvent> PresenceEventPtr;
typedef std::map<long long, PresenceEventPtr> PresenceMap;

struct ResourceInfo {
  long long id;
  iotcon_resource_h handle;
  std::set<int> observers;
  common::PostCallback request_listener;
  std::unordered_map<long long, iotcon_response_h> unhandled_responses;
  std::set<ResourceInfoPtr> children;
  std::set<ResourceInfoPtr> parents;

  ResourceInfo() :
    id(0), handle(nullptr) {}
  ~ResourceInfo() {
    iotcon_resource_destroy(handle);
    for (auto& it : unhandled_responses) {
      iotcon_response_destroy(it.second);
    }
  }
};

struct RemoteResourceInfo {
  iotcon_remote_resource_h resource;
  char* uri_path;
  iotcon_connectivity_type_e connectivity_type;
  char* host_address;
  char* device_id;
  iotcon_resource_types_h types;
  int ifaces;
  int properties;  // to check if observable
  iotcon_options_h options;
  iotcon_representation_h representation;
  RemoteResourceInfo() :
    resource(nullptr), uri_path(nullptr),
    connectivity_type(IOTCON_CONNECTIVITY_ALL), host_address(nullptr),
    device_id(nullptr), types(nullptr), ifaces(0),
    properties(0), options(nullptr), representation(nullptr) {}
  ~RemoteResourceInfo() {
    //according to native description, must not release any handles
  }
};

struct PresenceEvent {
  long long id;
  iotcon_presence_h handle;
  common::PostCallback presence_listener;
};

class IotconUtils {
 public:
  static void PropertiesToJson(int properties, picojson::object* res);
  static common::TizenResult ArrayToInterfaces(const picojson::array& interfaces, int* res);
  static picojson::array InterfacesToArray(int interfaces);
  static common::TizenResult ArrayToTypes(const picojson::array& types, iotcon_resource_types_h* res);
  static common::TizenResult ExtractFromResource(const ResourceInfoPtr& pointer,
                                                 char** uri_path,
                                                 iotcon_resource_types_h* res_types,
                                                 int* ifaces,
                                                 int* properties);
  static common::TizenResult ResourceToJson(ResourceInfoPtr pointer,
                                            picojson::object* res);
  static common::TizenResult ExtractFromRemoteResource(RemoteResourceInfo* resource);
  static common::TizenResult RemoteResourceToJson(iotcon_remote_resource_h handle,
                                                  picojson::object* res);

  static common::TizenResult RequestToJson(iotcon_request_h request,
                                           picojson::object* out);
  static common::TizenResult RepresentationToJson(iotcon_representation_h representation,
                                                  picojson::object* out);
  static common::TizenResult StateToJson(iotcon_state_h state,
                                         picojson::object* out);
  static common::TizenResult StateListToJson(iotcon_list_h list,
                                             picojson::array* out);
  static common::TizenResult OptionsToJson(iotcon_options_h  options,
                                           picojson::array* out);
  static common::TizenResult QueryToJson(iotcon_query_h query,
                                         picojson::object* out);
  static common::TizenResult PresenceResponseToJson(iotcon_presence_response_h presence,
                                                picojson::object* out);
  static common::TizenResult ExtractFromPresenceEvent(const PresenceEventPtr& pointer,
                                                 char** host,
                                                 iotcon_connectivity_type_e* con_type,
                                                 char** resource_type);
  static common::TizenResult PlatformInfoToJson(iotcon_platform_info_h platform,
                                                picojson::object* out);
  static common::TizenResult PlatformInfoGetProperty(iotcon_platform_info_h platform,
                                                     iotcon_platform_info_e property,
                                                     const std::string& name,
                                                     picojson::object* out);
  static common::TizenResult DeviceInfoToJson(iotcon_device_info_h device,
                                                picojson::object* out);
  static common::TizenResult DeviceInfoGetProperty(iotcon_device_info_h platform,
                                                   iotcon_device_info_e  property,
                                                   const std::string& name,
                                                   picojson::object* out);

  static common::TizenResult RepresentationFromResource(const ResourceInfoPtr& resource,
                                                        const picojson::value& states,
                                                        iotcon_representation_h* representation);

  static common::TizenResult StateFromJson(const picojson::object& state,
                                           iotcon_state_h* out);
  static common::TizenResult StateListFromJson(const picojson::array& list,
                                               iotcon_list_h* out);

  static common::TizenResult ConvertIotconError(int error);
  static std::string FromConnectivityType(iotcon_connectivity_type_e e);
  static std::string FromRequestType(iotcon_request_type_e e);
  static std::string FromObserveType(iotcon_observe_type_e e);
  static std::string FromInterface(iotcon_interface_e e);
  static std::string FromPresenceResponseResultType(iotcon_presence_result_e e);
  static std::string FromPresenceTriggerType(iotcon_presence_trigger_e e);

  static iotcon_interface_e ToInterface(const std::string& e);
  static iotcon_connectivity_type_e ToConnectivityType(const std::string& e);
  static iotcon_qos_e ToQos(const std::string& e);
};

} // namespace iotcon
} // namespace extension

#endif // WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__
