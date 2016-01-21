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

#include "common/picojson.h"
#include "common/extension.h"
#include "iotcon/iotcon_server_manager.h"

namespace extension {
namespace iotcon {

class IotconInstance : public common::ParsedInstance {
 public:
  IotconInstance();
  virtual ~IotconInstance();
 private:
  static void ConnectionChangedCallback(bool is_connected, void* user_data);

  void IotconResourceGetObserverIds(const picojson::value& args,
                                    picojson::object& out);
  void IotconResourceNotify(const picojson::value& args,
                            picojson::object& out);
  void IotconResourceAddResourceTypes(const picojson::value& args,
                                      picojson::object& out);
  void IotconResourceAddResourceInterfaces(const picojson::value& args,
                                           picojson::object& out);
  void IotconResourceAddChildResource(const picojson::value& args,
                                      picojson::object& out);
  void IotconResourceRemoveChildResource(const picojson::value& args,
                                         picojson::object& out);
  void IotconResourceSetRequestListener(const picojson::value& args,
                                        picojson::object& out);
  void IotconRemoteResourceUnsetRequestListener(const picojson::value& args,
                                                picojson::object& out);
  void IotconResponseSend(const picojson::value& args,
                          picojson::object& out);
  void IotconRemoteResourceGetCachedRepresentation(const picojson::value& args,
                                                   picojson::object& out);
  void IotconRemoteResourceMethodGet(const picojson::value& args,
                                     picojson::object& out);
  void IotconRemoteResourceMethodPut(const picojson::value& args,
                                     picojson::object& out);
  void IotconRemoteResourceMethodPost(const picojson::value& args,
                                      picojson::object& out);
  void IotconRemoteResourceMethodDelete(const picojson::value& args,
                                        picojson::object& out);
  void IotconRemoteResourceSetStateChangeListener(const picojson::value& args,
                                                  picojson::object& out);
  void IotconRemoteResourceUnsetStateChangeListener(const picojson::value& args,
                                                    picojson::object& out);
  void IotconRemoteResourceStartCaching(const picojson::value& args,
                                        picojson::object& out);
  void IotconRemoteResourceStopCaching(const picojson::value& args,
                                       picojson::object& out);
  void IotconRemoteResourceSetConnectionChangeListener(const picojson::value& args,
                                                       picojson::object& out);
  void IotconRemoteResourceUnsetConnectionChangeListener(const picojson::value& args,
                                                         picojson::object& out);
  void IotconClientFindResource(const picojson::value& args,
                                picojson::object& out);
  void IotconClientAddPresenceEventListener(const picojson::value& args,
                                            picojson::object& out);
  void IotconClientRemovePresenceEventListener(const picojson::value& args,
                                               picojson::object& out);
  void IotconClientGetDeviceInfo(const picojson::value& args,
                                 picojson::object& out);
  void IotconClientGetPlatformInfo(const picojson::value& args,
                                   picojson::object& out);
  void IotconServerCreateResource(const picojson::value& args,
                                  picojson::object& out);
  void IotconServerRemoveResource(const picojson::value& args,
                                  picojson::object& out);
  void IotconServerUpdateResource(const picojson::value& args,
                                  picojson::object& out);
  void IotconGetTimeout(const picojson::value& args,
                        picojson::object& out);
  void IotconSetTimeout(const picojson::value& args,
                        picojson::object& out);

  IotconServerManager manager_;
};

}  // namespace iotcon
}  // namespace extension

#endif  // IOTCON_IOTCON_INSTANCE_H_
