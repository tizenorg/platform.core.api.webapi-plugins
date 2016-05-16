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

#include "convergence/convergence_instance.h"
#include "convergence/convergence_manager.h"
#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/tools.h"
#include "common/optional.h"
#include "common/scope_exit.h"
#include "common/task-queue.h"


namespace extension {
namespace convergence {

namespace {
// The privileges that required in Convergence API
const std::string kPrivilegeInternet = "http://tizen.org/privilege/internet";
const std::string kPrivilegeBluetooth = "http://tizen.org/privilege/bluetooth";
const std::string kPrivilegeWifiDirect = "http://tizen.org/privilege/wifidirect";

} // namespace

using namespace common;
using namespace extension::convergence;

ConvergenceInstance::ConvergenceInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ConvergenceInstance::x, this, _1, _2));
  REGISTER_SYNC("Service_disconnect", ServiceDisconnect);
  REGISTER_SYNC("Service_connect", ServiceConnect);
  REGISTER_SYNC("Service_read", ServiceRead);
  REGISTER_SYNC("Service_start", ServiceStart);
  REGISTER_SYNC("Service_stop", ServiceStop);
  REGISTER_SYNC("Service_send", ServiceSend);
  REGISTER_SYNC("ConvergenceManager_stopDiscovery",
    ConvergenceManagerStopDiscovery);
  REGISTER_SYNC("ConvergenceManager_startDiscovery",
    ConvergenceManagerStartDiscovery);
  REGISTER_SYNC("Service_addListener", ServiceAddListener);
  REGISTER_SYNC("Service_removeListener", ServiceRemoveListener);
  #undef REGISTER_SYNC
}

ConvergenceInstance::~ConvergenceInstance() {
}


void ConvergenceInstance::ReplyAsync(ConvergenceCallbacks cbfunc,
                       int curListenerId, bool isSuccess, picojson::object& param) {
  LoggerD("Enter");

  param["status"] = picojson::value(isSuccess ? "success" : "error");
  param["curListenerId"] = picojson::value(static_cast<double>(curListenerId));

  switch(cbfunc) {
    case ConvergenceManagerDiscoveryCallback: {
      param["listenerId"] = picojson::value("CONVERGENCE_DISCOVERY_LISTENER");
      break;
    }
    case ServiceConnectCallback: {
      param["listenerId"] =
        picojson::value("CONVERGENCE_SERVICE_CONNECT_LISTENER");
      break;
    }
    case ServiceListenerCallback: {
      param["listenerId"] =
        picojson::value("CONVERGENCE_SERVICE_COMMAND_LISTENER");
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
      return;
    }
  }

  picojson::value result = picojson::value(param);
  LoggerD("---> %s", result.serialize().c_str());

  PostMessage(result.serialize().c_str());
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void ConvergenceInstance::ConvergenceManagerStartDiscovery(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "timeout", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto start_discovery =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter start_discovery");

    // Start the discovery procedure
    ConvergenceManager::GetInstance(this).StartDiscovery(
      static_cast<long>(args.get("timeout").get<double>()));

    picojson::object& object = result->get<picojson::object>();
    object["callbackId"] = args.get("callbackId");
    ReportSuccess(object);
  };

  auto start_discovery_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter start_discovery_result");
    result->get<picojson::object>()["callbackId"] = args.get("callbackId");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
    start_discovery,
    start_discovery_result,
    data);

  ReportSuccess(out);
}

void ConvergenceInstance::ConvergenceManagerStopDiscovery(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/


  // Running the discovery stop procedure
  ConvergenceManager::GetInstance(this).StopDiscovery();
  out["callbackId"] = args.get("callbackId");
  ReportSuccess(out);
}

void ConvergenceInstance::ServiceConnect(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto connect =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter connect");

    // Running the service connect procedure
    ConvergenceManager::GetInstance(this).Connect(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()),
      static_cast<int>(args.get("curListenerId").get<double>()));

    picojson::object& object = result->get<picojson::object>();
    object["callbackId"] = args.get("callbackId");
    ReportSuccess(object);
  };

  auto connect_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter connect_result");

    result->get<picojson::object>()["callbackId"] = args.get("callbackId");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
    connect,
    connect_result,
    data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceDisconnect(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto disconnect =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter disconnect");

    // Running the service disconnect procedure
    ConvergenceManager::GetInstance(this).Disconnect(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()));

    picojson::object& object = result->get<picojson::object>();
    ReportSuccess(object);
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Async<picojson::value>(
      disconnect,
      data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceStart(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto start = [this, args](const std::shared_ptr<picojson::value>& result) {

    // Running service start procedure
    ConvergenceManager::GetInstance(this).Start(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()),
      args.get("channel"),
      args.get("payload"));

    picojson::object& object = result->get<picojson::object>();
    object["callbackId"] = args.get("callbackId");
    ReportSuccess(object);
  };

  auto start_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = args.get("callbackId");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data = std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
    start,
    start_result,
    data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceRead(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto read = [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter read");

    // Running service read procedure
    ConvergenceManager::GetInstance(this).Read(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()),
      args.get("channel"),
      args.get("payload"));

    picojson::object& object = result->get<picojson::object>();
    object["callbackId"] = args.get("callbackId");
    ReportSuccess(object);
  };

  auto read_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter read_result");
    result->get<picojson::object>()["callbackId"] = args.get("callbackId");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
    read,
    read_result,
    data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceSend(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto send = [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter send");

    // Running service send procedure
    ConvergenceManager::GetInstance(this).Send(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()),
      args.get("channel"),
      args.get("payload"));

    picojson::object& object = result->get<picojson::object>();
    object["callbackId"] = args.get("callbackId");
    ReportSuccess(object);
  };

  auto send_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter send_result");
    result->get<picojson::object>()["callbackId"] = args.get("callbackId");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
    send,
    send_result,
    data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceStop(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto stop = [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter stop");

    // Running service stop procedure
    ConvergenceManager::GetInstance(this).Stop(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()),
      args.get("channel"),
      args.get("payload"));

    picojson::object& object = result->get<picojson::object>();
    object["callbackId"] = args.get("callbackId");
    ReportSuccess(object);
  };

  auto stop_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter stop_result");
    result->get<picojson::object>()["callbackId"] = args.get("callbackId");
    Instance::PostMessage(this, result->serialize().c_str());
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Queue<picojson::value>(
    stop,
    stop_result,
    data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceAddListener(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto setListener =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter setListener");

    // Adding a listener to the service
    ConvergenceManager::GetInstance(this).SetListener(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()),
      static_cast<int>(args.get("curListenerId").get<double>()));

    picojson::object& object = result->get<picojson::object>();
    ReportSuccess(object);
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Async<picojson::value>(
      setListener,
      data);

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceRemoveListener(
  const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto unsetListener =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    LoggerD("Enter unsetListener");

    // Removing the service listener
    ConvergenceManager::GetInstance(this).UnsetListener(
      args.get("deviceId").to_str().c_str(),
      static_cast<int>(args.get("serviceTypeNumber").get<double>()));

    picojson::object& object = result->get<picojson::object>();
    ReportSuccess(object);
  };

  auto data =
    std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}};

  TaskQueue::GetInstance().Async<picojson::value>(
    unsetListener,
    data);

  ReportSuccess(out);
}


#undef CHECK_EXIST

} // namespace convergence
} // namespace extension
