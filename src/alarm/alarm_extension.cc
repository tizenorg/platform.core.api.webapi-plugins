/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
