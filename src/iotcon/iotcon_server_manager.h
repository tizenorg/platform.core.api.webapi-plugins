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

#include "iotcon/iotcon_utils.h"

#include "common/tizen_result.h"

namespace extension {
namespace iotcon {

class IotconInstance;

class IotconServerManager {
 public:
  static IotconServerManager& GetInstance();

  common::TizenResult CreateResource(const std::string& uri_path,
                                     const picojson::array& interfaces_array,
                                     const picojson::array& types_array,
                                     int properties,
                                     ResourceInfoPtr res_pointer);
  common::TizenResult GetResourceById(long long id, ResourceInfoPtr* res_pointer) const;
  common::TizenResult DestroyResource(long long id);
  common::TizenResult GetResourceByHandle(iotcon_resource_h resource, ResourceInfoPtr* res_pointer) const;
  common::TizenResult GetResponseById(long long id, ResponsePtr* out) const;

 private:
  IotconServerManager() = default;
  IotconServerManager(const IotconServerManager&) = delete;
  IotconServerManager(IotconServerManager&&) = delete;
  IotconServerManager& operator=(const IotconServerManager&) = delete;
  IotconServerManager& operator=(IotconServerManager&&) = delete;

  static void RequestHandler(iotcon_resource_h resource,
                             iotcon_request_h request, void *user_data);

  ResourceInfoMap resource_map_;
};

} // namespace iotcon
} // namespace extension

#endif // WEBAPI_PLUGINS_IOTCON_SERVER_MANAGER_H__
