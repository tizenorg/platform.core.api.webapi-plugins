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

#include "bluetooth_health_application.h"

#include "common/converter.h"
#include "common/logger.h"
#include "common/extension.h"

#include "bluetooth_health_profile_handler.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

namespace {
const std::string kDataType = "dataType";
const std::string kName = "name";
const std::string kId = "_id";
} // namespace

using namespace common;

BluetoothHealthApplication::BluetoothHealthApplication(
    BluetoothHealthProfileHandler& handler)
    : handler_(handler) {
}

void BluetoothHealthApplication::Unregister(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);

  handler_.UnregisterSinkAppAsync(FromJson<std::string>(args, "id"),
                                  util::GetAsyncCallbackHandle(data));

  tools::ReportSuccess(out);
}

void BluetoothHealthApplication::ToJson(short data_type,
                                        const std::string& name,
                                        const char* id,
                                        picojson::object* out) {
  out->insert(std::make_pair(kDataType, picojson::value(static_cast<double>(data_type))));
  out->insert(std::make_pair(kName, picojson::value(name)));
  out->insert(std::make_pair(kId, picojson::value(id)));
}

} // namespace bluetooth
} // namespace extension
