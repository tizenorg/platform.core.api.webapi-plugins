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
    static long long GetTotalMemory();
    static long long GetAvailableMemory();
    static unsigned long GetCount(const std::string& property);
    static picojson::value GetPropertyValue(const std::string& prop);

    static void RegisterBatteryListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterBatteryListener();
    static void RegisterCpuListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterCpuListener();
    static void RegisterStorageListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterStorageListener();
    static void RegisterDisplayListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterDisplayListener();
    static void RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterDeviceOrientationListener();
    static void RegisterLocaleListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterLocaleListener();
    static void RegisterNetworkListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterNetworkListener();
    static void RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterWifiNetworkListener();
    static void RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterCellularNetworkListener();
    static void RegisterPeripheralListener(const SysteminfoUtilsCallback& callback);
    static void UnregisterPeripheralListener();

private:
    static void ReportBattery(picojson::object& out);
    static void ReportCpu(picojson::object& out);

    static void ReportDisplay(picojson::object& out);
    static void ReportDeviceOrientation(picojson::object& out);

    static void ReportBuild(picojson::object& out);
    static void ReportLocale(picojson::object& out);
    static void ReportNetwork(picojson::object& out);
    static void ReportWifiNetwork(picojson::object& out);
    static void ReportCellularNetwork(picojson::object& out);
    static void ReportSim(picojson::object& out);
    static void ReportPeripheral(picojson::object& out);

    static void ReportStorage(picojson::object& out);
};

typedef unsigned char byte;

class SystemInfoDeviceCapability {
public:
    static picojson::value GetCapability(const std::string& key);

    static bool IsBluetooth();
    static bool IsNfc();
    static bool IsNfcReservedPush();
    static unsigned short GetMultiTouchCount();
    static bool IsInputKeyboard();
    static bool IsInputKeyboardLayout();
    static bool IsWifi();
    static bool IsWifiDirect();
    static std::string GetPlatformName();
    static std::string GetPlatformVersion();
    static std::string GetWebApiVersion();
    static bool IsFmRadio();
    static bool IsOpengles();
    static bool IsOpenglesVersion11();
    static bool IsOpenglesVersion20();
    static std::string GetOpenglesTextureFormat();
    static bool IsSpeechRecognition();
    static bool IsSpeechSynthesis();
    static bool IsAccelerometer();
    static bool IsAccelerometerWakeup();
    static bool IsBarometer();
    static bool IsBarometerWakeup();
    static bool IsGyroscope();
    static bool IsGyroscopeWakeup();
    static bool IsCamera();
    static bool IsCameraFront();
    static bool IsCameraFrontFlash();
    static bool IsCameraBack();
    static bool IsCameraBackFlash();
    static bool IsLocation();
    static bool IsLocationGps();
    static bool IsLocationWps();
    static bool IsMicrophone();
    static bool IsUsbHost();
    static bool IsUsbAccessory();
    static bool IsScreenOutputRca();
    static bool IsScreenOutputHdmi();
    static bool IsGraphicsAcceleration();
    static bool IsPush();
    static bool IsTelephony();
    static bool IsTelephonyMMS();
    static bool IsTelephonySMS();
    static std::string GetPlatfomCoreCpuArch();
    static std::string GetPlatfomCoreFpuArch();
    static bool IsSipVoip();
    static bool IsMagnetometer();
    static bool IsMagnetometerWakeup();
    static bool IsPhotometer();
    static bool IsPhotometerWakeup();
    static bool IsProximity();
    static bool IsProximityWakeup();
    static bool IsTiltmeter();
    static bool IsTiltmeterWakeup();
    static bool IsDataEncryption();
    static bool IsAutoRotation();
    static bool IsVisionImageRecognition();
    static bool IsVisionQrcodeGeneration();
    static bool IsVisionQrcodeRecognition();
    static bool IsVisionFaceRecognition();
    static bool IsSecureElement();
    static std::string GetProfile();

    static std::string GetNativeAPIVersion();
    static std::string GetDuid();
    static bool IsScreenSizeNormal();
    static bool IsScreenSize480_800();
    static bool IsScreenSize720_1280();
    static bool IsShellAppWidget();
    static bool IsNativeOspCompatible();

    //additional capabilities
    static bool IsAccount();
    static bool IsArchive();
    static bool IsBadge();
    static bool IsBookmark();
    static bool IsCalendar();
    static bool IsContact();
    static bool IsContent();
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
    static bool IsScreenSize320_320();
private:
    static std::string GenerateDuid();
    static std::string GenerateId(char* pDeviceString);
    static void GenerateCrc64(char* pDeviceString, unsigned long long int* value);
    static std::string Base32Encode(byte* value);
};

} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_UTILS_H__
