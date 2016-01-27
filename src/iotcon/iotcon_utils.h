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

#ifndef WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__
#define WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__

#include <string>

#include <iotcon.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace iotcon {

class IotconUtils {
 public:
  static common::PlatformResult ResourceToJson(iotcon_resource_h handle, picojson::value* res);
};

} // namespace iotcon
} // namespace extension

#endif // WEBAPI_PLUGINS_IOTCON_IOTCON_UTILS_H__
