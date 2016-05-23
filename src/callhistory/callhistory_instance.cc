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

#include "callhistory/callhistory_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/tools.h"
#include "common/platform_exception.h"

namespace extension {
namespace callhistory {

namespace {
// The privileges that required in CallHistory API
const std::string kPrivilegeCallHistoryRead = "http://tizen.org/privilege/callhistory.read";
const std::string kPrivilegeCallHistoryWrite = "http://tizen.org/privilege/callhistory.write";
}

using namespace common;

CallHistoryInstance::CallHistoryInstance() : history_(*this) {
  LoggerD("Entered");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&CallHistoryInstance::x, this, _1, _2));
  REGISTER_SYNC("CallHistory_remove", Remove);
  REGISTER_SYNC("CallHistory_addChangeListener", AddChangeListener);
  REGISTER_SYNC("CallHistory_removeChangeListener", RemoveChangeListener);
  REGISTER_SYNC("CallHistory_setMissedDirection", SetMissedDirection);
#undef REGISTER_SYNC
#define REGISTER_ASYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&CallHistoryInstance::x, this, _1, _2));
  REGISTER_ASYNC("CallHistory_find", Find);
  REGISTER_ASYNC("CallHistory_removeBatch", RemoveBatch);
  REGISTER_ASYNC("CallHistory_removeAll", RemoveAll);
#undef REGISTER_ASYNC
}

CallHistoryInstance::~CallHistoryInstance() {
}

void CallHistoryInstance::Find(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeCallHistoryRead, &out);
  history_.find(args.get<picojson::object>());
  ReportSuccess(out);
}

void CallHistoryInstance::Remove(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeCallHistoryWrite, &out);
  PlatformResult result = history_.remove(args.get<picojson::object>());
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out);
  }
}

void CallHistoryInstance::RemoveBatch(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeCallHistoryWrite, &out);
  PlatformResult result = history_.removeBatch(args.get<picojson::object>());
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out);
  }
}

void CallHistoryInstance::RemoveAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeCallHistoryWrite, &out);
  history_.removeAll(args.get<picojson::object>());
  ReportSuccess(out);
}

void CallHistoryInstance::AddChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeCallHistoryRead, &out);
  PlatformResult result = history_.startCallHistoryChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out);
  }
}

void CallHistoryInstance::RemoveChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_PRIVILEGE_ACCESS(kPrivilegeCallHistoryRead, &out);
  PlatformResult result = history_.stopCallHistoryChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LogAndReportError(result, &out);
  }
}

void CallHistoryInstance::SetMissedDirection(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  if (!args.contains("uid")) {
    LoggerD("args doesn't contain attribute 'uid'");
    ReportError(out);
    return;
  }

  int uid = std::atoi(args.get("uid").get<std::string>().c_str());

  PlatformResult result = history_.setMissedDirection(uid);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(out);
  }
}

void CallHistoryInstance::CallHistoryChange(picojson::object& data) {
  LoggerD("Entered");
  picojson::value event = picojson::value(data);
  picojson::object& obj = event.get<picojson::object>();
  obj["listenerId"] = picojson::value("CallHistoryChangeCallback");

  Instance::PostMessage(this, event.serialize().c_str());
}

} // namespace callhistory
} // namespace extension