// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory/callhistory_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace callhistory {

using namespace common;

CallHistoryInstance::CallHistoryInstance() : history_(*this) {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&CallHistoryInstance::x, this, _1, _2));
  REGISTER_SYNC("CallHistory_remove", Remove);
  REGISTER_SYNC("CallHistory_addChangeListener", AddChangeListener);
  REGISTER_SYNC("CallHistory_removeChangeListener", RemoveChangeListener);
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
  history_.find(args.get<picojson::object>());
  ReportSuccess(out);
}

void CallHistoryInstance::Remove(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = history_.remove(args.get<picojson::object>());
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void CallHistoryInstance::RemoveBatch(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = history_.removeBatch(args.get<picojson::object>());
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void CallHistoryInstance::RemoveAll(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  history_.removeAll(args.get<picojson::object>());
  ReportSuccess(out);
}

void CallHistoryInstance::AddChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = history_.startCallHistoryChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void CallHistoryInstance::RemoveChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = history_.stopCallHistoryChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void CallHistoryInstance::CallHistoryChange(picojson::object& data) {
  LoggerD("Entered");
  picojson::value event = picojson::value(data);
  picojson::object& obj = event.get<picojson::object>();
  obj["listenerId"] = picojson::value("CallHistoryChangeCallback");

  PostMessage(event.serialize().c_str());
}

} // namespace callhistory
} // namespace extension
