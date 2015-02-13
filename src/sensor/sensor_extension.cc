// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensor/sensor_extension.h"
#include "sensor/sensor_instance.h"

// This will be generated from sensor_api.js
extern const char kSource_sensor_api[];

common::Extension* CreateExtension() {
  return new SensorExtension;
}

SensorExtension::SensorExtension() {
  SetExtensionName("tizen.sensorservice");
  SetJavaScriptAPI(kSource_sensor_api);
}

SensorExtension::~SensorExtension() {}

common::Instance* SensorExtension::CreateInstance() {
  return &extension::sensor::SensorInstance::GetInstance();
}
