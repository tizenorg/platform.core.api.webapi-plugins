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
#include <functional>
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

class SysteminfoInstance;

typedef std::function<void(SysteminfoInstance& instance)> SysteminfoUtilsCallback;

class SysteminfoUtils {
 public:
  static common::PlatformResult GetTotalMemory(long long& result);
  static common::PlatformResult GetAvailableMemory(long long& result);
  static common::PlatformResult GetCount(const std::string& property, unsigned long& ret);
  static common::PlatformResult GetPropertyValue(
      const std::string& prop, bool is_array_type, picojson::value& res);

  static common::PlatformResult RegisterBatteryListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance);
  static common::PlatformResult UnregisterBatteryListener();
  static common::PlatformResult RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                                    SysteminfoInstance& instance);
  static common::PlatformResult UnregisterCpuListener();
  static common::PlatformResult RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance);
  static common::PlatformResult UnregisterStorageListener();
  static common::PlatformResult RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance);
  static common::PlatformResult UnregisterDisplayListener();
  static common::PlatformResult RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                                  SysteminfoInstance& instance);
  static common::PlatformResult UnregisterDeviceOrientationListener();
  static common::PlatformResult RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance);
  static common::PlatformResult UnregisterLocaleListener();
  static common::PlatformResult RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance);
  static common::PlatformResult UnregisterNetworkListener();
  static common::PlatformResult RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance);
  static common::PlatformResult UnregisterWifiNetworkListener();
  static common::PlatformResult RegisterEthernetNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance);
  static common::PlatformResult UnregisterEthernetNetworkListener();
  static common::PlatformResult RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance);
  static common::PlatformResult UnregisterCellularNetworkListener();
  static common::PlatformResult RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance);
  static common::PlatformResult UnregisterPeripheralListener();
  static common::PlatformResult RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance);
  static common::PlatformResult UnregisterMemoryListener();
  static common::PlatformResult RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance);
  static common::PlatformResult UnregisterCameraFlashListener();

 private:
  static common::PlatformResult ReportProperty(const std::string& property, int index,
                                               picojson::object& res_obj);
  static common::PlatformResult ReportBattery(picojson::object& out);
  static common::PlatformResult ReportCpu(picojson::object& out);

  static common::PlatformResult ReportDisplay(picojson::object& out);
  static common::PlatformResult ReportDeviceOrientation(picojson::object& out);

  static common::PlatformResult ReportBuild(picojson::object& out);
  static common::PlatformResult ReportLocale(picojson::object& out);
  static common::PlatformResult ReportNetwork(picojson::object& out);
  static common::PlatformResult ReportWifiNetwork(picojson::object& out);
  static common::PlatformResult ReportEthernetNetwork(picojson::object& out);
  static common::PlatformResult ReportCellularNetwork(picojson::object& out, unsigned long count);
  static common::PlatformResult ReportSim(picojson::object& out, unsigned long count);
  static common::PlatformResult ReportPeripheral(picojson::object& out);
  static common::PlatformResult ReportMemory(picojson::object& out);
  static common::PlatformResult ReportCameraFlash(picojson::object& out, unsigned long count);

  static common::PlatformResult ReportStorage(picojson::object& out);
};

typedef unsigned char byte;

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_UTILS_H__
