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

#include "iotcon/iotcon_client_manager.h"

#include "common/logger.h"

namespace extension {
namespace iotcon {

using common::TizenResult;
using common::TizenSuccess;

namespace {

long long GetPresenceNextId() {
  static long long id = 0;
  return ++id;
}

}  // namespace

IotconClientManager& IotconClientManager::GetInstance() {
  static IotconClientManager instance;
  return instance;
}

TizenResult IotconClientManager::RestoreHandles() {
  ScopeLogger();

  for (const auto& it : presence_map_) {
    LoggerD("Restoring handle for presence event with id: %lld", it.first);

    PresenceEventPtr presence = it.second;
    char* host = nullptr;
    char* resource_type = nullptr;
    iotcon_connectivity_type_e con_type = IOTCON_CONNECTIVITY_IPV4;

    auto res = IotconUtils::ExtractFromPresenceEvent(presence, &host,
                                                   &con_type, &resource_type);
    if (!res){
      return res;
    }

    const iotcon_presence_h old_handle = presence->handle;
    int ret = iotcon_add_presence_cb(host, con_type, resource_type,
                                     PresenceHandler, this,&(presence->handle));
    if (IOTCON_ERROR_NONE != ret || nullptr == presence->handle) {
      LogAndReturnTizenError(IotconUtils::ConvertIotconError(ret),
                             ("iotcon_add_presence_cb() failed: %d (%s)",
                                 ret, get_error_message(ret)));
    }
    if (old_handle) {
      LoggerD("destroy handle which is currently invalid: %p", old_handle);
      iotcon_remove_presence_cb(old_handle);
    }
    LoggerD("new handle: %p", (presence->handle));
  }

  return TizenSuccess();
}

void IotconClientManager::PresenceHandler(iotcon_presence_h presence,
                                         iotcon_error_e err,
                                         iotcon_presence_response_h response,
                                         void *user_data) {
  ScopeLogger();

  if(IOTCON_ERROR_NONE != err) {
    LoggerE("Error in presence event callback!");
    return;
  }
  auto that = static_cast<IotconClientManager*>(user_data);

  for (const auto& p : that->presence_map_) {
    if (p.second->presence_listener && p.second->handle == presence) {
      picojson::value value{picojson::object{}};
      auto& obj = value.get<picojson::object>();
      auto ret = IotconUtils::PresenceResponseToJson(response,
                                                     &obj);
      if (!ret) {
        LoggerE("PresenceResponseToJson() failed");
        return;
      }
      // call listener
      p.second->presence_listener(TizenSuccess(), value);
    }
  }
};

common::TizenResult IotconClientManager::AddPresenceEventListener(
    const char* host, const iotcon_connectivity_type_e con_type_e,
    const char* resource_type, PresenceEventPtr presence) {
  ScopeLogger();

  auto result = IotconUtils::ConvertIotconError(iotcon_add_presence_cb(
      host, con_type_e, resource_type, PresenceHandler, this,
      &(presence->handle)));

  // storing PresenceEvent into map
  presence->id = GetPresenceNextId();
  presence_map_.insert(std::make_pair(presence->id, presence));

  return TizenSuccess();
}

common::TizenResult IotconClientManager::RemovePresenceEventListener(long long id) {
  auto it = presence_map_.find(id);
  if (it == presence_map_.end()) {
    return LogAndCreateTizenError(AbortError, "Presence callback with specified ID does not exist");
  }

  auto result = IotconUtils::ConvertIotconError(iotcon_remove_presence_cb(it->second->handle));

  if (!result) {
    LogAndReturnTizenError(result, ("iotcon_remove_presence_cb failed"));
  }

  presence_map_.erase(id);

  return TizenSuccess();
}

}  // namespace iotcon
}  // namespace extension
