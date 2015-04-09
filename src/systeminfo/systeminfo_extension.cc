// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "systeminfo/systeminfo_extension.h"

#include "systeminfo/systeminfo_instance.h"

// This will be generated from systeminfo_api.js
extern const char kSource_systeminfo_api[];

common::Extension* CreateExtension() {
  return new SysteminfoExtension;
}

SysteminfoExtension::SysteminfoExtension() {
  SetExtensionName("tizen.systeminfo");
  SetJavaScriptAPI(kSource_systeminfo_api);
}

SysteminfoExtension::~SysteminfoExtension() {}

common::Instance* SysteminfoExtension::CreateInstance() {
  return new extension::systeminfo::SysteminfoInstance();
}
