// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "systeminfo/systeminfo_instance.h"

#include <device/led.h>
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
static void OnBatteryChangedCallback(SysteminfoInstance& instance);
static void OnCpuChangedCallback(SysteminfoInstance& instance);
static void OnStorageChangedCallback(SysteminfoInstance& instance);
static void OnDisplayChangedCallback(SysteminfoInstance& instance);
static void OnDeviceOrientationChangedCallback(SysteminfoInstance& instance);
static void OnLocaleChangedCallback(SysteminfoInstance& instance);
static void OnNetworkChangedCallback(SysteminfoInstance& instance);
static void OnWifiNetworkChangedCallback(SysteminfoInstance& instance);
static void OnCellularNetworkChangedCallback(SysteminfoInstance& instance);
static void OnPeripheralChangedCallback(SysteminfoInstance& instance);
static void OnMemoryChangedCallback(SysteminfoInstance& instance);
static void OnBrigthnessChangedCallback(SysteminfoInstance& instance);

namespace {
const std::string kPropertyIdString = "propertyId";
const std::string kListenerIdString = "listenerId";
const std::string kListenerConstValue = "SysteminfoCommonListenerLabel";

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
const std::string kPropertyIdMemory= "MEMORY";
const std::string kPropertyIdCameraFlash= "CAMERA_FLASH";

#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

#define DEFAULT_REPORT_BOOL_CAPABILITY(str_name, feature_name) \
  ret = SystemInfoDeviceCapability::GetValueBool(feature_name, &bool_value); \
  if (ret.IsError()) { \
    ReportError(ret,&out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(bool_value)));

#define REPORT_BOOL_CAPABILITY(str_name, method) \
  ret = method(&bool_value); \
  if (ret.IsError()) { \
    ReportError(ret,&out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(bool_value)));

#define DEFAULT_REPORT_INT_CAPABILITY(str_name, feature_name) \
  ret = SystemInfoDeviceCapability::GetValueInt(feature_name, &int_value); \
  if (ret.IsError()) { \
    ReportError(ret,&out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(std::to_string(int_value))));

#define DEFAULT_REPORT_STRING_CAPABILITY(str_name, feature_name) \
  ret = SystemInfoDeviceCapability::GetValueString(feature_name, &str_value); \
  if (ret.IsError()) { \
    ReportError(ret,&out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(str_value)));

#define REPORT_STRING_CAPABILITY(str_name, method) \
  ret = method(&str_value); \
  if (ret.IsError()) { \
    ReportError(ret,&out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(str_value)));
}

SysteminfoInstance::SysteminfoInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&SysteminfoInstance::x, this, _1, _2));
  REGISTER_SYNC("SystemInfo_getCapabilities", GetCapabilities);
  REGISTER_SYNC("SystemInfo_getCapability", GetCapability);
  REGISTER_SYNC("SystemInfo_addPropertyValueChangeListener", AddPropertyValueChangeListener);
  REGISTER_SYNC("SystemInfo_removePropertyValueChangeListener", RemovePropertyValueChangeListener);
  REGISTER_SYNC("SystemInfo_getTotalMemory", GetTotalMemory);
  REGISTER_SYNC("SystemInfo_getAvailableMemory", GetAvailableMemory);
  REGISTER_SYNC("SystemInfo_getCount", GetCount);
  REGISTER_SYNC("SystemInfo_setBrightness", SetBrightness);
  REGISTER_SYNC("SystemInfo_getBrightness", GetBrightness);
  REGISTER_SYNC("SystemInfo_getMaxBrightness", GetMaxBrightness);

#undef REGISTER_SYNC
#define REGISTER_ASYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&SysteminfoInstance::x, this, _1, _2));
  REGISTER_ASYNC("SystemInfo_getPropertyValue", GetPropertyValue);
  REGISTER_ASYNC("SystemInfo_getPropertyValueArray", GetPropertyValueArray);
#undef REGISTER_ASYNC
}

SysteminfoInstance::~SysteminfoInstance() {
  LoggerD("Entered");
  //TODO Below solution is temporary
  //Implementation should be changed that each SysteminfoInstance object
  //should have own SystemInfoListeners manager
  SysteminfoUtils::UnregisterBatteryListener();
  SysteminfoUtils::UnregisterCpuListener();
  SysteminfoUtils::UnregisterStorageListener();
  SysteminfoUtils::UnregisterDisplayListener();
  SysteminfoUtils::UnregisterDeviceOrientationListener();
  SysteminfoUtils::UnregisterLocaleListener();
  SysteminfoUtils::UnregisterNetworkListener();
  SysteminfoUtils::UnregisterWifiNetworkListener();
  SysteminfoUtils::UnregisterCellularNetworkListener();
  SysteminfoUtils::UnregisterPeripheralListener();
  SysteminfoUtils::UnregisterMemoryListener();
}

void SysteminfoInstance::GetCapabilities(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  bool bool_value = false;
  int int_value = 0;
  std::string str_value = "";
  PlatformResult ret(ErrorCode::NO_ERROR);
  DEFAULT_REPORT_BOOL_CAPABILITY("bluetooth", "tizen.org/feature/network.bluetooth")
  DEFAULT_REPORT_BOOL_CAPABILITY("nfc", "tizen.org/feature/network.nfc")
  DEFAULT_REPORT_BOOL_CAPABILITY("nfcReservedPush", "tizen.org/feature/network.nfc.reserved_push")
  DEFAULT_REPORT_INT_CAPABILITY("multiTouchCount", "tizen.org/feature/multi_point_touch.point_count")
  DEFAULT_REPORT_BOOL_CAPABILITY("inputKeyboard", "tizen.org/feature/input.keyboard")
  REPORT_BOOL_CAPABILITY("inputKeyboardLayout", SystemInfoDeviceCapability::IsInputKeyboardLayout)
  DEFAULT_REPORT_BOOL_CAPABILITY("wifi", "tizen.org/feature/network.wifi")
  DEFAULT_REPORT_BOOL_CAPABILITY("wifiDirect", "tizen.org/feature/network.wifi.direct")
  DEFAULT_REPORT_STRING_CAPABILITY("platformName", "tizen.org/system/platform.name")
  DEFAULT_REPORT_STRING_CAPABILITY("platformVersion", "tizen.org/feature/platform.version")
  DEFAULT_REPORT_STRING_CAPABILITY("webApiVersion", "tizen.org/feature/platform.web.api.version")
  DEFAULT_REPORT_BOOL_CAPABILITY("fmRadio", "tizen.org/feature/fmradio")
  DEFAULT_REPORT_BOOL_CAPABILITY("opengles", "tizen.org/feature/opengles")
  DEFAULT_REPORT_BOOL_CAPABILITY("openglesVersion1_1", "tizen.org/feature/opengles.version.1_1")
  DEFAULT_REPORT_BOOL_CAPABILITY("openglesVersion2_0", "tizen.org/feature/opengles.version.2_0")
  REPORT_STRING_CAPABILITY("openglestextureFormat",
                           SystemInfoDeviceCapability::GetOpenglesTextureFormat)
  DEFAULT_REPORT_BOOL_CAPABILITY("speechRecognition", "tizen.org/feature/speech.recognition")
  DEFAULT_REPORT_BOOL_CAPABILITY("speechSynthesis", "tizen.org/feature/speech.synthesis")
  DEFAULT_REPORT_BOOL_CAPABILITY("accelerometer", "tizen.org/feature/sensor.accelerometer")
  DEFAULT_REPORT_BOOL_CAPABILITY("accelerometerWakeup", "tizen.org/feature/sensor.accelerometer.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("barometer", "tizen.org/feature/sensor.barometer")
  DEFAULT_REPORT_BOOL_CAPABILITY("barometerWakeup", "tizen.org/feature/sensor.barometer.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("gyroscope", "tizen.org/feature/sensor.gyroscope")
  DEFAULT_REPORT_BOOL_CAPABILITY("gyroscopeWakeup", "tizen.org/feature/sensor.gyroscope.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("camera", "tizen.org/feature/camera")
  DEFAULT_REPORT_BOOL_CAPABILITY("cameraFront", "tizen.org/feature/camera.front")
  DEFAULT_REPORT_BOOL_CAPABILITY("cameraFrontFlash", "tizen.org/feature/camera.front.flash")
  DEFAULT_REPORT_BOOL_CAPABILITY("cameraBack", "tizen.org/feature/camera.back")
  DEFAULT_REPORT_BOOL_CAPABILITY("cameraBackFlash", "tizen.org/feature/camera.back.flash")
  DEFAULT_REPORT_BOOL_CAPABILITY("location", "tizen.org/feature/location")
  DEFAULT_REPORT_BOOL_CAPABILITY("locationGps", "tizen.org/feature/location.gps")
  DEFAULT_REPORT_BOOL_CAPABILITY("locationWps", "tizen.org/feature/location.wps")
  DEFAULT_REPORT_BOOL_CAPABILITY("microphone", "tizen.org/feature/microphone")
  DEFAULT_REPORT_BOOL_CAPABILITY("usbHost", "tizen.org/feature/usb.host")
  DEFAULT_REPORT_BOOL_CAPABILITY("usbAccessory", "tizen.org/feature/usb.accessory")
  DEFAULT_REPORT_BOOL_CAPABILITY("screenOutputRca", "tizen.org/feature/screen.output.rca")
  DEFAULT_REPORT_BOOL_CAPABILITY("screenOutputHdmi", "tizen.org/feature/screen.output.hdmi")
  DEFAULT_REPORT_BOOL_CAPABILITY("graphicsAcceleration", "tizen.org/feature/graphics.acceleration")
  DEFAULT_REPORT_BOOL_CAPABILITY("push", "tizen.org/feature/network.push")
  DEFAULT_REPORT_BOOL_CAPABILITY("telephony", "tizen.org/feature/network.telephony")
  DEFAULT_REPORT_BOOL_CAPABILITY("telephonyMms", "tizen.org/feature/network.telephony.mms")
  DEFAULT_REPORT_BOOL_CAPABILITY("telephonySms", "tizen.org/feature/network.telephony.sms")
  REPORT_STRING_CAPABILITY("platformCoreCpuArch",
                             SystemInfoDeviceCapability::GetPlatfomCoreCpuArch)
  REPORT_STRING_CAPABILITY("platformCoreFpuArch",
                             SystemInfoDeviceCapability::GetPlatfomCoreFpuArch)
  DEFAULT_REPORT_BOOL_CAPABILITY("sipVoip", "tizen.org/feature/sip.voip")
  DEFAULT_REPORT_BOOL_CAPABILITY("magnetometer", "tizen.org/feature/sensor.magnetometer")
  DEFAULT_REPORT_BOOL_CAPABILITY("magnetometerWakeup", "tizen.org/feature/sensor.magnetometer.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("photometer", "tizen.org/feature/sensor.photometer")
  DEFAULT_REPORT_BOOL_CAPABILITY("photometerWakeup", "tizen.org/feature/sensor.photometer.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("proximity", "tizen.org/feature/sensor.proximity")
  DEFAULT_REPORT_BOOL_CAPABILITY("proximityWakeup", "tizen.org/feature/sensor.proximity.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("tiltmeter", "tizen.org/feature/sensor.tiltmeter")
  DEFAULT_REPORT_BOOL_CAPABILITY("tiltmeterWakeup", "tizen.org/feature/sensor.tiltmeter.wakeup")
  DEFAULT_REPORT_BOOL_CAPABILITY("dataEncryption", "tizen.org/feature/database.encryption")
  DEFAULT_REPORT_BOOL_CAPABILITY("autoRotation", "tizen.org/feature/screen.auto_rotation")
  DEFAULT_REPORT_BOOL_CAPABILITY("visionImageRecognition", "tizen.org/feature/vision.image_recognition")
  DEFAULT_REPORT_BOOL_CAPABILITY("visionQrcodeGeneration", "tizen.org/feature/vision.qrcode_generation")
  DEFAULT_REPORT_BOOL_CAPABILITY("visionQrcodeRecognition", "tizen.org/feature/vision.qrcode_recognition")
  DEFAULT_REPORT_BOOL_CAPABILITY("visionFaceRecognition", "tizen.org/feature/vision.face_recognition")
  DEFAULT_REPORT_BOOL_CAPABILITY("secureElement", "tizen.org/feature/network.secure_element")
  REPORT_STRING_CAPABILITY("profile", SystemInfoDeviceCapability::GetProfile)
  DEFAULT_REPORT_STRING_CAPABILITY("nativeApiVersion", "tizen.org/feature/platform.native.api.version")
  DEFAULT_REPORT_STRING_CAPABILITY("duid", "tizen.org/system/tizenid")
  DEFAULT_REPORT_BOOL_CAPABILITY("screenSizeNormal", "tizen.org/feature/screen.size.normal")
  DEFAULT_REPORT_BOOL_CAPABILITY("screenSize480_800", "tizen.org/feature/screen.size.normal.480.800")
  DEFAULT_REPORT_BOOL_CAPABILITY("screenSize720_1280", "tizen.org/feature/screen.size.normal.720.1280")
  DEFAULT_REPORT_BOOL_CAPABILITY("shellAppWidget", "tizen.org/feature/shell.appwidget")
  DEFAULT_REPORT_BOOL_CAPABILITY("nativeOspCompatible", "tizen.org/feature/platform.native.osp_compatible")

  ReportSuccess(result, out);
  LoggerD("Success");
}

void SysteminfoInstance::GetCapability(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "key", out)
  const std::string& key = args.get("key").get<std::string>();
  LoggerD("Getting capability with key: %s ", key.c_str());

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SystemInfoDeviceCapability::GetCapability(key, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
    LoggerD("Success");
  } else {
    ReportError(ret, &out);
  }
}

void SysteminfoInstance::GetPropertyValue(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "property", out)
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& prop_id = args.get("property").get<std::string>();
  LoggerD("Getting property with id: %s ", prop_id.c_str());

  auto get = [this, prop_id, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting");
    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = SysteminfoUtils::GetPropertyValue(prop_id, false, result);
    if (ret.IsError()) {
      ReportError(ret,&(response->get<picojson::object>()));
      return;
    }
    ReportSuccess(result, response->get<picojson::object>());
  };

  auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value{static_cast<double>(callback_id)}));
    LoggerD("message: %s", response->serialize().c_str());
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>
  (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SysteminfoInstance::GetPropertyValueArray(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "property", out)
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& prop_id = args.get("property").get<std::string>();
  LoggerD("Getting property arrray with id: %s ", prop_id.c_str());

  auto get = [this, prop_id, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting");
    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = SysteminfoUtils::GetPropertyValue(prop_id, true, result);
    if (ret.IsError()) {
      LoggerE("Failed: GetPropertyValue()");
      ReportError(ret,&(response->get<picojson::object>()));
      return;
    }
    ReportSuccess(result, response->get<picojson::object>());
  };

  auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>
  (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SysteminfoInstance::AddPropertyValueChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  // Check type of property for which listener should be registered
  CHECK_EXIST(args, "property", out)
  const std::string& property_name = args.get("property").get<std::string>();

  LoggerD("Adding listener for property with id: %s ", property_name.c_str());
  PlatformResult ret(ErrorCode::NO_ERROR);
  if (property_name == kPropertyIdBattery) {
    ret = SysteminfoUtils::RegisterBatteryListener(OnBatteryChangedCallback, *this);
  } else if (property_name == kPropertyIdCpu) {
    ret = SysteminfoUtils::RegisterCpuListener(OnCpuChangedCallback, *this);
  } else if (property_name == kPropertyIdStorage) {
    ret = SysteminfoUtils::RegisterStorageListener(OnStorageChangedCallback, *this);
  } else if (property_name == kPropertyIdDisplay) {
    ret = SysteminfoUtils::RegisterDisplayListener(OnDisplayChangedCallback, *this);
  } else if (property_name == kPropertyIdDeviceOrientation) {
    ret = SysteminfoUtils::RegisterDeviceOrientationListener(OnDeviceOrientationChangedCallback, *this);
  } else if (property_name == kPropertyIdBuild) {
    LoggerW("BUILD property's value is a fixed value");
    //should be accepted, but no registration is needed
    //ret = PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "BUILD property's value is a fixed value");
  } else if (property_name == kPropertyIdLocale) {
    ret = SysteminfoUtils::RegisterLocaleListener(OnLocaleChangedCallback, *this);
  } else if (property_name == kPropertyIdNetwork) {
    ret = SysteminfoUtils::RegisterNetworkListener(OnNetworkChangedCallback, *this);
  } else if (property_name == kPropertyIdWifiNetwork) {
    ret = SysteminfoUtils::RegisterWifiNetworkListener(OnWifiNetworkChangedCallback, *this);
  } else if (property_name == kPropertyIdCellularNetwork) {
    ret = SysteminfoUtils::RegisterCellularNetworkListener(OnCellularNetworkChangedCallback, *this);
  } else if (property_name == kPropertyIdSim) {
    //SIM listeners are not supported by core API, so we just pass over
    LoggerW("SIM listener is not supported by Core API - ignoring");
  } else if (property_name == kPropertyIdPeripheral) {
    ret = SysteminfoUtils::RegisterPeripheralListener(OnPeripheralChangedCallback, *this);
  } else if (property_name == kPropertyIdMemory) {
    ret = SysteminfoUtils::RegisterMemoryListener(OnMemoryChangedCallback, *this);
  } else if (property_name == kPropertyIdCameraFlash) {
    ret = SysteminfoUtils::RegisterCameraFlashListener(OnBrigthnessChangedCallback, *this);
  } else {
    LoggerE("Not supported property");
    ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not supported property");
  }
  if (ret.IsSuccess()) {
    ReportSuccess(out);
    LoggerD("Success");
    return;
  }
  LoggerD("Error");
  ReportError(ret, &out);
}

void SysteminfoInstance::GetTotalMemory(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  long long return_value = 0;
  PlatformResult ret = SysteminfoUtils::GetTotalMemory(return_value);
  if (ret.IsError()) {
    LoggerD("Error");
    ReportError(ret, &out);
    return;
  }
  result_obj.insert(std::make_pair("totalMemory",
                                   picojson::value(static_cast<double>(return_value))));

  ReportSuccess(result, out);
  LoggerD("Success");
}

void SysteminfoInstance::GetAvailableMemory(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  long long return_value = 0;
  PlatformResult ret = SysteminfoUtils::GetAvailableMemory(return_value);
  if (ret.IsError()) {
    LoggerD("Error");
    ReportError(ret, &out);
    return;
  }
  result_obj.insert(std::make_pair("availableMemory",
                                   picojson::value(static_cast<double>(return_value))));

  ReportSuccess(result, out);
  LoggerD("Success");
}

void SysteminfoInstance::GetCount(const picojson::value& args, picojson::object& out) {

  LoggerD("Enter");
  CHECK_EXIST(args, "property", out)
  const std::string& property = args.get("property").get<std::string>();
  LoggerD("Getting count of property with id: %s ", property.c_str());

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  unsigned long count = 0;
  PlatformResult ret = SysteminfoUtils::GetCount(property, count);
  if (ret.IsError()) {
    LoggerE("Failed: GetCount()");
    ReportError(ret, &out);
    return;
  }
  result_obj.insert(std::make_pair("count", picojson::value(static_cast<double>(count))));

  ReportSuccess(result, out);
  LoggerD("Success");
}

void SysteminfoInstance::RemovePropertyValueChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");

  // Check type of property for which listener should be removed
  CHECK_EXIST(args, "property", out)
  const std::string& property_name = args.get("property").get<std::string>();
  LoggerD("Removing listener for property with id: %s ", property_name.c_str());
  PlatformResult ret(ErrorCode::NO_ERROR);
  if (property_name == kPropertyIdBattery) {
    ret = SysteminfoUtils::UnregisterBatteryListener();
  } else if (property_name == kPropertyIdCpu) {
    ret = SysteminfoUtils::UnregisterCpuListener();
  } else if (property_name == kPropertyIdStorage) {
    ret = SysteminfoUtils::UnregisterStorageListener();
  } else if (property_name == kPropertyIdDisplay) {
    ret = SysteminfoUtils::UnregisterDisplayListener();
  } else if (property_name == kPropertyIdDeviceOrientation) {
    ret = SysteminfoUtils::UnregisterDeviceOrientationListener();
  } else if (property_name == kPropertyIdBuild) {
    LoggerW("BUILD property's value is a fixed value");
    //should be accepted, but no unregistration is needed
    //ret = PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "BUILD property's value is a fixed value");
  } else if (property_name == kPropertyIdLocale) {
    ret = SysteminfoUtils::UnregisterLocaleListener();
  } else if (property_name == kPropertyIdNetwork) {
    ret = SysteminfoUtils::UnregisterNetworkListener();
  } else if (property_name == kPropertyIdWifiNetwork) {
    ret = SysteminfoUtils::UnregisterWifiNetworkListener();
  } else if (property_name == kPropertyIdCellularNetwork) {
    ret = SysteminfoUtils::UnregisterCellularNetworkListener();
  } else if (property_name == kPropertyIdSim) {
    //SIM listeners are not supported by core API, so we just pass over
    LoggerW("SIM listener is not supported by Core API - ignoring");
  } else if (property_name == kPropertyIdPeripheral) {
    ret = SysteminfoUtils::UnregisterPeripheralListener();
  } else if (property_name == kPropertyIdMemory) {
    ret = SysteminfoUtils::UnregisterMemoryListener();
  } else if (property_name == kPropertyIdCameraFlash) {
    ret = SysteminfoUtils::UnregisterCameraFlashListener();
  } else {
    LoggerE("Not supported property");
    ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not supported property");
  }
  if (ret.IsSuccess()) {
    ReportSuccess(out);
    LoggerD("Success");
    return;
  }
  LoggerD("Error");
  ReportError(ret, &out);
}

static void ReportSuccess(const picojson::value& result, picojson::object& out) {
  out.insert(std::make_pair("status", picojson::value("success")));
  out.insert(std::make_pair("result",  picojson::value(result)));
}

void SysteminfoInstance::SetBrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");

  CHECK_EXIST(args, "brightness", out)

  const double brightness = args.get("brightness").get<double>();
  int result = DEVICE_ERROR_NONE;
  result = device_flash_set_brightness(brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error occured"), &out);
    return;
  }
  ReportSuccess(out);
}

void SysteminfoInstance::GetBrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");

  int result = DEVICE_ERROR_NONE;
  int brightness = 0;
  result = device_flash_get_brightness(&brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error occured"), &out);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(brightness)), out);
}

void SysteminfoInstance::GetMaxBrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");

  int result = DEVICE_ERROR_NONE;
  int brightness = 0;
  result = device_flash_get_max_brightness(&brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Not supported property"), &out);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(brightness)), out);
}

//Callback functions
void OnBatteryChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdBattery);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdBattery, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnCpuChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdCpu);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdCpu, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnStorageChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdStorage);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnDisplayChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdDisplay);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdDisplay, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnDeviceOrientationChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdDeviceOrientation);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdDeviceOrientation, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnLocaleChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdLocale);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdLocale, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnNetworkChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdNetwork);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdNetwork, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnWifiNetworkChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdWifiNetwork);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdWifiNetwork, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnCellularNetworkChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdCellularNetwork);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdCellularNetwork, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnPeripheralChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdPeripheral);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdPeripheral, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnMemoryChangedCallback(SysteminfoInstance& instance)
{
  LoggerD("Enter");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdMemory);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdMemory, true, result);
  if (ret.IsSuccess()) {
    ReportSuccess(result,response->get<picojson::object>());
    instance.PostMessage(response->serialize().c_str());
  }
}

void OnBrigthnessChangedCallback(SysteminfoInstance &instance)
{
    LoggerD("Enter");
    const std::shared_ptr<picojson::value>& response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    response->get<picojson::object>()[kPropertyIdString] = picojson::value(kPropertyIdCameraFlash);
    response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);

    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdCameraFlash, true, result);
    if (ret.IsSuccess()) {
      ReportSuccess(result,response->get<picojson::object>());
      instance.PostMessage(response->serialize().c_str());
    }
}

} // namespace systeminfo
} // namespace extension
