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

#ifndef BLUETOOTH_BLUETOOTH_UTIL_H_
#define BLUETOOTH_BLUETOOTH_UTIL_H_

#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace bluetooth {
namespace util {

double GetAsyncCallbackHandle(const picojson::value& data);

const picojson::object& GetArguments(const picojson::value& data);

common::PlatformResult GetBluetoothError(int error_code, const std::string& hint);
std::string GetBluetoothErrorMessage(int error_code);

} // util
} // bluetooth
} // extension

#endif // BLUETOOTH_BLUETOOTH_UTIL_H_
