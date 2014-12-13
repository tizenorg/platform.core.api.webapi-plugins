// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "systeminfo/systeminfo_instance.h"

#include <functional>
#include <memory>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include "systeminfo-utils.h"

namespace extension {
namespace systeminfo {

using namespace common;
using namespace extension::systeminfo;

//Callback functions declarations
static void OnBatteryChangedCallback();
static void OnCpuChangedCallback();
static void OnStorageChangedCallback();
static void OnDisplayChangedCallback();
static void OnDeviceOrientationChangedCallback();
static void OnLocaleChangedCallback();
static void OnNetworkChangedCallback();
static void OnWifiNetworkChangedCallback();
static void OnCellularNetworkChangedCallback();
static void OnPeripheralChangedCallback();

namespace {
const std::string kPropertyIdBattery = "BATTERY";
const std::string kPropertyIdCpu = "CPU";
const std::string kPropertyIdStorage = "STORAGE";
const std::string kPropertyIdDisplay = "DISPLAY";
const std::string kPropertyIdDeviceOrientation = "DEVICE_ORIENTATION";
const std::string kPropertyIdBuild = "BUILD";
const std::string kPropertyIdLocale = "LOCALE";
const std::string kPropertyIdNetwork = "NETWORK";
const std::string kPropertyIdWifiNetwork = "WIFI_NETWORK";
const std::string kPropertyIdCellularNetwork = "CELLULAR_NETWORK";
const std::string kPropertyIdSim = "SIM";
const std::string kPropertyIdPeripheral = "PERIPHERAL";
}

SysteminfoInstance& SysteminfoInstance::getInstance() {
    static SysteminfoInstance instance;
    return instance;
}

SysteminfoInstance::SysteminfoInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SysteminfoInstance::x, this, _1, _2));
  REGISTER_SYNC("SystemInfo_getCapabilities", GetCapabilities);
  REGISTER_SYNC("SystemInfo_getCapability", GetCapability);
  REGISTER_SYNC("SystemInfo_addPropertyValueChangeListener", AddPropertyValueChangeListener);
  REGISTER_SYNC("SystemInfo_removePropertyValueChangeListener", RemovePropertyValueChangeListener);
  #undef REGISTER_SYNC
  #define REGISTER_ASYNC(c,x) \
    RegisterHandler(c, std::bind(&SysteminfoInstance::x, this, _1, _2));
  REGISTER_ASYNC("SystemInfo_getPropertyValue", GetPropertyValue);
  #undef REGISTER_ASYNC
}

SysteminfoInstance::~SysteminfoInstance() {
}

void SysteminfoInstance::GetCapabilities(const picojson::value& args, picojson::object& out) {
    LoggerD("");
    picojson::value result = picojson::value(picojson::object());
    picojson::object& result_obj = result.get<picojson::object>();

    result_obj.insert(std::make_pair("bluetooth",
            SystemInfoDeviceCapability::IsBluetooth() ));
    result_obj.insert(std::make_pair("nfc",
            SystemInfoDeviceCapability::IsNfc() ));
    result_obj.insert(std::make_pair("nfcReservedPush",
            SystemInfoDeviceCapability::IsNfcReservedPush() ));
    result_obj.insert(std::make_pair("multiTouchCount",
            std::to_string(SystemInfoDeviceCapability::GetMultiTouchCount())));
    result_obj.insert(std::make_pair("inputKeyboard",
            SystemInfoDeviceCapability::IsInputKeyboard() ));
    result_obj.insert(std::make_pair("inputKeyboardLayout",
            SystemInfoDeviceCapability::IsInputKeyboardLayout() ));
    result_obj.insert(std::make_pair("wifi",
            SystemInfoDeviceCapability::IsWifi() ));
    result_obj.insert(std::make_pair("wifiDirect",
            SystemInfoDeviceCapability::IsWifiDirect() ));
    result_obj.insert(std::make_pair("platformName",
            SystemInfoDeviceCapability::GetPlatformName() ));
    result_obj.insert(std::make_pair("platformVersion",
            SystemInfoDeviceCapability::GetPlatformVersion() ));
    result_obj.insert(std::make_pair("webApiVersion",
            SystemInfoDeviceCapability::GetWebApiVersion() ));
    result_obj.insert(std::make_pair("fmRadio",
            SystemInfoDeviceCapability::IsFmRadio() ));
    result_obj.insert(std::make_pair("opengles",
            SystemInfoDeviceCapability::IsOpengles() ));
    result_obj.insert(std::make_pair("openglesVersion1_1",
            SystemInfoDeviceCapability::IsOpenglesVersion11() ));
    result_obj.insert(std::make_pair("openglesVersion2_0",
            SystemInfoDeviceCapability::IsOpenglesVersion20() ));
    result_obj.insert(std::make_pair("openglestextureFormat",
            SystemInfoDeviceCapability::GetOpenglesTextureFormat() ));
    result_obj.insert(std::make_pair("speechRecognition",
            SystemInfoDeviceCapability::IsSpeechRecognition() ));
    result_obj.insert(std::make_pair("speechSynthesis",
            SystemInfoDeviceCapability::IsSpeechSynthesis() ));
    result_obj.insert(std::make_pair("accelerometer",
            SystemInfoDeviceCapability::IsAccelerometer( ) ));
    result_obj.insert(std::make_pair("accelerometerWakeup",
            SystemInfoDeviceCapability::IsAccelerometerWakeup() ));
    result_obj.insert(std::make_pair("barometer",
            SystemInfoDeviceCapability::IsBarometer() ));
    result_obj.insert(std::make_pair("barometerWakeup",
            SystemInfoDeviceCapability::IsBarometerWakeup() ));
    result_obj.insert(std::make_pair("gyroscope",
            SystemInfoDeviceCapability::IsGyroscope() ));
    result_obj.insert(std::make_pair("gyroscopeWakeup",
            SystemInfoDeviceCapability::IsGyroscopeWakeup() ));
    result_obj.insert(std::make_pair("camera",
            SystemInfoDeviceCapability::IsCamera() ));
    result_obj.insert(std::make_pair("cameraFront",
            SystemInfoDeviceCapability::IsCameraFront() ));
    result_obj.insert(std::make_pair("cameraFrontFlash",
            SystemInfoDeviceCapability::IsCameraFrontFlash() ));
    result_obj.insert(std::make_pair("cameraBack",
            SystemInfoDeviceCapability::IsCameraBack() ));
    result_obj.insert(std::make_pair("cameraBackFlash",
            SystemInfoDeviceCapability::IsCameraBackFlash() ));
    result_obj.insert(std::make_pair("location",
            SystemInfoDeviceCapability::IsLocation() ));
    result_obj.insert(std::make_pair("locationGps",
            SystemInfoDeviceCapability::IsLocationGps() ));
    result_obj.insert(std::make_pair("locationWps",
            SystemInfoDeviceCapability::IsLocationWps() ));
    result_obj.insert(std::make_pair("microphone",
            SystemInfoDeviceCapability::IsMicrophone() ));
    result_obj.insert(std::make_pair("usbHost",
            SystemInfoDeviceCapability::IsUsbHost() ));
    result_obj.insert(std::make_pair("usbAccessory",
            SystemInfoDeviceCapability::IsUsbAccessory() ));
    result_obj.insert(std::make_pair("screenOutputRca",
            SystemInfoDeviceCapability::IsScreenOutputRca() ));
    result_obj.insert(std::make_pair("screenOutputHdmi",
            SystemInfoDeviceCapability::IsScreenOutputHdmi() ));
    result_obj.insert(std::make_pair("graphicsAcceleration",
            SystemInfoDeviceCapability::IsGraphicsAcceleration() ));
    result_obj.insert(std::make_pair("push",
            SystemInfoDeviceCapability::IsPush() ));
    result_obj.insert(std::make_pair("telephony",
            SystemInfoDeviceCapability::IsTelephony() ));
    result_obj.insert(std::make_pair("telephonyMms",
            SystemInfoDeviceCapability::IsTelephonyMMS() ));
    result_obj.insert(std::make_pair("telephonySms",
            SystemInfoDeviceCapability::IsTelephonySMS() ));
    result_obj.insert(std::make_pair("platformCoreCpuArch",
            SystemInfoDeviceCapability::GetPlatfomCoreCpuArch() ));
    result_obj.insert(std::make_pair("platformCoreFpuArch",
            SystemInfoDeviceCapability::GetPlatfomCoreFpuArch() ));
    result_obj.insert(std::make_pair("sipVoip",
            SystemInfoDeviceCapability::IsSipVoip() ));
    result_obj.insert(std::make_pair("magnetometer",
            SystemInfoDeviceCapability::IsMagnetometer() ));
    result_obj.insert(std::make_pair("magnetometerWakeup",
            SystemInfoDeviceCapability::IsMagnetometerWakeup() ));
    result_obj.insert(std::make_pair("photometer",
            SystemInfoDeviceCapability::IsPhotometer() ));
    result_obj.insert(std::make_pair("photometerWakeup",
            SystemInfoDeviceCapability::IsPhotometerWakeup() ));
    result_obj.insert(std::make_pair("proximity",
            SystemInfoDeviceCapability::IsProximity() ));
    result_obj.insert(std::make_pair("proximityWakeup",
            SystemInfoDeviceCapability::IsProximityWakeup() ));
    result_obj.insert(std::make_pair("tiltmeter",
            SystemInfoDeviceCapability::IsTiltmeter() ));
    result_obj.insert(std::make_pair("tiltmeterWakeup",
            SystemInfoDeviceCapability::IsTiltmeterWakeup() ));
    result_obj.insert(std::make_pair("dataEncryption",
            SystemInfoDeviceCapability::IsDataEncryption() ));
    result_obj.insert(std::make_pair("autoRotation",
            SystemInfoDeviceCapability::IsAutoRotation() ));
    result_obj.insert(std::make_pair("visionImageRecognition",
            SystemInfoDeviceCapability::IsVisionImageRecognition() ));
    result_obj.insert(std::make_pair("visionQrcodeGeneration",
            SystemInfoDeviceCapability::IsVisionQrcodeGeneration() ));
    result_obj.insert(std::make_pair("visionQrcodeRecognition",
            SystemInfoDeviceCapability::IsVisionQrcodeRecognition() ));
    result_obj.insert(std::make_pair("visionFaceRecognition",
            SystemInfoDeviceCapability::IsVisionFaceRecognition() ));
    result_obj.insert(std::make_pair("secureElement",
            SystemInfoDeviceCapability::IsSecureElement() ));
    result_obj.insert(std::make_pair("profile",
            SystemInfoDeviceCapability::GetProfile() ));
    result_obj.insert(std::make_pair("nativeApiVersion",
                SystemInfoDeviceCapability::GetNativeAPIVersion() ));
    result_obj.insert(std::make_pair("duid",
                SystemInfoDeviceCapability::GetDuid() ));
    result_obj.insert(std::make_pair("screenSizeNormal",
                SystemInfoDeviceCapability::IsScreenSizeNormal() ));
    result_obj.insert(std::make_pair("screenSize480_800",
                SystemInfoDeviceCapability::IsScreenSize480_800() ));
    result_obj.insert(std::make_pair("screenSize720_1280",
                SystemInfoDeviceCapability::IsScreenSize720_1280() ));
    result_obj.insert(std::make_pair("shellAppWidget",
                SystemInfoDeviceCapability::IsShellAppWidget() ));
    result_obj.insert(std::make_pair("nativeOspCompatible",
                SystemInfoDeviceCapability::IsNativeOspCompatible() ));
    ReportSuccess(result, out);
    LoggerD("Success");
}

void SysteminfoInstance::GetCapability(const picojson::value& args, picojson::object& out) {

    const std::string& key = args.get("key").get<std::string>();
    LoggerD("Getting capability with key: %s ", key.c_str());

    picojson::value result = SystemInfoDeviceCapability::GetCapability(key);
    ReportSuccess(result, out);
    LoggerD("Success");
}

void SysteminfoInstance::GetPropertyValue(const picojson::value& args, picojson::object& out) {
    LoggerD("");
    const double callback_id = args.get("callbackId").get<double>();

    const std::string& prop_id = args.get("property").get<std::string>();
    LoggerD("Getting property with id: %s ", prop_id.c_str());

    auto get = [this, prop_id, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting");
        try {
            picojson::value result = SysteminfoUtils::GetPropertyValue(prop_id);
            ReportSuccess(result, response->get<picojson::object>());
        } catch (const PlatformException& e) {
            ReportError(e,response->get<picojson::object>());
        }
    };

    auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        obj.insert(std::make_pair("cmd", picojson::value("SystemInfo_getPropertyValue")));
        PostMessage(response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>
        (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SysteminfoInstance::AddPropertyValueChangeListener(const picojson::value& args, picojson::object& out) {
    LoggerD("");

    try {
        // Check type of property for which listener should be registered
        const std::string& property_name = args.get("property").get<std::string>();
        LoggerD("Adding listener for property with id: %s ", property_name.c_str());
        if (property_name == kPropertyIdBattery) {
            SysteminfoUtils::RegisterBatteryListener(OnBatteryChangedCallback);
        } else if (property_name == kPropertyIdCpu) {
            SysteminfoUtils::RegisterCpuListener(OnCpuChangedCallback);
        } else if (property_name == kPropertyIdStorage) {
            SysteminfoUtils::RegisterStorageListener(OnStorageChangedCallback);
        } else if (property_name == kPropertyIdDisplay) {
            SysteminfoUtils::RegisterDisplayListener(OnDisplayChangedCallback);
        } else if (property_name == kPropertyIdDeviceOrientation) {
            SysteminfoUtils::RegisterDeviceOrientationListener(OnDeviceOrientationChangedCallback);
        } else if (property_name == kPropertyIdBuild) {
            LoggerW("BUILD property's value is a fixed value");
            throw NotSupportedException("BUILD property's value is a fixed value");
        } else if (property_name == kPropertyIdLocale) {
            SysteminfoUtils::RegisterLocaleListener(OnLocaleChangedCallback);
        } else if (property_name == kPropertyIdNetwork) {
            SysteminfoUtils::RegisterNetworkListener(OnNetworkChangedCallback);
        } else if (property_name == kPropertyIdWifiNetwork) {
            SysteminfoUtils::RegisterWifiNetworkListener(OnWifiNetworkChangedCallback);
        } else if (property_name == kPropertyIdCellularNetwork) {
            SysteminfoUtils::RegisterCellularNetworkListener(OnCellularNetworkChangedCallback);
        } else if (property_name == kPropertyIdSim) {
            //SIM listeners are not supported by core API, so we just pass over
            LoggerW("SIM listener is not supported by Core API - ignoring");
        } else if (property_name == kPropertyIdPeripheral) {
            SysteminfoUtils::RegisterPeripheralListener(OnPeripheralChangedCallback);
        } else {
            LoggerE("Not supported property");
            throw InvalidValuesException("Not supported property");
        }
        ReportSuccess(out);
        LoggerD("Success");
    } catch (const PlatformException& e) {
        LoggerD("Error");
        ReportError(e, out);
    }

}

void SysteminfoInstance::RemovePropertyValueChangeListener(const picojson::value& args, picojson::object& out) {
    LoggerD("");

    try {
        // Check type of property for which listener should be removed
        const std::string& property_name = args.get("property").get<std::string>();
        LoggerD("Removing listener for property with id: %s ", property_name.c_str());
        if (property_name == kPropertyIdBattery) {
            SysteminfoUtils::UnregisterBatteryListener();
        } else if (property_name == kPropertyIdCpu) {
            SysteminfoUtils::UnregisterCpuListener();
        } else if (property_name == kPropertyIdStorage) {
            SysteminfoUtils::UnregisterStorageListener();
        } else if (property_name == kPropertyIdDisplay) {
            SysteminfoUtils::UnregisterDisplayListener();
        } else if (property_name == kPropertyIdDeviceOrientation) {
            SysteminfoUtils::UnregisterDeviceOrientationListener();
        } else if (property_name == kPropertyIdBuild) {
            LoggerW("BUILD property's value is a fixed value");
            throw NotSupportedException("BUILD property's value is a fixed value");
        } else if (property_name == kPropertyIdLocale) {
            SysteminfoUtils::UnregisterLocaleListener();
        } else if (property_name == kPropertyIdNetwork) {
            SysteminfoUtils::UnregisterNetworkListener();
        } else if (property_name == kPropertyIdWifiNetwork) {
            SysteminfoUtils::UnregisterWifiNetworkListener();
        } else if (property_name == kPropertyIdCellularNetwork) {
            SysteminfoUtils::UnregisterCellularNetworkListener();
        } else if (property_name == kPropertyIdSim) {
            //SIM listeners are not supported by core API, so we just pass over
            LoggerW("SIM listener is not supported by Core API - ignoring");
        } else if (property_name == kPropertyIdPeripheral) {
            SysteminfoUtils::UnregisterPeripheralListener();
        } else {
            LoggerE("Not supported property");
            throw InvalidValuesException("Not supported property");
        }
        ReportSuccess(out);
        LoggerD("Success");
    } catch (const PlatformException& e) {
        LoggerD("Error");
        ReportError(e, out);
    }
}

static void ReportSuccess(const picojson::value& result, picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("success")));
  out.insert(std::make_pair("result", result));
}


//Callback functions
void OnBatteryChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdBattery);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdBattery);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnCpuChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdCpu);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdCpu);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnStorageChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdStorage);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnDisplayChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
                std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdDisplay);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdDisplay);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnDeviceOrientationChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdDeviceOrientation);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdDeviceOrientation);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnLocaleChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdLocale);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdLocale);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnNetworkChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdNetwork);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdNetwork);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnWifiNetworkChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdWifiNetwork);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdWifiNetwork);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnCellularNetworkChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdCellularNetwork);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdCellularNetwork);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

void OnPeripheralChangedCallback()
{
    LoggerD("");
    const std::shared_ptr<picojson::value>& response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()["propertyId"] = picojson::value(kPropertyIdPeripheral);

    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdPeripheral);
    ReportSuccess(result,response->get<picojson::object>());

    SysteminfoInstance::getInstance().PostMessage(response->serialize().c_str());
}

} // namespace systeminfo
} // namespace extension
