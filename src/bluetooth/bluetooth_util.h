// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLUETOOTH_BLUETOOTH_UTIL_H_
#define BLUETOOTH_BLUETOOTH_UTIL_H_

#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"

namespace extension {
namespace bluetooth {
namespace util {

void CheckAccess(const std::string& privilege);

void AsyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response);
void AsyncResponse(double callback_handle, const common::PlatformResult& result);
void SyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response);

void FireEvent(const std::string& event, picojson::value& value);
void FireEvent(const std::string& event, const picojson::value& value);
void FireEvent(const std::string& event, const std::shared_ptr<picojson::value>& value);

void ReportSuccess(picojson::object& out);
void ReportSuccess(const picojson::value& result, picojson::object& out);

void ReportError(picojson::object& out);
void ReportError(const common::PlatformException& ex, picojson::object& out);

double GetAsyncCallbackHandle(const picojson::value& data);

const picojson::object& GetArguments(const picojson::value& data);

} // util
} // bluetooth
} // extension

#endif // BLUETOOTH_BLUETOOTH_UTIL_H_
