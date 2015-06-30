// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../tvinputdevice/tvinputdevice_extension.h"
#include "../tvinputdevice/tvinputdevice_instance.h"

// This will be generated from tvinputdevice_api.js
extern const char kSource_tvinputdevice_api[];

namespace extension {
namespace tvinputdevice {

TVInputDeviceExtension::TVInputDeviceExtension() {
    SetExtensionName("tizen.tvinputdevice");
    SetJavaScriptAPI(kSource_tvinputdevice_api);
}

TVInputDeviceExtension::~TVInputDeviceExtension() {}

common::Instance* TVInputDeviceExtension::CreateInstance() {
    return new TVInputDeviceInstance;
}

}  // namespace tvinputdevice
}  // namespace extension

common::Extension* CreateExtension() {
    return new extension::tvinputdevice::TVInputDeviceExtension;
}
