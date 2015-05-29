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

#include "networkbearerselection/networkbearerselection_extension.h"

#include "networkbearerselection/networkbearerselection_instance.h"

// This will be generated from networkbearerselection_api.js
extern const char kSource_networkbearerselection_api[];

common::Extension* CreateExtension() {
  return new NetworkBearerSelectionExtension;
}

NetworkBearerSelectionExtension::NetworkBearerSelectionExtension() {
  SetExtensionName("tizen.networkbearerselection");
  SetJavaScriptAPI(kSource_networkbearerselection_api);
}

NetworkBearerSelectionExtension::~NetworkBearerSelectionExtension() {}

common::Instance* NetworkBearerSelectionExtension::CreateInstance() {
  return new extension::networkbearerselection::NetworkBearerSelectionInstance;
}
