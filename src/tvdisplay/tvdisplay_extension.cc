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
 
#include "tvdisplay/tvdisplay_extension.h"
#include "tvdisplay/tvdisplay_instance.h"

// This will be generated from tvdisplay_api.js.
extern const char kSource_tvdisplay_api[];


namespace extension {
namespace tvdisplay {

TVDisplayExtension::TVDisplayExtension() {
    SetExtensionName("tizen.tvdisplaycontrol");
    SetJavaScriptAPI(kSource_tvdisplay_api);
}

TVDisplayExtension::~TVDisplayExtension() {}

common::Instance* TVDisplayExtension::CreateInstance() {
    return new extension::tvdisplay::TVDisplayInstance();
}

}  // namespace tvdisplay
}  // namespace extension

// entry point
common::Extension* CreateExtension() {
  return new extension::tvdisplay::TVDisplayExtension();
}

