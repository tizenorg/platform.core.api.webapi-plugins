// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "datacontrol/datacontrol_extension.h"

#include "datacontrol/datacontrol_instance.h"

// This will be generated from datacontrol_api.js
extern const char kSource_datacontrol_api[];

common::Extension* CreateExtension() {
  return new DatacontrolExtension;
}

DatacontrolExtension::DatacontrolExtension() {
  SetExtensionName("tizen.datacontrol");
  SetJavaScriptAPI(kSource_datacontrol_api);
}

DatacontrolExtension::~DatacontrolExtension() {}

common::Instance* DatacontrolExtension::CreateInstance() {
  return new extension::datacontrol::DatacontrolInstance;
}