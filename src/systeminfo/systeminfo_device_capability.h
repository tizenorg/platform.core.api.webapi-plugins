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

#ifndef WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_DEVICE_CAPABILITY_H__
#define WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_DEVICE_CAPABILITY_H__

#include <string>
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace systeminfo {

class SystemInfoDeviceCapability {
 public:
  static common::PlatformResult GetCapability(const std::string& key, picojson::value* result);
  static common::PlatformResult GetValueBool(const char *key, bool* value);
  static common::PlatformResult GetValueInt(const char *key, int* value);
  static common::PlatformResult GetValueString(const char *key, std::string* str_value);

  static common::PlatformResult IsInputKeyboardLayout(bool* result);
  static common::PlatformResult GetOpenglesTextureFormat(std::string* result);
  static common::PlatformResult GetPlatfomCoreCpuArch(std::string* return_value);
  static common::PlatformResult GetPlatfomCoreFpuArch(std::string* return_value);
  static common::PlatformResult GetProfile(std::string* return_value);
  static common::PlatformResult GetPlatformCoreCpuFrequency(int* return_value);
  static common::PlatformResult IsNativeOspCompatible(bool* result);
  static common::PlatformResult GetNativeAPIVersion(std::string* return_value);
  static common::PlatformResult GetPlatformVersionName(std::string* result);
  static bool IsBluetoothAlwaysOn();
  static bool IsScreen();
};

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_DEVICE_CAPABILITY_H__
