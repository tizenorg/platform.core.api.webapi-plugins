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

#ifndef NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_INSTANCE_H_
#define NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_INSTANCE_H_

#include "common/extension.h"
#include "networkbearerselection_manager.h"
#include <map>

namespace extension {
namespace networkbearerselection {

class NetworkBearerSelectionInstance : public common::ParsedInstance,
                                       public NetworkBearerSelectionListener {
 public:
  NetworkBearerSelectionInstance();
  virtual ~NetworkBearerSelectionInstance();

 private:
  void NetworkBearerSelectionRequestRouteToHost(const picojson::value& args,
                                                picojson::object& out);
  void NetworkBearerSelectionReleaseRouteToHost(const picojson::value& args,
                                                picojson::object& out);

  virtual void onNBSSuccess(const std::string& domain_name);
  virtual void onNBSError(const std::string& domain_name,
                          const std::string& info);
  virtual void onNBSDisconnect(const std::string& domain_name);

  std::multimap<std::string, int> listenerMap;
};

}  // namespace networkbearerselection
}  // namespace extension

#endif  // NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_INSTANCE_H_
