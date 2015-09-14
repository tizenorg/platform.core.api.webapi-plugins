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
#include <device/callback.h>
#include <device/device-error.h>
#include <sensor_internal.h>
#include <wifi.h>

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
const int BASE_GATHERING_INTERVAL = 100;
const double kPropertyWatcherTime = 1;
// index of default property for sim-related callbacks
const int kDefaultListenerIndex = 0;

const std::string kPropertyIdString = "propertyId";
const std::string kListenerIdString = "listenerId";
const std::string kChangedPropertyIndexString = "changedPropertyIndex";
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
const std::string kPropertyIdEthernetNetwork = "ETHERNET_NETWORK";
const std::string kPropertyIdCellularNetwork = "CELLULAR_NETWORK";
const std::string kPropertyIdSim = "SIM";
const std::string kPropertyIdPeripheral = "PERIPHERAL";
const std::string kPropertyIdMemory= "MEMORY";
const std::string kPropertyIdCameraFlash= "CAMERA_FLASH";

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

//Callback static functions
static void OnBatteryChangedCb(keynode_t* node, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdBattery);
}

static gboolean OnCpuChangedCb(gpointer event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallCpuListenerCallback();
  return G_SOURCE_CONTINUE;
}

static gboolean OnStorageChangedCb(gpointer event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallStorageListenerCallback();
  return G_SOURCE_CONTINUE;
}

static void OnMmcChangedCb(keynode_t* node, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdStorage);
}

static void OnDisplayChangedCb(keynode_t* node, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdDisplay);
}

static void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdDeviceOrientation);
}

static void OnDeviceOrientationChangedCb(sensor_t sensor, unsigned int event_type,
                                  sensor_data_t *data, void *user_data) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(user_data);
  manager->CallListenerCallback(kPropertyIdDeviceOrientation);
}

static void OnLocaleChangedCb(system_settings_key_e key, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdLocale);
}

static void OnNetworkChangedCb(connection_type_e type, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdNetwork);
}

static void OnNetworkValueChangedCb(const char* ipv4_address,
                             const char* ipv6_address, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdWifiNetwork);
  manager->CallListenerCallback(kPropertyIdEthernetNetwork);
  manager->CallListenerCallback(kPropertyIdCellularNetwork);
}

static void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdCellularNetwork, kDefaultListenerIndex);
}

static void OnTapiValueChangedCb(TapiHandle *handle, const char *noti_id,
                                 void *data, void *user_data) {
  LoggerD("Enter");
  LoggerD("Changed key: %s", noti_id);
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(user_data);
  int index = manager->GetChangedTapiIndex(handle);
  LoggerD("Changed SIM index is: %d", index);
  manager->CallListenerCallback(kPropertyIdCellularNetwork, index);
}

static void OnPeripheralChangedCb(keynode_t* node, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdPeripheral);
}

static void OnMemoryChangedCb(keynode_t* node, void* event_ptr) {
  LoggerD("Enter");
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(event_ptr);
  manager->CallListenerCallback(kPropertyIdMemory);
}

static void OnBrightnessChangedCb(device_callback_e type, void* value, void* user_data) {
  LoggerD("Entered");
  if (type == DEVICE_CALLBACK_FLASH_BRIGHTNESS) {
    SysteminfoManager* manager = static_cast<SysteminfoManager*>(user_data);
    manager->CallListenerCallback(kPropertyIdCameraFlash);
  }
}

static void OnWifiLevelChangedCb (wifi_rssi_level_e rssi_level, void *user_data) {
  LoggerD("Entered");
  LoggerD("Level %d", rssi_level);
  SysteminfoManager* manager = static_cast<SysteminfoManager*>(user_data);
  manager->SetWifiLevel(rssi_level);
}

} //namespace

SysteminfoManager::SysteminfoManager(SysteminfoInstance* instance)
    : instance_(instance),
      prop_manager_(*this),
      sensor_handle_(-1),
      wifi_level_(WIFI_RSSI_LEVEL_0),
      cpu_load_(0),
      last_cpu_load_(0),
      available_capacity_internal_(0),
      last_available_capacity_internal_(0),
      available_capacity_mmc_(0),
      last_available_capacity_mmc_(0),
      sim_count_(0),
      tapi_handles_{nullptr},
      cpu_event_id_(0),
      storage_event_id_(0),
      connection_handle_(nullptr) {
        LoggerD("Entered");
        int error = wifi_initialize();
        if (WIFI_ERROR_NONE != error) {
          std::string log_msg = "Initialize failed: " + std::string(get_error_message(error));
          LoggerE("%s", log_msg.c_str());
        } else {
          LoggerD("WIFI initialization succeed");
        }

        error = wifi_set_rssi_level_changed_cb(OnWifiLevelChangedCb, this);
        if (WIFI_ERROR_NONE != error) {
          std::string log_msg = "Setting wifi listener failed: " +
              std::string(get_error_message(error));
          LoggerE("%s", log_msg.c_str());
        } else {
          LoggerD("Setting wifi listener succeed");
        }
        InitCameraTypes();
      }

SysteminfoManager::~SysteminfoManager() {
  LoggerD("Enter");
  DisconnectSensor(sensor_handle_);

  if (IsListenerRegistered(kPropertyIdBattery)) { UnregisterBatteryListener(); }
  if (IsListenerRegistered(kPropertyIdCpu)) { UnregisterCpuListener(); }
  if (IsListenerRegistered(kPropertyIdStorage)) { UnregisterStorageListener(); }
  if (IsListenerRegistered(kPropertyIdDisplay)) { UnregisterDisplayListener(); }
  if (IsListenerRegistered(kPropertyIdDeviceOrientation)) { UnregisterDeviceOrientationListener(); }
  if (IsListenerRegistered(kPropertyIdLocale)) { UnregisterLocaleListener(); }
  if (IsListenerRegistered(kPropertyIdNetwork)) { UnregisterNetworkListener(); }
  if (IsListenerRegistered(kPropertyIdWifiNetwork)) { UnregisterWifiNetworkListener(); }
  if (IsListenerRegistered(kPropertyIdEthernetNetwork)) { UnregisterEthernetNetworkListener(); }
  if (IsListenerRegistered(kPropertyIdCellularNetwork)) { UnregisterCellularNetworkListener(); }
  if (IsListenerRegistered(kPropertyIdPeripheral)) { UnregisterPeripheralListener(); }
  if (IsListenerRegistered(kPropertyIdMemory)) { UnregisterMemoryListener(); }
  if (IsListenerRegistered(kPropertyIdCameraFlash)) { UnregisterCameraFlashListener(); }

  unsigned int i = 0;
  while(tapi_handles_[i]) {
    tel_deinit(tapi_handles_[i]);
    i++;
  }

  if (nullptr != connection_handle_) {
    connection_destroy(connection_handle_);
  }

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

void SysteminfoManager::AddPropertyValueChangeListener(const picojson::value& args,
                                                       picojson::object* out) {
  LoggerD("Enter");
  // Check type of property for which listener should be registered
  CHECK_EXIST(args, "property", out)
  const std::string& property_name = args.get("property").get<std::string>();

  PlatformResult ret(ErrorCode::NO_ERROR);
  if (!IsListenerRegistered(property_name)){
    LoggerD("Adding listener for property with id: %s ", property_name.c_str());
    if (property_name == kPropertyIdBattery) {
      ret = RegisterBatteryListener();
    } else if (property_name == kPropertyIdCpu) {
      ret = RegisterCpuListener();
    } else if (property_name == kPropertyIdStorage) {
      ret = RegisterStorageListener();
    } else if (property_name == kPropertyIdDisplay) {
      ret = RegisterDisplayListener();
    } else if (property_name == kPropertyIdDeviceOrientation) {
      ret = RegisterDeviceOrientationListener();
    } else if (property_name == kPropertyIdBuild) {
      LoggerW("BUILD property's value is a fixed value");
      //should be accepted, but no registration is needed
      ret = PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "BUILD property's value is a fixed value");
    } else if (property_name == kPropertyIdLocale) {
      ret = RegisterLocaleListener();
    } else if (property_name == kPropertyIdNetwork) {
      ret = RegisterNetworkListener();
    } else if (property_name == kPropertyIdWifiNetwork) {
      ret = RegisterWifiNetworkListener();
    } else if (property_name == kPropertyIdEthernetNetwork) {
      ret = RegisterEthernetNetworkListener();
    } else if (property_name == kPropertyIdCellularNetwork) {
      ret = RegisterCellularNetworkListener();
    } else if (property_name == kPropertyIdSim) {
      //SIM listeners are not supported by core API, so we just pass over
      LoggerW("SIM listener is not supported by Core API - ignoring");
    } else if (property_name == kPropertyIdPeripheral) {
      ret = RegisterPeripheralListener();
    } else if (property_name == kPropertyIdMemory) {
      ret = RegisterMemoryListener();
    } else if (property_name == kPropertyIdCameraFlash) {
      ret = RegisterCameraFlashListener();
    } else {
      LoggerE("Not supported property");
      ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not supported property");
    }
    if (ret.IsSuccess()) {
      registered_listeners_.insert(property_name);
    }
  } else {
    LoggerD("Adding listener for property with id: %s is not needed, already registered",
            property_name.c_str());
  }
  if (ret.IsSuccess()) {
    ReportSuccess(*out);
    LoggerD("Success");
    return;
  }
  LoggerD("Error");
  ReportError(ret, out);
}

void SysteminfoManager::RemovePropertyValueChangeListener(const picojson::value& args,
                                                          picojson::object* out) {
  LoggerD("Enter");

  // Check type of property for which listener should be removed
  CHECK_EXIST(args, "property", out)
  const std::string& property_name = args.get("property").get<std::string>();

  PlatformResult ret(ErrorCode::NO_ERROR);
  if (IsListenerRegistered(property_name)){
    LoggerD("Removing listener for property with id: %s ", property_name.c_str());
    registered_listeners_.erase(property_name);
    if (property_name == kPropertyIdBattery) {
      ret = UnregisterBatteryListener();
    } else if (property_name == kPropertyIdCpu) {
      ret = UnregisterCpuListener();
    } else if (property_name == kPropertyIdStorage) {
      ret = UnregisterStorageListener();
    } else if (property_name == kPropertyIdDisplay) {
      ret = UnregisterDisplayListener();
    } else if (property_name == kPropertyIdDeviceOrientation) {
      ret = UnregisterDeviceOrientationListener();
    } else if (property_name == kPropertyIdBuild) {
      LoggerW("BUILD property's value is a fixed value");
      //should be accepted, but no unregistration is needed
      //ret = PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "BUILD property's value is a fixed value");
    } else if (property_name == kPropertyIdLocale) {
      ret = UnregisterLocaleListener();
    } else if (property_name == kPropertyIdNetwork) {
      ret = UnregisterNetworkListener();
    } else if (property_name == kPropertyIdWifiNetwork) {
      ret = UnregisterWifiNetworkListener();
    } else if (property_name == kPropertyIdEthernetNetwork) {
      ret = UnregisterEthernetNetworkListener();
    } else if (property_name == kPropertyIdCellularNetwork) {
      ret = UnregisterCellularNetworkListener();
    } else if (property_name == kPropertyIdSim) {
      //SIM listeners are not supported by core API, so we just pass over
      LoggerW("SIM listener is not supported by Core API - ignoring");
    } else if (property_name == kPropertyIdPeripheral) {
      ret = UnregisterPeripheralListener();
    } else if (property_name == kPropertyIdMemory) {
      ret = UnregisterMemoryListener();
    } else if (property_name == kPropertyIdCameraFlash) {
      ret = UnregisterCameraFlashListener();
    } else {
      LoggerE("Not supported property");
      ret = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not supported property");
    }
  } else {
    LoggerD("Removing listener for property with id: %s is not needed, not registered",
            property_name.c_str());
  }
  if (ret.IsSuccess()) {
    ReportSuccess(*out);
    LoggerD("Success");
    return;
  }
  LoggerD("Error");
  ReportError(ret, out);
}

#define CHECK_LISTENER_ERROR(method) \
  ret = method; \
  if (ret.IsError()) { \
    return ret; \
  }

bool SysteminfoManager::IsIpChangeCallbackNotRegistered() {
  LoggerD("Entered");
  return !(IsListenerRegistered(kPropertyIdWifiNetwork) ||
      IsListenerRegistered(kPropertyIdEthernetNetwork) ||
      IsListenerRegistered(kPropertyIdCellularNetwork));
}

PlatformResult SysteminfoManager::RegisterIpChangeCallback() {
  LoggerD("Entered");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  int error = connection_set_ip_address_changed_cb(handle,
                                                   OnNetworkValueChangedCb,
                                                   static_cast<void*>(this));
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to register ip change callback: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot register ip change callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterIpChangeCallback() {
  LoggerD("Entered");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  int error = connection_unset_ip_address_changed_cb(handle);
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to unregister ip change callback: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot unregister ip change callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterBatteryListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY,
                                             OnBatteryChangedCb, this))
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
                                             OnBatteryChangedCb, this))
                                             LoggerD("Added callback for BATTERY");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterBatteryListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY,
                                               OnBatteryChangedCb))
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW,
                                               OnBatteryChangedCb))
                                               LoggerD("Removed callback for BATTERY");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterCpuListener() {
  LoggerD("Entered");
  cpu_event_id_ = g_timeout_add_seconds(kPropertyWatcherTime, OnCpuChangedCb, static_cast<void*>(this));
  LoggerD("Added callback for CPU");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterCpuListener() {
  LoggerD("Entered");
  g_source_remove(cpu_event_id_);
  cpu_event_id_ = 0;
  LoggerD("Removed callback for CPU");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterStorageListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS,
                                             OnMmcChangedCb, this))

  storage_event_id_ = g_timeout_add_seconds(kPropertyWatcherTime,
                                             OnStorageChangedCb, static_cast<void*>(this));
  LoggerD("Added callback for STORAGE");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterStorageListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb))

  g_source_remove(storage_event_id_);
  storage_event_id_ = 0;
  LoggerD("Removed callback for STORAGE");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterDisplayListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS,
                                             OnDisplayChangedCb, this))
  LoggerD("Added callback for DISPLAY");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterDisplayListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS,
                                               OnDisplayChangedCb))
  LoggerD("Removed callback for DISPLAY");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterDeviceOrientationListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL,
                                             OnDeviceAutoRotationChangedCb, this))

  bool sensor_ret = sensord_register_event(GetSensorHandle(), AUTO_ROTATION_EVENT_CHANGE_STATE,
                                           BASE_GATHERING_INTERVAL, 0,
                                           OnDeviceOrientationChangedCb, this);
  if (!sensor_ret) {
    LoggerE("Failed to register orientation change event listener");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to register orientation change event listener");
  }

  LoggerD("Added callback for DEVICE_ORIENTATION");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterDeviceOrientationListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(
      SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL,
                                               OnDeviceAutoRotationChangedCb))
  bool sensor_ret = sensord_unregister_event(GetSensorHandle(), AUTO_ROTATION_EVENT_CHANGE_STATE);
  if (!sensor_ret) {
    LoggerE("Failed to unregister orientation change event listener");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister"
                          " orientation change event listener");
  }

  LoggerD("Removed callback for DEVICE_ORIENTATION");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterLocaleListener() {
  LoggerD("Entered");
  if (SYSTEM_SETTINGS_ERROR_NONE !=
      system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY,
                                     OnLocaleChangedCb, static_cast<void*>(this)) ) {
    LoggerE("Country change callback registration failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Country change callback registration failed");
  }
  if (SYSTEM_SETTINGS_ERROR_NONE !=
      system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                     OnLocaleChangedCb, static_cast<void*>(this)) ) {
    LoggerE("Language change callback registration failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Language change callback registration failed");
  }
  LoggerD("Added callback for LOCALE");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterLocaleListener() {
  LoggerD("Entered");
  if (SYSTEM_SETTINGS_ERROR_NONE !=
      system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE) ) {
    LoggerE("Unregistration of language change callback failed");
  }
  if (SYSTEM_SETTINGS_ERROR_NONE !=
      system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY) ) {
    LoggerE("Unregistration of country change callback failed");
  }
  LoggerD("Removed callback for LOCALE");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterNetworkListener() {
  LoggerD("Entered");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  if (CONNECTION_ERROR_NONE !=
      connection_set_type_changed_cb(handle, OnNetworkChangedCb, static_cast<void*>(this))) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Registration of listener failed");
  }
  LoggerD("Added callback for NETWORK");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterNetworkListener() {
  LoggerD("Entered");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  if (CONNECTION_ERROR_NONE != connection_unset_type_changed_cb(handle)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unregistration of listener failed");
  }
  LoggerD("Removed callback for NETWORK");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterWifiNetworkListener() {
  LoggerD("Entered");

  if (IsIpChangeCallbackNotRegistered()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(RegisterIpChangeCallback());
    LoggerD("Registered IP change listener");
  } else {
    LoggerD("No need to register IP listener on platform, already registered");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterWifiNetworkListener() {
  LoggerD("Entered");
  // if there is no other ip-relateded listeners left, unregister
  if (IsIpChangeCallbackNotRegistered()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback());
    LoggerD("Removed IP change listener");
  } else {
    LoggerD("Removed callback for WIFI_NETWORK, but IP change listener still works");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterEthernetNetworkListener() {
  LoggerD("Entered");
  PlatformResult ret = SysteminfoUtils::CheckIfEthernetNetworkSupported();
  if (ret.IsError()){
    return ret;
  }

  if (IsIpChangeCallbackNotRegistered()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(RegisterIpChangeCallback());
    LoggerD("Registered IP change listener");
  } else {
    LoggerD("No need to register IP listener on platform, already registered");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterEthernetNetworkListener() {
  LoggerD("Entered");

  if (IsIpChangeCallbackNotRegistered()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback());
    LoggerD("Removed IP change listener");
  } else {
    LoggerD("Removed callback for ETHERNET_NETWORK, but IP change listener still works");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterCellularNetworkListener() {
  LoggerD("Entered");
  PlatformResult ret = SysteminfoUtils::CheckTelephonySupport();
  if (ret.IsError()) {
      return ret;
  }

  if (IsIpChangeCallbackNotRegistered()) {
    CHECK_LISTENER_ERROR(RegisterIpChangeCallback());
    LoggerD("Registered IP change listener");
  } else {
    LoggerD("No need to register IP listener on platform, already registered");
  }

  if (!IsListenerRegistered(kPropertyIdCellularNetwork)) {
    CHECK_LISTENER_ERROR(SysteminfoUtils::RegisterVconfCallback(
        VCONFKEY_TELEPHONY_FLIGHT_MODE, OnCellularNetworkValueChangedCb, this))
    int sim_count = GetSimCount();
    TapiHandle **tapis = GetTapiHandles();
    for (int i = 0; i < sim_count; ++i) {
      CHECK_LISTENER_ERROR(SysteminfoUtils::RegisterTapiChangeCallback(
          tapis[i], TAPI_PROP_NETWORK_CELLID, OnTapiValueChangedCb, this))
      CHECK_LISTENER_ERROR(SysteminfoUtils::RegisterTapiChangeCallback(
          tapis[i], TAPI_PROP_NETWORK_LAC, OnTapiValueChangedCb, this))
      CHECK_LISTENER_ERROR(SysteminfoUtils::RegisterTapiChangeCallback(
          tapis[i], TAPI_PROP_NETWORK_ROAMING_STATUS, OnTapiValueChangedCb, this))
    }
    LoggerD("Added callback for CELLULAR_NETWORK");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterCellularNetworkListener() {
  LoggerD("Entered");

  // if there is no other ip-relateded listeners left, unregister
  if (!IsListenerRegistered(kPropertyIdCellularNetwork)) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterVconfCallback(
        VCONFKEY_TELEPHONY_FLIGHT_MODE, OnCellularNetworkValueChangedCb))
    int sim_count = GetSimCount();
    TapiHandle **tapis = GetTapiHandles();
    for (int i = 0; i < sim_count; ++i) {
      CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterTapiChangeCallback(
          tapis[i], TAPI_PROP_NETWORK_CELLID))
      CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterTapiChangeCallback(
          tapis[i], TAPI_PROP_NETWORK_LAC))
      CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterTapiChangeCallback(
          tapis[i], TAPI_PROP_NETWORK_ROAMING_STATUS))
    }
  }

  if (IsIpChangeCallbackNotRegistered()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback());
    LoggerD("Removed IP change listener");
  } else {
    LoggerD("Removed callback for CELLULAR_NETWORK, but IP change listener still works");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterPeripheralListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  int value = 0;
/*  if (-1 != vconf_get_int(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &value)) {
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS,
                                               OnPeripheralChangedCb, instance))
  }*/
  if (-1 != vconf_get_int(VCONFKEY_SYSMAN_HDMI, &value)) {
    CHECK_LISTENER_ERROR(SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SYSMAN_HDMI,
                                               OnPeripheralChangedCb, this))
  }

  LoggerD("Added callback for PERIPHERAL");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterPeripheralListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  int value = 0;
/*  if (-1 != vconf_get_int(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &value)) {
    CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS,
                                                 OnPeripheralChangedCb))
  }*/
  if (-1 != vconf_get_int(VCONFKEY_SYSMAN_HDMI, &value)) {
    CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SYSMAN_HDMI,
                                                 OnPeripheralChangedCb))
  }

  LoggerD("Removed callback for PERIPHERAL");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterMemoryListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  int value = 0;
  if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
    CHECK_LISTENER_ERROR(SysteminfoUtils::RegisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY,
                                                                OnMemoryChangedCb, this))
  }
  LoggerD("Added callback for MEMORY");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterMemoryListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  int value = 0;
  if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
    CHECK_LISTENER_ERROR(SysteminfoUtils::UnregisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY,
                                                                  OnMemoryChangedCb))
  }
  LoggerD("Removed callback for MEMORY");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::RegisterCameraFlashListener() {
  LoggerD("Entered");
  if (DEVICE_ERROR_NONE != device_add_callback(DEVICE_CALLBACK_FLASH_BRIGHTNESS,
                                               OnBrightnessChangedCb, this)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR);
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoManager::UnregisterCameraFlashListener() {
  LoggerD("Entered");
  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  if (DEVICE_ERROR_NONE != device_remove_callback(DEVICE_CALLBACK_FLASH_BRIGHTNESS,
                                                  OnBrightnessChangedCb)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR);
  }
  LoggerD("Removed callback for camera_flash");
  return PlatformResult(ErrorCode::NO_ERROR);
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
                                                   unsigned long* count) {
  LoggerD("Enter");

  if ("BATTERY" == property || "CPU" == property || "STORAGE" == property ||
      "DISPLAY" == property || "DEVICE_ORIENTATION" == property ||
      "BUILD" == property || "LOCALE" == property || "WIFI_NETWORK" == property ||
      "PERIPHERAL" == property || "MEMORY" == property) {
    *count = kDefaultPropertyCount;
  } else if ("CELLULAR_NETWORK" == property || "SIM" == property || "NETWORK" == property) {
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

void SysteminfoManager::SetWifiLevel(wifi_rssi_level_e level) {
  LoggerD("Entered");
  wifi_level_ = level;
}

int SysteminfoManager::GetSensorHandle() {
  LoggerD("Enter");
  std::lock_guard<std::mutex> lock(sensor_mutex_);
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
  LoggerD("Entered");
  available_capacity_internal_ = capacity;
}

void SysteminfoManager::SetAvailableCapacityMmc(unsigned long long capacity) {
  LoggerD("Entered");
  available_capacity_mmc_ = capacity;
}

int SysteminfoManager::GetSimCount() {
  LoggerD("Entered");
  InitTapiHandles();
  return sim_count_;
}

void SysteminfoManager::InitTapiHandles() {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(tapi_mutex_);
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
        LoggerD("%d modem: %s", sim_count_, cp_list[sim_count_]);
        sim_count_++;
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

int SysteminfoManager::GetChangedTapiIndex(TapiHandle* tapi) {
  LoggerD("Enter");
  TapiHandle** handles = GetTapiHandles();
  for (int i = 0; i < sim_count_; ++i) {
    if (handles[i] == tapi) {
      return i;
    }
  }
  return -1;
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

PlatformResult SysteminfoManager::GetConnectionHandle(connection_h& handle) {
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(connection_mutex_);
  if (nullptr == connection_handle_) {
    int error = connection_create(&connection_handle_);
    if (CONNECTION_ERROR_NONE != error) {
      LoggerE("Failed to create connection: %d", error);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot create connection");
    }
  }
  handle = connection_handle_;
  return PlatformResult(ErrorCode::NO_ERROR);
}

bool SysteminfoManager::IsListenerRegistered(const std::string& property_id) {
  LoggerD("Entered");
  return (registered_listeners_.find(property_id) != registered_listeners_.end());
}

void SysteminfoManager::PostListenerResponse(const std::string& property_id,
                                     const picojson::value& result,
                                     int property_index) {
  LoggerD("Entered");
  const std::shared_ptr<picojson::value>& response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
  response->get<picojson::object>()[kPropertyIdString] = picojson::value(property_id);
  response->get<picojson::object>()[kListenerIdString] = picojson::value(kListenerConstValue);
  // Added pushing index of property to handle only default for signle-property listeners in js
  response->get<picojson::object>()[kChangedPropertyIndexString] =
      picojson::value(static_cast<double>(property_index));

  ReportSuccess(result,response->get<picojson::object>());
  Instance::PostMessage(instance_, response->serialize().c_str());
}


void SysteminfoManager::CallListenerCallback(const std::string& property_id,
                                             int property_index) {
  LoggerD("Enter");
  if(IsListenerRegistered(property_id)) {
    LoggerD("listener for %s property is registered, calling it", property_id.c_str());

    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = GetPropertiesManager().GetPropertyValue(
        property_id, true, &result);
    if (ret.IsSuccess()) {
      PostListenerResponse(property_id, result, property_index);
    }
  } else {
    LoggerD("listener for %s property is not registered, ignoring", property_id.c_str());
  }
}

void SysteminfoManager::CallCpuListenerCallback() {
  LoggerD("Enter");
  std::string property_id = kPropertyIdCpu;
  if(IsListenerRegistered(property_id)) {
    LoggerD("listener for %s property is registered, calling it", property_id.c_str());
    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = GetPropertiesManager().GetPropertyValue(
        property_id, true, &result);
    if (ret.IsSuccess()) {
      if (cpu_load_ == last_cpu_load_) {
        LoggerD("Cpu load didn't change, ignoring");
        return;
      }
      last_cpu_load_ = cpu_load_;
      PostListenerResponse(property_id, result);
    }
  } else {
    LoggerD("listener for %s property is not registered, ignoring", property_id.c_str());
  }
}

void SysteminfoManager::CallStorageListenerCallback() {
  LoggerD("Enter");
  std::string property_id = kPropertyIdStorage;
  if(IsListenerRegistered(property_id)) {
    LoggerD("listener for %s property is registered, calling it", property_id.c_str());

    picojson::value result = picojson::value(picojson::object());
    PlatformResult ret = GetPropertiesManager().GetPropertyValue(
        property_id, true, &result);
    if (ret.IsSuccess()) {
      // check if anything changed
      if (available_capacity_internal_ == last_available_capacity_internal_ &&
          available_capacity_mmc_ == last_available_capacity_mmc_) {
        LoggerD("Storage state didn't change, ignoring");
        return;
      }
      // refresh previous values
      last_available_capacity_internal_ = available_capacity_internal_;
      last_available_capacity_mmc_ = available_capacity_mmc_;

      PostListenerResponse(property_id, result);
    }
  } else {
    LoggerD("listener for %s property is not registered, ignoring", property_id.c_str());
  }
}

}  // namespace systeminfo
}  // namespace extension
