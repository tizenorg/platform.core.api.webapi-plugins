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

#include "push/push_extension.h"
#include "push/push_instance.h"

// This will be generated from push_api.js
extern const char kSource_push_api[];

namespace extension {
namespace push {

PushExtension::PushExtension() {
    SetExtensionName("tizen.push");
    SetJavaScriptAPI(kSource_push_api);
}

PushExtension::~PushExtension() {}

PushManager& PushExtension::manager() {
    // Initialize API on first request
    return PushManager::getInstance();
}

common::Instance* PushExtension::CreateInstance() {
    return new PushInstance;
}

}  // namespace push
}  // namespace extension

common::Extension* CreateExtension() {
    return new extension::push::PushExtension;
}
