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

#ifndef CONVERGENCE_CONVERGENCE_INSTANCE_H_
#define CONVERGENCE_CONVERGENCE_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace convergence {

enum ConvergenceCallbacks {
  ServiceConnectCallback,
  ConvergenceManagerDiscoveryCallback,
  ServiceListenerCallback,
};


class ConvergenceInstance : public common::ParsedInstance {
 public:
  ConvergenceInstance();
  virtual ~ConvergenceInstance();
 public:
  void ReplyAsync(ConvergenceCallbacks cbfunc,
    int curListenerId, bool isSuccess, picojson::object& param);

 private:
  void ServiceRemoveListener(const picojson::value& args, picojson::object& out);
  void ServiceDisconnect(const picojson::value& args, picojson::object& out);
  void ServiceConnect(const picojson::value& args, picojson::object& out);
  void ServiceRead(const picojson::value& args, picojson::object& out);
  void ServiceStart(const picojson::value& args, picojson::object& out);
  void ServiceStop(const picojson::value& args, picojson::object& out);
  void ServiceSend(const picojson::value& args, picojson::object& out);
  void ConvergenceManagerStopDiscovery(const picojson::value& args,
   picojson::object& out);
  void ConvergenceManagerStartDiscovery(const picojson::value& args,
   picojson::object& out);
  void ServiceAddListener(const picojson::value& args, picojson::object& out);
};

} // namespace convergence
} // namespace extension

#endif // CONVERGENCE_CONVERGENCE_INSTANCE_H_
