// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sound/sound_extension.h"

#include "sound/sound_instance.h"

// This will be generated from sound_api.js
extern const char kSource_sound_api[];

common::Extension* CreateExtension() {
  return new SoundExtension;
}

SoundExtension::SoundExtension() {
  SetExtensionName("tizen.sound");
  SetJavaScriptAPI(kSource_sound_api);
}

SoundExtension::~SoundExtension() {}

common::Instance* SoundExtension::CreateInstance() {
  return new extension::sound::SoundInstance;
}