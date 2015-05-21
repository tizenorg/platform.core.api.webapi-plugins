// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "systemsetting/systemsetting_extension.h"

#include "systemsetting/systemsetting_instance.h"

// This will be generated from systemsetting_api.js
extern const char kSource_systemsetting_api[];

common::Extension* CreateExtension() {
  return new SystemSettingExtension;
}

SystemSettingExtension::SystemSettingExtension() {
  SetExtensionName("tizen.systemsetting");
  SetJavaScriptAPI(kSource_systemsetting_api);
}

SystemSettingExtension::~SystemSettingExtension() {}

common::Instance* SystemSettingExtension::CreateInstance() {
  return new extension::systemsetting::SystemSettingInstance;
}
