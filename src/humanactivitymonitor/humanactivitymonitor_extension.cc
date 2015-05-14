// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "humanactivitymonitor/humanactivitymonitor_extension.h"

#include "humanactivitymonitor/humanactivitymonitor_instance.h"

// This will be generated from humanactivitymonitor_api.js
extern const char kSource_humanactivitymonitor_api[];

common::Extension* CreateExtension() {
  return new HumanActivityMonitorExtension;
}

HumanActivityMonitorExtension::HumanActivityMonitorExtension() {
  SetExtensionName("tizen.humanactivitymonitor");
  SetJavaScriptAPI(kSource_humanactivitymonitor_api);
}

HumanActivityMonitorExtension::~HumanActivityMonitorExtension() {}

common::Instance* HumanActivityMonitorExtension::CreateInstance() {
  return new extension::humanactivitymonitor::HumanActivityMonitorInstance;
}
