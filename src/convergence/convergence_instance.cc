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


#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <unistd.h>

#include "convergence/convergence_instance.h"
#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/tools.h"
#include "common/optional.h"
#include "common/scope_exit.h"
#include "common/task-queue.h"


namespace extension {
namespace convergence {

using namespace common;
using namespace extension::convergence;

ConvergenceInstance::ConvergenceInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ConvergenceInstance::x, this, _1, _2));
  REGISTER_SYNC("ConvergenceManager_startDiscovery", ConvergenceManagerStartDiscovery);
  #undef REGISTER_SYNC
}

ConvergenceInstance::~ConvergenceInstance() {
}


#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

static void *discovery_thread_func(void *arg) {

sleep(1);
LoggerI("3...");
sleep(1);
LoggerI("2...");
sleep(1);
LoggerI("1...");
sleep(1);
LoggerI("BOOM!");
sleep(1);

  picojson::value result = picojson::value(picojson::object());
  result.get<picojson::object>()["listenerId"] = picojson::value("CONVERGENCELISTENER");
  result.get<picojson::object>()["device"] = picojson::value("device_data");

  ConvergenceInstance *owner = (ConvergenceInstance *)arg;
  Instance::PostMessage(owner, result.serialize().c_str());

sleep(1);
LoggerI("4...");
sleep(1);
LoggerI("5...");
sleep(1);
LoggerI("6...");
sleep(100);


  return NULL;
}

void ConvergenceInstance::ConvergenceManagerStartDiscovery(const picojson::value& args, picojson::object& out) {
  LoggerI("ConvergenceManagerStartDiscovery");

  const std::string arg_str = args.serialize();
  LoggerI("ARGS: %s", arg_str.c_str());

  /*
   * Routine 1 | FAILURE | PostMessage from an arbitrary thread
   */
//#if 0
  pthread_t discovery_thread;
  pthread_create(&discovery_thread, NULL, discovery_thread_func, this);
//#endif

  /*
   * Routine 2 | OK | PostMessage from the main thread
   */
#if 0
  auto start_discovery = [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerI("start discovery lambda");

    picojson::object& object = result->get<picojson::object>();
    ReportSuccess(object);

  };

  auto start_discovery_result = [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerI("start discovery result lambda");

    result->get<picojson::object>()["listenerId"] = picojson::value("CONVERGENCELISTENER");
    result->get<picojson::object>()["device"] = picojson::value("device_info");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data = std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

LoggerI("adding jobs to the queue");
  TaskQueue::GetInstance().Queue<picojson::value>(
      start_discovery,
      start_discovery_result,
      data);
#endif

LoggerI("return from start discovery function");
  ReportSuccess(out);

}

#undef CHECK_EXIST

} // namespace convergence
} // namespace extension
