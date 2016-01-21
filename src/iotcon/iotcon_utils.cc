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

#include "iotcon_utils.h"

#include <memory>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/scope_exit.h"

namespace extension {
namespace iotcon {

using common::PlatformResult;
using common::ErrorCode;

PlatformResult IotconUtils::ResourceToJson(iotcon_resource_h handle, picojson::value* res) {
  LoggerD("Entered");

  // TODO implement conversion
  return LogAndCreateResult(ErrorCode::NOT_SUPPORTED_ERR, "Not implemented yet");
}

} // namespace iotcon
} // namespace extension
