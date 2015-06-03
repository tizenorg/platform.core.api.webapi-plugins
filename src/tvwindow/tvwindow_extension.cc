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
 
#include "tvwindow/tvwindow_extension.h"
#include "tvwindow/tvwindow_instance.h"

// This will be generated from datasync_api.js.
extern const char kSource_tvwindow_api[];

namespace extension {
namespace tvwindow {

TVWindowExtension::TVWindowExtension() {
  SetExtensionName("tizen.tvwindow");
  SetJavaScriptAPI(kSource_tvwindow_api);
}

TVWindowExtension::~TVWindowExtension() {}

TVWindowManager& TVWindowExtension::manager() {
  // Initialize API on first request
  return TVWindowManager::getInstance();
}

common::Instance* TVWindowExtension::CreateInstance() {
  return new TVWindowInstance(*this);
}

}  // namespace tvwindow
}  // namespace extension

// entry point
common::Extension* CreateExtension() {
  return new extension::tvwindow::TVWindowExtension;
}
