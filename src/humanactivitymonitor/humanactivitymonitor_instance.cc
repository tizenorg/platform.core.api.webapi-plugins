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

#include "humanactivitymonitor/humanactivitymonitor_instance.h"

#include <functional>
#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_result.h"
#include "common/task-queue.h"
#include "humanactivitymonitor/humanactivitymonitor_manager.h"

namespace extension {
namespace humanactivitymonitor {

using common::PlatformResult;
using common::ErrorCode;
using common::TaskQueue;

HumanActivityMonitorInstance::HumanActivityMonitorInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&HumanActivityMonitorInstance::x, this, _1, _2));
  REGISTER_SYNC("HumanActivityMonitorManager_getHumanActivityData",
                HumanActivityMonitorManagerGetHumanActivityData);
  REGISTER_SYNC("HumanActivityMonitorManager_start",
                HumanActivityMonitorManagerStart);
  REGISTER_SYNC("HumanActivityMonitorManager_stop",
                HumanActivityMonitorManagerStop);
  REGISTER_SYNC("HumanActivityMonitorManager_setAccumulativePedometerListener",
                HumanActivityMonitorManagerSetAccumulativePedometerListener);
  REGISTER_SYNC("HumanActivityMonitorManager_unsetAccumulativePedometerListener",
                HumanActivityMonitorManagerUnsetAccumulativePedometerListener);
#undef REGISTER_SYNC
}

HumanActivityMonitorInstance::~HumanActivityMonitorInstance() {
  LoggerD("Enter");
}

PlatformResult HumanActivityMonitorInstance::Init() {
  LoggerD("Enter");
  if (!manager_) {

    manager_ = std::make_shared<HumanActivityMonitorManager>();
    const PlatformResult& result = manager_->Init();
    if (!result) {
      LOGGER(ERROR) << "Error initializing manager: " << result.message();
      manager_.reset();
      return result;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) { \
      ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, \
          name" is required argument"), &out); \
      return; \
    }


void HumanActivityMonitorInstance::HumanActivityMonitorManagerGetHumanActivityData(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)

  PlatformResult result = Init();
  if (!result) {
    LoggerE("Failed: Init()");
    ReportError(result, &out);
    return;
  }

  auto get = [this, args]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = args.get("callbackId");

    picojson::value data = picojson::value();
    PlatformResult result = manager_->GetHumanActivityData(
        args.get("type").get<std::string>(),
        &data);

    if (result) {
      ReportSuccess(data, response_obj);
    } else {
      LoggerE("Failed: manager_->GetHumanActivityData()");
      ReportError(result, &response_obj);
    }

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(get);

  ReportSuccess(out);
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStart(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)

  PlatformResult result = Init();
  if (!result) {
    LoggerE("Failed: Init()");
    ReportError(result, &out);
    return;
  }

  JsonCallback cb = [this, args](picojson::value* data) -> void {
    if (!data) {
      LOGGER(ERROR) << "No data passed to json callback";
      return;
    }

    picojson::object& data_o = data->get<picojson::object>();
    data_o["listenerId"] = args.get("listenerId");

    Instance::PostMessage(this, data->serialize().c_str());
  };

  result = manager_->SetListener(args.get("type").get<std::string>(), cb);
  if (result) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed: manager_->SetListener()");
    ReportError(result, &out);
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStop(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)

  PlatformResult result = Init();
  if (!result) {
    LoggerE("Failed: Init()");
    ReportError(result, &out);
    return;
  }

  result = manager_->UnsetListener(args.get("type").get<std::string>());
  if (result) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed: manager_->UnsetListener()");
    ReportError(result, &out);
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerSetAccumulativePedometerListener(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerUnsetAccumulativePedometerListener(
    const picojson::value& args, picojson::object& out) {
  // TODO(r.galka) implement
}

#undef CHECK_EXIST

}  // namespace humanactivitymonitor
}  // namespace extension
