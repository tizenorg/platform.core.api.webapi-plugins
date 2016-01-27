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

#ifndef WEBAPI_PLUGINS_IOTCON_SERVER_MANAGER_H__
#define WEBAPI_PLUGINS_IOTCON_SERVER_MANAGER_H__

#include <memory>
#include <map>
#include <string>
#include <iotcon.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace iotcon {

struct ResourceInfo {
  const char *uri_path;
  iotcon_resource_types_h res_types;
  int ifaces;
  int properties;
  iotcon_resource_h handle;
  ResourceInfo() :
    uri_path(nullptr), res_types(nullptr), ifaces(0),
    properties(0), handle(nullptr) {}
  ~ResourceInfo() {
    delete uri_path;
    iotcon_resource_types_destroy(res_types);
    iotcon_resource_destroy (handle);
  }
};

typedef std::shared_ptr<ResourceInfo> ResourceInfoPtr;
typedef std::map<long long, ResourceInfoPtr> ResourceInfoMap;

class IotconInstance;

class IotconServerManager {
 public:
  IotconServerManager(IotconInstance* instance);
  ~IotconServerManager();

  common::PlatformResult RestoreHandles();

  common::PlatformResult CreateResource(/*std::string uri_path, bool is_discoverable,
                                        bool is_observable, picojson::array array,
                                        iotcon_resource_h* res_handle*/);

 private:
  IotconInstance* instance_;
  ResourceInfoMap resource_map_;
};
} // namespace iotcon
} // namespace extension

#endif // WEBAPI_PLUGINS_IOTCON_SERVER_MANAGER_H__
