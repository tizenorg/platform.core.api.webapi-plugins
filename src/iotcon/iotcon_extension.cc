/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "iotcon/iotcon_extension.h"

#include "iotcon/iotcon_instance.h"

// This will be generated from iotcon_api.js
extern const char kSource_iotcon_api[];

namespace extension {
namespace iotcon {

IotconExtension::IotconExtension() {
  SetExtensionName("tizen.iotcon");
  SetJavaScriptAPI(kSource_iotcon_api);

  const char* entry_points[] = {
    "tizen.IotconOption",
    "tizen.Query",
    "tizen.QueryFilter",
    "tizen.Representation",
    "tizen.Response",
    "tizen.State",
    nullptr
  };
  SetExtraJSEntryPoints(entry_points);
}

IotconExtension::~IotconExtension() {
}

common::Instance* IotconExtension::CreateInstance() {
  return new IotconInstance();
}

}  // namespace iotcon
}  // namespace extension

common::Extension* CreateExtension() {
  return new extension::iotcon::IotconExtension();
}
