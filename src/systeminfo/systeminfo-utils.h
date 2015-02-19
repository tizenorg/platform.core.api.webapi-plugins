/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

namespace extension {
namespace systeminfo {

struct CpuInfo {
  long long usr;
  long long nice;
  long long system;
  long long idle;
  double load;
};

typedef std::function<void(void)> SysteminfoUtilsCallback;

class SysteminfoUtils {
 public:
  static common::PlatformResult GetTotalMemory(long long& result);
  static common::PlatformResult GetAvailableMemory(long long& result);
  static common::PlatformResult GetCount(const std::string& property, unsigned long& ret);
  static common::PlatformResult GetPropertyValue(
      const std::string& prop, bool is_array_type, picojson::value& res);

  static common::PlatformResult RegisterBatteryListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterBatteryListener();
  static common::PlatformResult RegisterCpuListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterCpuListener();
  static common::PlatformResult RegisterStorageListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterStorageListener();
  static common::PlatformResult RegisterDisplayListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterDisplayListener();
  static common::PlatformResult RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterDeviceOrientationListener();
  static common::PlatformResult RegisterLocaleListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterLocaleListener();
  static common::PlatformResult RegisterNetworkListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterNetworkListener();
  static common::PlatformResult RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterWifiNetworkListener();
  static common::PlatformResult RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterCellularNetworkListener();
  static common::PlatformResult RegisterPeripheralListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterPeripheralListener();
  static common::PlatformResult RegisterMemoryListener(const SysteminfoUtilsCallback& callback);
  static common::PlatformResult UnregisterMemoryListener();

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
  static common::PlatformResult ReportCellularNetwork(picojson::object& out);
  static common::PlatformResult ReportSim(picojson::object& out, unsigned long count);
  static common::PlatformResult ReportPeripheral(picojson::object& out);
  static common::PlatformResult ReportMemory(picojson::object& out);

  static common::PlatformResult ReportStorage(picojson::object& out);
};

typedef unsigned char byte;

class SystemInfoDeviceCapability {
 public:
  static common::PlatformResult GetCapability(const std::string& key, picojson::value& result);
  static common::PlatformResult GetValueBool(const char *key, bool& value);
  static common::PlatformResult GetValueInt(const char *key, int& value);
  static common::PlatformResult GetValueString(const char *key, std::string& str_value);

  static common::PlatformResult IsBluetooth(bool& result);
  static common::PlatformResult IsInputKeyboardLayout(bool& result);
  static common::PlatformResult GetOpenglesTextureFormat(std::string& result);
  static common::PlatformResult GetPlatfomCoreCpuArch(std::string& return_value);
  static common::PlatformResult GetPlatfomCoreFpuArch(std::string& return_value);
  static common::PlatformResult GetProfile(std::string& return_value);
  static std::string GetDuid();

//  //additional capabilities
  static bool IsAccount();
  static bool IsArchive();
  static bool IsBadge();
  static bool IsBookmark();
  static bool IsCalendar();
  static bool IsContact();
  static bool IsContent();
  static bool IsDataControl();
  static bool IsDataSync();
  static bool IsDownload();
  static bool IsExif();
  static bool IsGamePad();
  static bool IsMessagingEmail();
  static bool IsMessaging();
  static bool IsBluetootHealth();
  static bool IsBluetoothAlwaysOn();
  static bool IsNfcEmulation();
  static bool IsNotification();
  static bool IsPower();
  static bool IsWebSetting();
  static bool IsSystemSetting();
  static bool IsSystemSettingHomeScreen();
  static bool IsSystemSettingLockScreen();
  static bool IsSystemSettingIncomingCall();
  static bool IsSystemSettingNotificationEmail();
  static bool IsBattery();
  static bool IsCoreAPI();
  static bool IsPressure();
  static bool IsUltraviolet();
  static bool IsPedometer();
  static bool IsWristUp();
  static bool IsHrm();
  static bool IsScreen();
  static common::PlatformResult IsScreenSize320_320(bool& return_value);
 private:
  static std::string GenerateDuid();
  static std::string GenerateId(char* pDeviceString);
  static void GenerateCrc64(char* pDeviceString, unsigned long long int* value);
  static std::string Base32Encode(byte* value);
};

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_UTILS_H__
