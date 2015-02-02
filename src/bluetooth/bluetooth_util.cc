// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bluetooth_util.h"

#include "common/logger.h"
#include "common/task-queue.h"

#include "bluetooth_instance.h"

namespace extension {
namespace bluetooth {
namespace util {

namespace {
const char* JSON_CALLBACK_ID = "callbackId";
const char* JSON_LISTENER_ID = "listenerId";
const char* JSON_STATUS = "status";
const char* JSON_RESULT = "result";
const char* JSON_CALLBACK_SUCCCESS = "success";
const char* JSON_CALLBACK_ERROR = "error";
const char* JSON_DATA = "args";
} // namespace


void CheckAccess(const std::string& privilege) {
    // TODO: check access to privilege, throw exception on failure
}

void AsyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response) {
    common::TaskQueue::GetInstance().Async<picojson::value>([callback_handle](const std::shared_ptr<picojson::value>& response) {
        SyncResponse(callback_handle, response);
    }, response);
}

void SyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response) {
    auto& obj = response->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callback_handle);
    BluetoothInstance::GetInstance().PostMessage(response->serialize().c_str());
}

void FireEvent(const std::string& event, picojson::value& value) {
    auto& obj = value.get<picojson::object>();
    obj[JSON_LISTENER_ID] = picojson::value(event);
    BluetoothInstance::GetInstance().PostMessage(value.serialize().c_str());
}

void FireEvent(const std::string& event, const picojson::value& value) {
    picojson::value v{value};
    FireEvent(event, v);
}

void FireEvent(const std::string& event, const std::shared_ptr<picojson::value>& value) {
    FireEvent(event, *value.get());
}

void ReportSuccess(picojson::object& out) {
    out.insert(std::make_pair(JSON_STATUS, picojson::value(JSON_CALLBACK_SUCCCESS)));
}

void ReportSuccess(const picojson::value& result, picojson::object& out) {
    ReportSuccess(out);
    out.insert(std::make_pair(JSON_RESULT, result));
}

void ReportError(picojson::object& out) {
    out.insert(std::make_pair(JSON_STATUS, picojson::value(JSON_CALLBACK_ERROR)));
}

void ReportError(const common::PlatformException& ex, picojson::object& out) {
    ReportError(out);
    out.insert(std::make_pair(JSON_CALLBACK_ERROR, ex.ToJSON()));
}

double GetAsyncCallbackHandle(const picojson::value& data) {
    return data.get(JSON_CALLBACK_ID).get<double>();
}

const picojson::object& GetArguments(const picojson::value& data) {
    return data.get<picojson::object>();
}

} // util
} // bluetooth
} // extension
