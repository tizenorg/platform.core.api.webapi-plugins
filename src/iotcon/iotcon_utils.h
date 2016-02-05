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

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <stdlib.h>

#include <iotcon.h>

#include "common/tizen_result.h"

namespace extension {
namespace iotcon {

extern const std::string kIsDiscoverable;
extern const std::string kIsObservable;
extern const std::string kResourceTypes;
extern const std::string kResourceInterfaces;
extern const std::string kResourceChildren;
extern const std::string kUriPath;
extern const std::string kResourceId;

struct ResourceInfo {
  long long id;
  std::vector<long long> children_ids;
  iotcon_resource_h handle;
  ResourceInfo() :
    id(0), handle(nullptr) {}
  ~ResourceInfo() {
    iotcon_resource_destroy (handle);
  }
};

typedef std::shared_ptr<ResourceInfo> ResourceInfoPtr;
typedef std::map<long long, ResourceInfoPtr> ResourceInfoMap;

class IotconServerManager;

class IotconUtils {
 public:
  static common::TizenResult StringToInterface(const std::string& interface, iotcon_interface_e* res);
  static common::TizenResult ArrayToInterfaces(const picojson::array& interfaces, int* res);
  static picojson::array InterfacesToArray(int interfaces);
  static common::TizenResult ArrayToTypes(const picojson::array& types, iotcon_resource_types_h* res);
  static common::TizenResult ExtractFromResource(const ResourceInfoPtr& pointer,
                                                 char** uri_path,
                                                 iotcon_resource_types_h* res_types,
                                                 int* ifaces,
                                                 int* properties);
  static common::TizenResult ResourceToJson(ResourceInfoPtr pointer,
                                            const IotconServerManager& manager,
                                            picojson::object* res);
};

} // namespace iotcon
} // namespace extension

#endif // WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__
