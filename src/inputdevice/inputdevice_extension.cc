// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../inputdevice/inputdevice_extension.h"
#include "../inputdevice/inputdevice_instance.h"

// This will be generated from inputdevice_api.js
extern const char kSource_inputdevice_api[];

namespace extension {
namespace inputdevice {

InputDeviceExtension::InputDeviceExtension() {
    SetExtensionName("tizen.inputdevice");
    SetJavaScriptAPI(kSource_inputdevice_api);
}

InputDeviceExtension::~InputDeviceExtension() {}

common::Instance* InputDeviceExtension::CreateInstance() {
    return new InputDeviceInstance;
}

}  // namespace inputdevice
}  // namespace extension

common::Extension* CreateExtension() {
    return new extension::inputdevice::InputDeviceExtension;
}
