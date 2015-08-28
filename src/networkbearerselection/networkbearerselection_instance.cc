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

#include "networkbearerselection/networkbearerselection_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

namespace extension {
namespace networkbearerselection {

using namespace common;
using namespace extension::networkbearerselection;

NetworkBearerSelectionInstance::NetworkBearerSelectionInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(      \
      c, std::bind(&NetworkBearerSelectionInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterSyncHandler(           \
      c, std::bind(&NetworkBearerSelectionInstance::x, this, _1, _2));
  REGISTER_SYNC("NetworkBearerSelection_requestRouteToHost",
                NetworkBearerSelectionRequestRouteToHost);
  REGISTER_ASYNC("NetworkBearerSelection_releaseRouteToHost",
                 NetworkBearerSelectionReleaseRouteToHost);
#undef REGISTER_SYNC
#undef REGISTER_ASYNC

  NetworkBearerSelectionManager::GetInstance()->AddListener(this);
}

NetworkBearerSelectionInstance::~NetworkBearerSelectionInstance() {
  LoggerD("Enter");
  NetworkBearerSelectionManager::GetInstance()->RemoveListener(this);
}

#define CHECK_EXIST(args, name, out)                                       \
  if (!args.contains(name)) {                                              \
    ReportError(TypeMismatchException(name " is required argument"), out); \
    return;                                                                \
  }

void NetworkBearerSelectionInstance::NetworkBearerSelectionRequestRouteToHost(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");

  CHECK_EXIST(args, "domainName", out)
  CHECK_EXIST(args, "id", out)

  const std::string& domainName = args.get("domainName").get<std::string>();
  const int listenerId = static_cast<int>(args.get("id").get<double>());

  auto get = [=]()->void {
    NetworkBearerSelectionManager::GetInstance()->requestRouteToHost(
        domainName);
  };

  listenerMap.insert(std::make_pair(domainName, listenerId));

  common::TaskQueue::GetInstance().Async(get);
  ReportSuccess(out);
}

void NetworkBearerSelectionInstance::NetworkBearerSelectionReleaseRouteToHost(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "domainName", out)
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& domainName = args.get("domainName").get<std::string>();

  auto get = [ this, callback_id ](bool status)->void {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    if (status)
      ReportSuccess(obj);
    else
      ReportError(UnknownException("PLATFORM ERROR"), obj);
    obj["callbackId"] = picojson::value(callback_id);
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto reply = [=](bool status)->void {
    LoggerD("enter");
    common::TaskQueue::GetInstance().Async(std::bind(get, status));
  };

  bool status =
      NetworkBearerSelectionManager::GetInstance()->releaseRouteToHost(
          domainName, reply);
  if (status) {
    ReportSuccess(out);
  } else {
    ReportError(out);
  }
}

void NetworkBearerSelectionInstance::onNBSSuccess(
    const std::string& domain_name) {
  LoggerD("enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj["domainName"] = picojson::value(domain_name);
  obj["state"] = picojson::value("Success");

  auto iterRange = listenerMap.equal_range(domain_name);
  for (auto iter = iterRange.first; iter != iterRange.second; ++iter) {
    auto listenerId = (*iter).second;
    obj["listenerId"] = picojson::value("NetworkBearerSelectionCallback_" +
                                        std::to_string(listenerId));
    obj["id"] = picojson::value(static_cast<double>(listenerId));
    LoggerD("Posting: %s", event.serialize().c_str());
    Instance::PostMessage(this, event.serialize().c_str());
  }
}

void NetworkBearerSelectionInstance::onNBSError(const std::string& domain_name,
                                                const std::string& info) {
  LoggerD("enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  ReportError(UnknownException(info), obj);
  obj["domainName"] = picojson::value(domain_name);
  obj["state"] = picojson::value("Error");

  auto iterRange = listenerMap.equal_range(domain_name);
  for (auto iter = iterRange.first; iter != iterRange.second; ++iter) {
    auto listenerId = (*iter).second;
    obj["listenerId"] = picojson::value("NetworkBearerSelectionCallback_" +
                                        std::to_string(listenerId));
    obj["id"] = picojson::value(static_cast<double>(listenerId));
    LoggerD("Posting: %s", event.serialize().c_str());
    Instance::PostMessage(this, event.serialize().c_str());
  }
  listenerMap.erase(domain_name);
}

void NetworkBearerSelectionInstance::onNBSDisconnect(
    const std::string& domain_name) {
  LoggerD("enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj["domainName"] = picojson::value(domain_name);
  obj["state"] = picojson::value("Disconnected");

  auto iterRange = listenerMap.equal_range(domain_name);
  for (auto iter = iterRange.first; iter != iterRange.second; ++iter) {
    auto listenerId = (*iter).second;
    obj["listenerId"] = picojson::value("NetworkBearerSelectionCallback_" +
                                        std::to_string(listenerId));
    obj["id"] = picojson::value(static_cast<double>(listenerId));
    LoggerD("Posting: %s", event.serialize().c_str());
    Instance::PostMessage(this, event.serialize().c_str());
  }
  listenerMap.erase(domain_name);
}

#undef CHECK_EXIST

}  // namespace networkbearerselection
}  // namespace extension
