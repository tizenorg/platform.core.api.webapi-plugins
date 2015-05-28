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

#include "common/logger.h"
#include "websetting/websetting_extension.h"

extern const char kSource_websetting_api[];

common::Extension* CreateExtension() {
  return new WebSettingExtension();
}

WebSettingExtension::WebSettingExtension() {
  LoggerD("Entered");
  SetExtensionName("tizen.websetting");
  SetJavaScriptAPI(kSource_websetting_api);
}

WebSettingExtension::~WebSettingExtension() {
  LoggerD("Entered");
}

common::Instance* WebSettingExtension::CreateInstance() {
  LoggerD("Entered");
  return new common::ParsedInstance();
}
