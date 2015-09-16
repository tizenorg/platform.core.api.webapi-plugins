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

#ifndef WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_UTILS_H__
#define WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_UTILS_H__

#include <string>
#include <system_settings.h>
#include <vconf.h>
#include <vconf-internal-keys.h>
#include <ITapiSim.h>
#include <TelNetwork.h>
#include "common/picojson.h"
#include "common/platform_result.h"
#include "systeminfo/systeminfo_device_capability.h"

namespace extension {
namespace systeminfo {

struct CpuInfo {
  long long usr;
  long long nice;
  long long system;
  long long idle;
  double load;
};

class SysteminfoUtils {
 public:
  static common::PlatformResult GetVconfInt(const char *key, int *value);
  static common::PlatformResult GetRuntimeInfoString(system_settings_key_e key,
                                                     std::string* platform_string);
  static common::PlatformResult CheckTelephonySupport();
  static common::PlatformResult CheckCameraFlashSupport();
  static common::PlatformResult CheckIfEthernetNetworkSupported();
  static common::PlatformResult GetTotalMemory(long long* result);
  static common::PlatformResult GetAvailableMemory(long long* result);
  static common::PlatformResult RegisterVconfCallback(const char *in_key, vconf_callback_fn cb,
                                                            void* event_ptr);
  static common::PlatformResult UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb);
  static common::PlatformResult RegisterTapiChangeCallback(TapiHandle *handle, const char *noti_id,
                                                           tapi_notification_cb callback,
                                                           void *user_data);
  static common::PlatformResult UnregisterTapiChangeCallback(TapiHandle *handle,
                                                             const char *noti_id);
};

typedef unsigned char byte;

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_UTILS_H__
