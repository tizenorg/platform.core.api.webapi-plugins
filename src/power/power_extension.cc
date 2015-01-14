// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "power/power_extension.h"

#include "power/power_instance.h"

// This will be generated from power_api.js
extern const char kSource_power_api[];

common::Extension* CreateExtension() {
  return new PowerExtension;
}

PowerExtension::PowerExtension() {
  SetExtensionName("tizen.power");
  SetJavaScriptAPI(kSource_power_api);
}

PowerExtension::~PowerExtension() {}

common::Instance* PowerExtension::CreateInstance() {
  return new extension::power::PowerInstance;
}
