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

#ifndef IOTCON_IOTCON_INSTANCE_H_
#define IOTCON_IOTCON_INSTANCE_H_

#include "common/tizen_instance.h"

#include "iotcon/iotcon_server_manager.h"

namespace extension {
namespace iotcon {

class IotconInstance : public common::TizenInstance {
 public:
  IotconInstance();
  virtual ~IotconInstance();
 private:
  static void ConnectionChangedCallback(bool is_connected, void* user_data);

  common::TizenResult ResourceGetObserverIds(const picojson::object& args);
  common::TizenResult ResourceNotify(const picojson::object& args);
  common::TizenResult ResourceAddResourceTypes(const picojson::object& args,
                                               const common::AsyncToken& token);
  common::TizenResult ResourceAddResourceInterfaces(const picojson::object& args,
                                                    const common::AsyncToken& token);
  common::TizenResult ResourceAddChildResource(const picojson::object& args);
  common::TizenResult ResourceRemoveChildResource(const picojson::object& args);
  common::TizenResult ResourceSetRequestListener(const picojson::object& args);
  common::TizenResult ResourceUnsetRequestListener(const picojson::object& args);
  common::TizenResult ResponseSend(const picojson::object& args);
  common::TizenResult RemoteResourceGetCachedRepresentation(const picojson::object& args);
  common::TizenResult RemoteResourceMethodGet(const picojson::object& args,
                                              const common::AsyncToken& token);
  common::TizenResult RemoteResourceMethodPut(const picojson::object& args,
                                              const common::AsyncToken& token);
  common::TizenResult RemoteResourceMethodPost(const picojson::object& args,
                                               const common::AsyncToken& token);
  common::TizenResult RemoteResourceMethodDelete(const picojson::object& args,
                                                 const common::AsyncToken& token);
  common::TizenResult RemoteResourceSetStateChangeListener(const picojson::object& args);
  common::TizenResult RemoteResourceUnsetStateChangeListener(const picojson::object& args);
  common::TizenResult RemoteResourceStartCaching(const picojson::object& args);
  common::TizenResult RemoteResourceStopCaching(const picojson::object& args);
  common::TizenResult RemoteResourceSetConnectionChangeListener(const picojson::object& args);
  common::TizenResult RemoteResourceUnsetConnectionChangeListener(const picojson::object& args);
  common::TizenResult ClientFindResource(const picojson::object& args,
                                         const common::AsyncToken& token);
  common::TizenResult ClientAddPresenceEventListener(const picojson::object& args);
  common::TizenResult ClientRemovePresenceEventListener(const picojson::object& args);
  common::TizenResult ClientGetDeviceInfo(const picojson::object& args,
                                          const common::AsyncToken& token);
  common::TizenResult ClientGetPlatformInfo(const picojson::object& args,
                                            const common::AsyncToken& token);
  common::TizenResult ServerCreateResource(const picojson::object& args);
  common::TizenResult ServerRemoveResource(const picojson::object& args);
  common::TizenResult GetTimeout(const picojson::object& args);
  common::TizenResult SetTimeout(const picojson::object& args);
};

}  // namespace iotcon
}  // namespace extension

#endif  // IOTCON_IOTCON_INSTANCE_H_
