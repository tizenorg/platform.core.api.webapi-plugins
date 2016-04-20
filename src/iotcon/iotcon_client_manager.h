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

#ifndef WEBAPI_PLUGINS_IOTCON_CLIENT_MANAGER_H__
#define WEBAPI_PLUGINS_IOTCON_CLIENT_MANAGER_H__

#include <memory>
#include <map>
#include <string>
#include <iotcon.h>

#include "iotcon/iotcon_utils.h"

#include "common/tizen_result.h"

namespace extension {
namespace iotcon {

class IotconClientManager {
 public:
  static IotconClientManager& GetInstance();

  common::TizenResult AddPresenceEventListener(const char* host,
                                               const iotcon_connectivity_type_e con_type_e,
                                               const char* resource_type,
                                               PresenceEventPtr presence);
  common::TizenResult RemovePresenceEventListener(long long id);
  picojson::value StoreRemoteResource(FoundRemoteInfoPtr ptr);
  picojson::value RemoveRemoteResource(FoundRemoteInfoPtr ptr);
  common::TizenResult GetRemoteById(long long id, FoundRemoteInfoPtr* res_pointer) const;

 private:
  IotconClientManager() = default;
  IotconClientManager(const IotconClientManager&) = delete;
  IotconClientManager(IotconClientManager&&) = delete;
  IotconClientManager& operator=(const IotconClientManager&) = delete;
  IotconClientManager& operator=(IotconClientManager&&) = delete;

  picojson::value PrepareManageIdAnswer(bool keep_id, long long id = 0);
  static void PresenceHandler(iotcon_presence_h  resource,
                             iotcon_error_e err,
                             iotcon_presence_response_h response,
                             void *user_data);

  PresenceMap presence_map_;
  FoundRemotesMap remotes_map_;
};

} // namespace iotcon
} // namespace extension

#endif // WEBAPI_PLUGINS_IOTCON_CLIENT_MANAGER_H__
