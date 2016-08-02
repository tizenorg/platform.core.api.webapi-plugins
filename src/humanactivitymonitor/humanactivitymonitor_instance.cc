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
#include "common/tools.h"
#include "humanactivitymonitor/humanactivitymonitor_manager.h"

namespace extension {
namespace humanactivitymonitor {

namespace {

const std::string kPrivilegeHealthInfo = "http://tizen.org/privilege/healthinfo";
const std::string kPrivilegeLocation = "http://tizen.org/privilege/location";

const int DEFAULT_HRM_INTERVAL = 1440;  // 1440 (1 day) default value for HRM's interval
const int DEFAULT_RETENTION_PERIOD = 1; // 1 hour default value for retention period
}  // namespace

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
  REGISTER_SYNC("HumanActivityMonitorManager_addActivityRecognitionListener",
                HumanActivityMonitorManagerAddActivityRecognitionListener);
  REGISTER_SYNC("HumanActivityMonitorManager_removeActivityRecognitionListener",
                HumanActivityMonitorManagerRemoveActivityRecognitionListener);
  REGISTER_SYNC("HumanActivityMonitorManager_startRecorder",
                HumanActivityMonitorManagerStartRecorder);
  REGISTER_SYNC("HumanActivityMonitorManager_stopRecorder",
                HumanActivityMonitorManagerStopRecorder);
  REGISTER_SYNC("HumanActivityMonitorManager_readRecorderData",
                 HumanActivityMonitorManagerReadRecorderData);
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
      LogAndReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, \
          name" is required argument"), &out); \
      return; \
    }


void HumanActivityMonitorInstance::HumanActivityMonitorManagerGetHumanActivityData(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)

  CHECK_PRIVILEGE_ACCESS(kPrivilegeHealthInfo, &out);

  const auto type = args.get("type").get<std::string>();

  if (kActivityTypeGps == type) {
    CHECK_PRIVILEGE_ACCESS(kPrivilegeLocation, &out);
  }

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  const auto callback_id = args.get("callbackId").get<double>();

  auto get = [this, type, callback_id]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = picojson::value(callback_id);

    picojson::value data = picojson::value();
    PlatformResult result = manager_->GetHumanActivityData(type, &data);

    if (result) {
      ReportSuccess(data, response_obj);
    } else {
      LogAndReportError(result, &response_obj, ("Failed: manager_->GetHumanActivityData()"));
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

  CHECK_PRIVILEGE_ACCESS(kPrivilegeHealthInfo, &out);

  const auto type = args.get("type").get<std::string>();

  if (kActivityTypeGps == type) {
    CHECK_PRIVILEGE_ACCESS(kPrivilegeLocation, &out);
  }

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  const auto listener_id = args.get("listenerId").get<std::string>();

  JsonCallback cb = [this, listener_id](picojson::value* data) -> void {
    if (!data) {
      LOGGER(ERROR) << "No data passed to json callback";
      return;
    }

    picojson::object& data_o = data->get<picojson::object>();
    data_o["listenerId"] = picojson::value(listener_id);

    Instance::PostMessage(this, data->serialize().c_str());
  };

  result = manager_->SetListener(type, cb, args);
  if (result) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out, ("Failed: manager_->SetListener()"));
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStop(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)

  CHECK_PRIVILEGE_ACCESS(kPrivilegeHealthInfo, &out);

  const auto type = args.get("type").get<std::string>();

  if (kActivityTypeGps == type) {
    CHECK_PRIVILEGE_ACCESS(kPrivilegeLocation, &out);
  }

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  result = manager_->UnsetListener(type);
  if (result) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out, ("Failed: manager_->UnsetListener()"));
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerAddActivityRecognitionListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeHealthInfo, &out);
  CHECK_EXIST(args, "type", out)

  const auto& type = args.get("type").get<std::string>();

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  const auto& listener_id = args.get("listenerId").get<std::string>();

  JsonCallback cb = [this, listener_id](picojson::value* data) -> void {
    if (!data) {
      LOGGER(ERROR) << "No data passed to json callback";
      return;
    }

    picojson::object& data_o = data->get<picojson::object>();
    data_o["listenerId"] = picojson::value(listener_id);

    Instance::PostMessage(this, data->serialize().c_str());
  };

  long watch_id = 0;

  result = manager_->AddActivityRecognitionListener(type, cb, &watch_id);
  if (result) {
    out["watchId"] = picojson::value(static_cast<double>(watch_id));
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out, ("Failed: manager_->AddActivityRecognitionListener()"));
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerRemoveActivityRecognitionListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeHealthInfo, &out);
  CHECK_EXIST(args, "watchId", out)

  const long watchId = static_cast<long>(args.get("watchId").get<double>());

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  result = manager_->RemoveActivityRecognitionListener(watchId);
  if (result) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out, ("Failed: manager_->RemoveActivityRecognitionListener()"));
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStartRecorder(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  CHECK_PRIVILEGE_ACCESS(kPrivilegeHealthInfo, &out);
  CHECK_EXIST(args, "type", out)

  const auto& type = args.get("type").get<std::string>();

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  int interval = DEFAULT_HRM_INTERVAL;
  int retention_period = DEFAULT_RETENTION_PERIOD;

  if (args.contains("options")) {
    const auto& options = args.get("options");
    auto& js_interval = options.get("interval");
    auto& js_retention_period = options.get("retentionPeriod");

    if (js_interval.is<double>()) {
      interval = js_interval.get<double>();
    }

    if (js_retention_period.is<double>()) {
      retention_period = js_retention_period.get<double>();
    }
  }

  LoggerD("interval: %d retentionPeriod: %d", interval, retention_period);

  result = manager_->StartDataRecorder(type, interval, retention_period);
  if (result) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out, ("Failed: manager_->StartDataRecorder()"));
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerStopRecorder(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)

  const auto& type = args.get("type").get<std::string>();

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  result = manager_->StopDataRecorder(type);
  if (result) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out, ("Failed: manager_->StopDataRecorder()"));
  }
}

void HumanActivityMonitorInstance::HumanActivityMonitorManagerReadRecorderData(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "type", out)
  CHECK_EXIST(args, "query", out)

  const auto& type = args.get("type").get<std::string>();
  const auto& query = args.get("query");

  PlatformResult result = Init();
  if (!result) {
    LogAndReportError(result, &out, ("Failed: Init()"));
    return;
  }

  const auto callback_id = args.get("callbackId").get<double>();

  auto get = [this, type, query, callback_id]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = picojson::value(callback_id);

    //picojson::value data = picojson::value();
    picojson::value array_value{picojson::array{}};
    auto* array = &array_value.get<picojson::array>();

    PlatformResult result = manager_->ReadRecorderData(type, array, query);

    if (result) {
      ReportSuccess(array_value, response_obj);
    } else {
      LogAndReportError(result, &response_obj, ("Failed: manager_->ReadRecorderData()"));
    }

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(get);

  ReportSuccess(out);
}

#undef CHECK_EXIST

}  // namespace humanactivitymonitor
}  // namespace extension
