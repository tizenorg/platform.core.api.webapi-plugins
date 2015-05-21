// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "alarm_extension.h"
#include "alarm_instance.h"

namespace {
const char* kAlarm = "tizen.alarm";
const char* kAlarmRelative = "tizen.AlarmRelative";
const char* kAlarmAbsolute = "tizen.AlarmAbsolute";
}

// This will be generated from alarm_api.js.
extern const char kSource_alarm_api[];

common::Extension* CreateExtension() {
  return new AlarmExtension;
}

AlarmExtension::AlarmExtension() {
  SetExtensionName(kAlarm);
  SetJavaScriptAPI(kSource_alarm_api);
  const char* entry_points[] = {
      kAlarmRelative,
      kAlarmAbsolute,
      NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

AlarmExtension::~AlarmExtension() {}

common::Instance* AlarmExtension::CreateInstance() {
  return new extension::alarm::AlarmInstance();
}
