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

#include "tvchannel/tvchannel_extension.h"
#include "tvchannel/tvchannel_instance.h"

// This will be generated from datasync_api.js.
extern const char kSource_tvchannel_api[];

TVChannelExtension::TVChannelExtension() {
    SetExtensionName("tizen.tvchannel");
    SetJavaScriptAPI(kSource_tvchannel_api);
}

TVChannelExtension::~TVChannelExtension() {
}

common::Instance* TVChannelExtension::CreateInstance() {
    return new extension::tvchannel::TVChannelInstance();
}

// entry point
common::Extension* CreateExtension() {
    return new TVChannelExtension();
}

