// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bluetooth_extension.h"
#include "bluetooth_instance.h"

namespace {
  const char* kBluetooth = "tizen.bluetooth";
}
// This will be generated from bluetooth_api.js.
extern const char kSource_bluetooth_api[];

common::Extension* CreateExtension() {
  return new BluetoothExtension;
}

BluetoothExtension::BluetoothExtension() {
  SetExtensionName(kBluetooth);
  SetJavaScriptAPI(kSource_bluetooth_api);
  const char* entry_points[] = {
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

BluetoothExtension::~BluetoothExtension() {}

common::Instance* BluetoothExtension::CreateInstance() {
  return &extension::bluetooth::BluetoothInstance::GetInstance();
}
