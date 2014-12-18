// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvaudio/tvaudio_extension.h"
#include "tvaudio/tvaudio_instance.h"

// This will be generated from tvaudio_api.js
extern const char kSource_tvaudio_api[];

namespace extension {
namespace tvaudio {

TVAudioExtension::TVAudioExtension() {
    SetExtensionName("tizen.tvaudio");
    SetJavaScriptAPI(kSource_tvaudio_api);
}

TVAudioExtension::~TVAudioExtension() {}

AudioControlManager& TVAudioExtension::manager() {
    // Initialize API on first request
    return AudioControlManager::getInstance();
}

common::Instance* TVAudioExtension::CreateInstance() {
    return new TVAudioInstance;
}

}  // namespace tvaudio
}  // namespace extension

common::Extension* CreateExtension() {
    return new extension::tvaudio::TVAudioExtension;
}
