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

#include "systeminfo/systeminfo_manager.h"

#include <functional>
#include <memory>

#include <device.h>
#include <sensor_internal.h>

#include "systeminfo/systeminfo_instance.h"
#include "systeminfo/systeminfo_device_capability.h"
#include "systeminfo/systeminfo-utils.h"
#include "common/logger.h"
#include "common/converter.h"
#include "common/task-queue.h"

using common::PlatformResult;
using common::ErrorCode;
using common::TypeMismatchException;
using common::tools::ReportError;
using common::tools::ReportSuccess;
using common::Instance;
using common::TaskQueue;

namespace extension {
namespace systeminfo {

namespace {
const int kDefaultPropertyCount = 1;

#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), *out);\
      return;\
    }

#define DEFAULT_REPORT_BOOL_CAPABILITY(str_name, feature_name) \
  ret = SystemInfoDeviceCapability::GetValueBool(feature_name, &bool_value); \
  if (ret.IsError()) { \
    ReportError(ret, out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(bool_value)));

#define REPORT_BOOL_CAPABILITY(str_name, method) \
  ret = method(&bool_value); \
  if (ret.IsError()) { \
    ReportError(ret, out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(bool_value)));

#define DEFAULT_REPORT_INT_CAPABILITY(str_name, feature_name) \
  ret = SystemInfoDeviceCapability::GetValueInt(feature_name, &int_value); \
  if (ret.IsError()) { \
    ReportError(ret, out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(std::to_string(int_value))));

#define DEFAULT_REPORT_STRING_CAPABILITY(str_name, feature_name) \
  ret = SystemInfoDeviceCapability::GetValueString(feature_name, &str_value); \
  if (ret.IsError()) { \
    ReportError(ret, out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(str_value)));

#define REPORT_STRING_CAPABILITY(str_name, method) \
  ret = method(&str_value); \
  if (ret.IsError()) { \
    ReportError(ret, out); \
    return; \
  } \
  result_obj.insert(std::make_pair(str_name, picojson::value(str_value)));

} //namespace

SysteminfoManager::SysteminfoManager(SysteminfoInstance* instance)
    : instance_(instance),
      prop_manager_(*this),
      sensor_handle_(-1),
      wifi_level_(WIFI_RSSI_LEVEL_0),
      cpu_load_(0),
      available_capacity_internal_(0),
      available_capacity_mmc_(0),
      sim_count_(0),
      tapi_handles_{nullptr} {
  LoggerD("Entered");
    int error = wifi_initialize();
    if (WIFI_ERROR_NONE != error) {
      std::string log_msg = "Initialize failed: " + std::string(get_error_message(error));
      LoggerE("%s", log_msg.c_str());
    } else {
      LoggerD("WIFI initialization succeed");
    }

    //TODO
//    error = wifi_set_rssi_level_changed_cb(OnWifiLevelChangedCb, nullptr);
//    if (WIFI_ERROR_NONE != error) {
//      std::string log_msg = "Setting wifi listener failed: " + parseWifiNetworkError(error);
//      LoggerE("%s", log_msg.c_str());
//    } else {
//      LoggerD("Setting wifi listener succeed");
//    }
    InitCameraTypes();
}

SysteminfoManager::~SysteminfoManager() {
  LoggerD("Enter");
  DisconnectSensor(sensor_handle_);


  unsigned int i = 0;
  while(tapi_handles_[i]) {
    tel_deinit(tapi_handles_[i]);
    i++;
  }
//TODO
//  if (nullptr != m_connection_handle) {
//    connection_destroy(m_connection_handle);
//  }

  wifi_deinitialize();
}

void SysteminfoManager::GetCapabilities(const picojson::value& args, picojson::object* out) {
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

  ReportSuccess(result, *out);
  LoggerD("Success");
}

void SysteminfoManager::GetCapability(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "key", out)
  const std::string& key = args.get("key").get<std::string>();
  LoggerD("Getting capability with key: %s ", key.c_str());

  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SystemInfoDeviceCapability::GetCapability(key, &result);
  if (ret.IsSuccess()) {
    ReportSuccess(result, *out);
    LoggerD("Success");
  } else {
    ReportError(ret, out);
  }
}

void SysteminfoManager::GetPropertyValue(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "property", out)
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& prop_id = args.get("property").get<std::string>();
  LoggerD("Getting property with id: %s ", prop_id.c_str());

  auto get = [this, prop_id, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting");
    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = prop_manager_.GetPropertyValue(prop_id, false, &result);
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
    Instance::PostMessage(instance_, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>
  (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SysteminfoManager::GetPropertyValueArray(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "property", out)
  const double callback_id = args.get("callbackId").get<double>();
  const std::string& prop_id = args.get("property").get<std::string>();
  LoggerD("Getting property arrray with id: %s ", prop_id.c_str());

  auto get = [this, prop_id, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting");
    picojson::value result = picojson::value(picojson::object());

    PlatformResult ret = prop_manager_.GetPropertyValue(prop_id, true, &result);
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
    Instance::PostMessage(instance_, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>
  (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SysteminfoManager::GetTotalMemory(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  long long return_value = 0;
  PlatformResult ret = SysteminfoUtils::GetTotalMemory(&return_value);
  if (ret.IsError()) {
    LoggerD("Error");
    ReportError(ret, out);
    return;
  }
  result_obj.insert(std::make_pair("totalMemory",
                                   picojson::value(static_cast<double>(return_value))));

  ReportSuccess(result, *out);
  LoggerD("Success");
}

void SysteminfoManager::GetAvailableMemory(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  long long return_value = 0;
  PlatformResult ret = SysteminfoUtils::GetAvailableMemory(&return_value);
  if (ret.IsError()) {
    LoggerD("Error");
    ReportError(ret, out);
    return;
  }
  result_obj.insert(std::make_pair("availableMemory",
                                   picojson::value(static_cast<double>(return_value))));

  ReportSuccess(result, *out);
  LoggerD("Success");
}

void SysteminfoManager::GetCount(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "property", out)
  const std::string& property = args.get("property").get<std::string>();
  LoggerD("Getting count of property with id: %s ", property.c_str());

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  unsigned long count = 0;
  PlatformResult ret = GetPropertyCount(property, &count);
  if (ret.IsError()) {
    LoggerE("Failed: GetCount()");
    ReportError(ret, out);
    return;
  }
  result_obj.insert(std::make_pair("count", picojson::value(static_cast<double>(count))));

  ReportSuccess(result, *out);
  LoggerD("Success");
}

void SysteminfoManager::AddPropertyValueChangeListener(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");
  // Check type of property for which listener should be registered
  CHECK_EXIST(args, "property", out)
  const std::string& property_name = args.get("property").get<std::string>();

  LoggerD("Adding listener for property with id: %s ", property_name.c_str());
//  PlatformResult ret(ErrorCode::NO_ERROR);
//  if (property_name == kPropertyIdBattery) {
//    ret = SysteminfoUtils::RegisterBatteryListener(OnBatteryChangedCallback, *this);
//  } else if (property_name == kPropertyIdCpu) {
//    ret = SysteminfoUtils::RegisterCpuListener(OnCpuChangedCallback, *this);
//  } else if (property_name == kPropertyIdStorage) {
//    ret = SysteminfoUtils::RegisterStorageListener(OnStorageChangedCallback, *this);
//  } else if (property_name == kPropertyIdDisplay) {
//    ret = SysteminfoUtils::RegisterDisplayListener(OnDisplayChangedCallback, *this);
//  } else if (property_name == kPropertyIdDeviceOrientation) {
//    ret = SysteminfoUtils::RegisterDeviceOrientationListener(OnDeviceOrientationChangedCallback, *this);
//  } else if (property_name == kPropertyIdBuild) {
//    LoggerW("BUILD property's value is a fixed value");
//    //should be accepted, but no registration is needed
//    //ret = PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "BUILD property's value is a fixed value");
//  } else if (property_name == kPropertyIdLocale) {
//    ret = SysteminfoUtils::RegisterLocaleListener(OnLocaleChangedCallback, *this);
//  } else if (property_name == kPropertyIdNetwork) {
//    ret = SysteminfoUtils::RegisterNetworkListener(OnNetworkChangedCallback, *this);
//  } else if (property_name == kPropertyIdWifiNetwork) {
//    ret = SysteminfoUtils::RegisterWifiNetworkListener(OnWifiNetworkChangedCallback, *this);
//  } else if (property_name == kPropertyIdEthernetNetwork) {
//    ret = SysteminfoUtils::RegisterEthernetNetworkListener(OnEthernetNetworkChangedCallback, *this);
//  } else if (property_name == kPropertyIdCellularNetwork) {
//    ret = SysteminfoUtils::RegisterCellularNetworkListener(OnCellularNetworkChangedCallback, *this);
//  } else if (property_name == kPropertyIdSim) {
//    //SIM listeners are not supported by core API, so we just pass over
//    LoggerW("SIM listener is not supported by Core API - ignoring");
//  } else if (property_name == kPropertyIdPeripheral) {
//    ret = SysteminfoUtils::RegisterPeripheralListener(OnPeripheralChangedCallback, *this);
//  } else if (property_name == kPropertyIdMemory) {
//    ret = SysteminfoUtils::RegisterMemoryListener(OnMemoryChangedCallback, *this);
//  } else if (property_name == kPropertyIdCameraFlash) {
//    ret = SysteminfoUtils::RegisterCameraFlashListener(OnBrigthnessChangedCallback, *this);
//  } else {
//    LoggerE("Not supported property");
//    ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not supported property");
//  }
//  if (ret.IsSuccess()) {
//    ReportSuccess(out);
//    LoggerD("Success");
//    return;
//  }
//  LoggerD("Error");
//  ReportError(ret, &out);
}

void SysteminfoManager::RemovePropertyValueChangeListener(const picojson::value& args, picojson::object* out) {
  LoggerD("Enter");

  // Check type of property for which listener should be removed
  CHECK_EXIST(args, "property", out)
  const std::string& property_name = args.get("property").get<std::string>();
  LoggerD("Removing listener for property with id: %s ", property_name.c_str());
  PlatformResult ret(ErrorCode::NO_ERROR);
//  if (property_name == kPropertyIdBattery) {
//    ret = SysteminfoUtils::UnregisterBatteryListener();
//  } else if (property_name == kPropertyIdCpu) {
//    ret = SysteminfoUtils::UnregisterCpuListener();
//  } else if (property_name == kPropertyIdStorage) {
//    ret = SysteminfoUtils::UnregisterStorageListener();
//  } else if (property_name == kPropertyIdDisplay) {
//    ret = SysteminfoUtils::UnregisterDisplayListener();
//  } else if (property_name == kPropertyIdDeviceOrientation) {
//    ret = SysteminfoUtils::UnregisterDeviceOrientationListener();
//  } else if (property_name == kPropertyIdBuild) {
//    LoggerW("BUILD property's value is a fixed value");
//    //should be accepted, but no unregistration is needed
//    //ret = PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "BUILD property's value is a fixed value");
//  } else if (property_name == kPropertyIdLocale) {
//    ret = SysteminfoUtils::UnregisterLocaleListener();
//  } else if (property_name == kPropertyIdNetwork) {
//    ret = SysteminfoUtils::UnregisterNetworkListener();
//  } else if (property_name == kPropertyIdWifiNetwork) {
//    ret = SysteminfoUtils::UnregisterWifiNetworkListener();
//  } else if (property_name == kPropertyIdEthernetNetwork) {
//    ret = SysteminfoUtils::UnregisterEthernetNetworkListener();
//  } else if (property_name == kPropertyIdCellularNetwork) {
//    ret = SysteminfoUtils::UnregisterCellularNetworkListener();
//  } else if (property_name == kPropertyIdSim) {
//    //SIM listeners are not supported by core API, so we just pass over
//    LoggerW("SIM listener is not supported by Core API - ignoring");
//  } else if (property_name == kPropertyIdPeripheral) {
//    ret = SysteminfoUtils::UnregisterPeripheralListener();
//  } else if (property_name == kPropertyIdMemory) {
//    ret = SysteminfoUtils::UnregisterMemoryListener();
//  } else if (property_name == kPropertyIdCameraFlash) {
//    ret = SysteminfoUtils::UnregisterCameraFlashListener();
//  } else {
//    LoggerE("Not supported property");
//    ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not supported property");
//  }
//  if (ret.IsSuccess()) {
//    ReportSuccess(out);
//    LoggerD("Success");
//    return;
//  }
//  LoggerD("Error");
//  ReportError(ret, &out);
}

void SysteminfoManager::SetBrightness(const picojson::value& args, picojson::object* out) {
  LoggerD("entered");

  CHECK_EXIST(args, "brightness", out)

  const double brightness = args.get("brightness").get<double>();
  int result = device_flash_set_brightness(brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error occured"), out);
    return;
  }
  ReportSuccess(*out);
}

void SysteminfoManager::GetBrightness(const picojson::value& args, picojson::object* out) {
  LoggerD("entered");

  int brightness = 0;
  int result = device_flash_get_brightness(&brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error occured"), out);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(brightness)), *out);
}

void SysteminfoManager::GetMaxBrightness(const picojson::value& args, picojson::object* out) {
  LoggerD("entered");

  int brightness = 0;
  int result = device_flash_get_max_brightness(&brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Not supported property"), out);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(brightness)), *out);
}

PlatformResult SysteminfoManager::GetPropertyCount(const std::string& property,
                                                   unsigned long* count)
{
  LoggerD("Enter");

  if ("BATTERY" == property || "CPU" == property || "STORAGE" == property ||
      "DISPLAY" == property || "DEVICE_ORIENTATION" == property ||
      "BUILD" == property || "LOCALE" == property || "NETWORK" == property ||
      "WIFI_NETWORK" == property || "PERIPHERAL" == property ||
      "MEMORY" == property) {
    *count = kDefaultPropertyCount;
  } else if ("CELLULAR_NETWORK" == property) {
    PlatformResult ret = SysteminfoUtils::CheckTelephonySupport();
    if (ret.IsError()) {
      *count = 0;
    } else {
      *count = GetSimCount();
    }
  } else if ("SIM" == property) {
    PlatformResult ret = SysteminfoUtils::CheckTelephonySupport();
    if (ret.IsError()) {
      *count = 0;
    } else {
      *count = GetSimCount();
    }
  } else if ("CAMERA_FLASH" == property) {
    *count = GetCameraTypesCount();
  } else if ("ETHERNET_NETWORK" == property) {
    PlatformResult ret = SysteminfoUtils::CheckIfEthernetNetworkSupported();
    if (ret.IsError()) {
      *count = 0;
    } else {
      *count = kDefaultPropertyCount;
    }
  } else {
    LoggerD("Property with given id is not supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Property with given id is not supported");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

wifi_rssi_level_e SysteminfoManager::GetWifiLevel() {
  LoggerD("Enter");
  return wifi_level_;
}

int SysteminfoManager::GetSensorHandle() {
  LoggerD("Enter");
  if (sensor_handle_ < 0) {
    LoggerD("Connecting to sensor");
    ConnectSensor(&sensor_handle_);
  } else {
    LoggerD("Sensor already connected");
  }
  return sensor_handle_;
}

PlatformResult SysteminfoManager::ConnectSensor(int* result) {
  LoggerD("Entered");
  sensor_t sensor = sensord_get_sensor(AUTO_ROTATION_SENSOR);
  int handle_orientation = sensord_connect(sensor);
  if (handle_orientation < 0) {
    std::string log_msg = "Failed to connect auto rotation sensor";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  bool ret = sensord_start(handle_orientation, 0);
  if(!ret) {
    sensord_disconnect(handle_orientation);
    std::string log_msg = "Failed to start auto rotation sensor";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  LoggerD("Sensor starts successfully = %d", handle_orientation);
  *result = handle_orientation;
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SysteminfoManager::DisconnectSensor(int handle_orientation) {
  LoggerD("Enter");
  if (handle_orientation >= 0) {
    LoggerD("Entered");
    bool state = sensord_stop(handle_orientation);
    LoggerD("sensord_stop() returned state = %d", state);
    state = sensord_disconnect(handle_orientation);
    LoggerD("sensord_disconnect() returned state %d", state);
  } else {
    LoggerD("sensor already disconnected - no action needed");
  }
}

void SysteminfoManager::SetCpuInfoLoad(double load) {
  LoggerD("Enter");
  cpu_load_ = load;
}

void SysteminfoManager::SetAvailableCapacityInternal(unsigned long long capacity) {
  LoggerD("Enter");
  available_capacity_internal_ = capacity;
}

void SysteminfoManager::SetAvailableCapacityMmc(unsigned long long capacity) {
  LoggerD("Enter");
  available_capacity_mmc_ = capacity;
}

int SysteminfoManager::GetSimCount() {
  LoggerD("Entered");
  InitTapiHandles();
  return sim_count_;
}

void SysteminfoManager::InitTapiHandles() {
  LoggerD("Entered");
  if (nullptr == tapi_handles_[0]){  //check if anything is in table
    sim_count_ = 0;
    char **cp_list = tel_get_cp_name_list();
    if (nullptr != cp_list) {
      while (cp_list[sim_count_]) {
        tapi_handles_[sim_count_] = tel_init(cp_list[sim_count_]);
        if (nullptr == tapi_handles_[sim_count_]) {
          LoggerE("Failed to connect with tapi, handle is null");
          break;
        }
        sim_count_++;
        LoggerD("%d modem: %s", sim_count_, cp_list[sim_count_]);
      }
    } else {
      LoggerE("Failed to get cp list");
      sim_count_ = kTapiMaxHandle;
    }
    g_strfreev(cp_list);
  }
}

TapiHandle* SysteminfoManager::GetTapiHandle() {
  LoggerD("Entered");
  InitTapiHandles();
  return tapi_handles_[0];
}

TapiHandle** SysteminfoManager::GetTapiHandles() {
  LoggerD("Enter");
  InitTapiHandles();
  return tapi_handles_;
}

void SysteminfoManager::InitCameraTypes() {
  LoggerD("Enter");
  bool supported = false;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
      "tizen.org/feature/camera.back.flash", &supported);
  if (ret.IsSuccess()) {
    if (supported) {
      camera_types_.push_back("BACK");
    }
  }
  ret = SystemInfoDeviceCapability::GetValueBool(
      "tizen.org/feature/camera.front.flash", &supported);
  if (ret.IsSuccess()) {
    if (supported) {
      camera_types_.push_back("FRONT");
    }
  }
}

std::string SysteminfoManager::GetCameraTypes(int index) {
  LoggerD("Enter");
  if (index >= camera_types_.size()) {
    return "";
  }
  return camera_types_[index];
}

int SysteminfoManager::GetCameraTypesCount() {
  LoggerD("Enter");
  return camera_types_.size();
}

}  // namespace systeminfo
}  // namespace extension
