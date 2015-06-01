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

#include "bluetooth_service_handler.h"

#include "common/converter.h"
#include "common/logger.h"
#include "common/extension.h"

#include "bluetooth_adapter.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

using namespace common;

BluetoothServiceHandler::BluetoothServiceHandler(BluetoothAdapter& adapter)
    : adapter_(adapter) {
}

void BluetoothServiceHandler::Unregister(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);

  adapter_.UnregisterUUID(FromJson<std::string>(args, "uuid"),
                          util::GetAsyncCallbackHandle(data));

  tools::ReportSuccess(out);
}

} // namespace bluetooth
} // namespace extension
