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

#include "convergence/convergence_instance.h"

#include <functional>
#include <string>

#include "convergence/convergence_manager.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/task-queue.h"
#include "common/tools.h"


namespace extension {
namespace convergence {

namespace {
// The privileges that required in Convergence API
const std::string kPrivilegeInternet = "http://tizen.org/privilege/internet";
const std::string kPrivilegeBluetooth = "http://tizen.org/privilege/bluetooth";
const std::string kPrivilegeWifiDirect = "http://tizen.org/privilege/wifidirect";

// JS listener keys
static const std::string kJSListenerStatus = "status";
static const std::string kJSCurrentListenerId = "curListenerId";
static const std::string kJSTargetListenerId = "listenerId";
static const std::string kSuccess = "success";
static const std::string kError = "error";

// Arguments, passed from JS
static const std::string kJSCallbackId = "callbackId";
static const std::string kJSArgumentDeviceId = "deviceId";
static const std::string kJSArgumentServiceTypeNumber = "serviceTypeNumber";
static const std::string kJSArgumentChannel = "channel";
static const std::string kJSArgumentPayload = "payload";
static const std::string kJSArgumentTimeout = "timeout";
static const std::string kJSArgumentService = "service";
} // namespace

using namespace common;

ConvergenceInstance::ConvergenceInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ConvergenceInstance::x, this, _1, _2));
  REGISTER_SYNC("ConvergenceManager_stopDiscovery",
    ConvergenceManagerStopDiscovery);
  REGISTER_SYNC("Service_addListener", ServiceAddListener);
  REGISTER_SYNC("Service_removeListener", ServiceRemoveListener);
  REGISTER_SYNC("Service_createLocalService", ServiceCreateLocal);
  #undef REGISTER_SYNC
  #define REGISTER_ASYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ConvergenceInstance::x, this, _1, _2));
  REGISTER_ASYNC("ConvergenceManager_startDiscovery",
    ConvergenceManagerStartDiscovery);
  REGISTER_ASYNC("Service_connect", ServiceConnect);
  REGISTER_ASYNC("Service_disconnect", ServiceDisconnect);
  REGISTER_ASYNC("Service_read", ServiceRead);
  REGISTER_ASYNC("Service_start", ServiceStart);
  REGISTER_ASYNC("Service_stop", ServiceStop);
  REGISTER_ASYNC("Service_send", ServiceSend);
  #undef REGISTER_ASYNC
}

ConvergenceInstance::~ConvergenceInstance() {
}

void ConvergenceInstance::ReplyAsync(ConvergenceCallbacks callback_function_type,
  int curListenerId, bool isSuccess, picojson::object& param) {
  ScopeLogger();

  param[kJSListenerStatus] = picojson::value(isSuccess ? kSuccess : kError);
  param[kJSCurrentListenerId] = picojson::value(static_cast<double>(curListenerId));

  switch(callback_function_type) {
    case kConvergenceManagerDiscoveryCallback: {
      param[kJSTargetListenerId] = picojson::value("CONVERGENCE_DISCOVERY_LISTENER");
      break;
    }
    case kServiceConnectCallback: {
      param[kJSTargetListenerId] =
        picojson::value("CONVERGENCE_SERVICE_CONNECT_LISTENER");
      break;
    }
    case kServiceListenerCallback: {
      param[kJSTargetListenerId] =
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
  ScopeLogger();
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "timeout", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto start_discovery =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("start_discovery");

    // Start the discovery procedure
    ConvergenceManager::GetInstance(this).StartDiscovery(
      static_cast<long>(args.get(kJSArgumentTimeout).get<double>()));

    picojson::object& object = result->get<picojson::object>();
    object[kJSCallbackId] = args.get(kJSCallbackId);
    ReportSuccess(object);
  };

  auto start_discovery_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("start_discovery_result");
    result->get<picojson::object>()[kJSCallbackId] = args.get(kJSCallbackId);
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
  ScopeLogger();

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  // Running the discovery stop procedure
  ConvergenceManager::GetInstance(this).StopDiscovery();
  //out[kJSCallbackId] = args.get(kJSCallbackId);
  ReportSuccess(out);
}

void ConvergenceInstance::ServiceConnect(
  const picojson::value& args, picojson::object& out) {
  ScopeLogger();
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  auto connect =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("connect");

    // Running the service connect procedure
    ConvergenceManager::GetInstance(this).Connect(
      args.get(kJSArgumentDeviceId).to_str().c_str(),
      static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
      static_cast<int>(args.get(kJSCurrentListenerId).get<double>()));

    picojson::object& object = result->get<picojson::object>();
    object[kJSCallbackId] = args.get(kJSCallbackId);
    ReportSuccess(object);
  };

  auto connect_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("connect_result");

    result->get<picojson::object>()[kJSCallbackId] = args.get(kJSCallbackId);
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
  ScopeLogger();

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  //LoggerI("ARGS: %s", args.serialize().c_str());

  auto disconnect =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("disconnect");

    // Running the service disconnect procedure
    ConvergenceManager::GetInstance(this).Disconnect(
      args.get(kJSArgumentDeviceId).to_str().c_str(),
      static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()));

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
  ScopeLogger();
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto start = [this, args](const std::shared_ptr<picojson::value>& result) {

    // Running service start procedure
    ConvergenceManager::GetInstance(this).Start(
      args.get(kJSArgumentDeviceId).to_str().c_str(),
      static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
      args.get(kJSArgumentChannel),
      args.get(kJSArgumentPayload));

    picojson::object& object = result->get<picojson::object>();
    object[kJSCallbackId] = args.get(kJSCallbackId);
    ReportSuccess(object);
  };

  auto start_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()[kJSCallbackId] = args.get(kJSCallbackId);
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
  ScopeLogger();
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto read = [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("read");

    // Running service read procedure
    ConvergenceManager::GetInstance(this).Read(
      args.get(kJSArgumentDeviceId).to_str().c_str(),
      static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
      args.get(kJSArgumentChannel),
      args.get(kJSArgumentPayload));

    picojson::object& object = result->get<picojson::object>();
    object[kJSCallbackId] = args.get(kJSCallbackId);
    ReportSuccess(object);
  };

  auto read_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("read_result");
    result->get<picojson::object>()[kJSCallbackId] = args.get(kJSCallbackId);
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
  ScopeLogger();
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto send = [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("send");

    // Running service send procedure
    ConvergenceManager::GetInstance(this).Send(
      args.get(kJSArgumentDeviceId).to_str().c_str(),
      static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
      args.get(kJSArgumentChannel),
      args.get(kJSArgumentPayload));

    picojson::object& object = result->get<picojson::object>();
    object[kJSCallbackId] = args.get(kJSCallbackId);
    ReportSuccess(object);
  };

  auto send_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("send_result");
    result->get<picojson::object>()[kJSCallbackId] = args.get(kJSCallbackId);
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
  ScopeLogger();
  CHECK_EXIST(args, "callbackId", out)

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  auto stop = [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("stop");

    // Running service stop procedure
    ConvergenceManager::GetInstance(this).Stop(
      args.get(kJSArgumentDeviceId).to_str().c_str(),
      static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
      args.get(kJSArgumentChannel),
      args.get(kJSArgumentPayload));

    picojson::object& object = result->get<picojson::object>();
    object[kJSCallbackId] = args.get(kJSCallbackId);
    ReportSuccess(object);
  };

  auto stop_result =
    [this, args](const std::shared_ptr<picojson::value>& result) {
    ScopeLogger("stop_result");
    result->get<picojson::object>()[kJSCallbackId] = args.get(kJSCallbackId);
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
  ScopeLogger();

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  // Adding a listener to the service (completely sync API call)
  ConvergenceManager::GetInstance(this).SetListener(
    args.get(kJSArgumentDeviceId).to_str().c_str(),
    static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
    static_cast<int>(args.get(kJSCurrentListenerId).get<double>()));

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceRemoveListener(
  const picojson::value& args, picojson::object& out) {
  ScopeLogger();

  /*CHECK_PRIVILEGE_ACCESS(kPrivilegeInternet, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeBluetooth, &out)
  CHECK_PRIVILEGE_ACCESS(kPrivilegeWifiDirect, &out)*/

  LoggerI("ARGS: %s", args.serialize().c_str());

  // Removing the service listener (completely sunc API call)
  ConvergenceManager::GetInstance(this).UnsetListener(
    args.get(kJSArgumentDeviceId).to_str().c_str(),
    static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()));

  ReportSuccess(out);
}

void ConvergenceInstance::ServiceCreateLocal(
  const picojson::value& args, picojson::object& out) {
  ScopeLogger();

  LoggerI("ARGS: %s", args.serialize().c_str());

  // Removing the service listener (completely sunc API call)
  ConvergenceManager::GetInstance(this).CreateLocalService(
    static_cast<int>(args.get(kJSArgumentServiceTypeNumber).get<double>()),
    args.get(kJSArgumentService));

  ReportSuccess(out);
}


#undef CHECK_EXIST

} // namespace convergence
} // namespace extension
