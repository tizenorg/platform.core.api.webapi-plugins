// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "radio/radio_extension.h"
#include "radio/radio_instance.h"

#include <common/logger.h>

// This will be generated from radio_api.js
extern const char kSource_radio_api[];

common::Extension* CreateExtension() {
  return new RadioExtension;
}

RadioExtension::RadioExtension() {
  SetExtensionName("tizen.fmradio");
  SetJavaScriptAPI(kSource_radio_api);
  LoggerD("RadioExtension()");
}

RadioExtension::~RadioExtension() {}

common::Instance* RadioExtension::CreateInstance() {
    return &extension::radio::RadioInstance::getInstance();
}
