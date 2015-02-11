// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
