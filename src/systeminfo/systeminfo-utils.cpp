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

#include "systeminfo-utils.h"
#include "systeminfo/systeminfo_instance.h"

#include <fstream>
#include <sstream>
#include <memory>
#include <mutex>

#include <system_settings.h>
#include <system_info.h>
#include <sys/statfs.h>

#include <vconf.h>
#include <vconf-internal-keys.h>
#include <net_connection.h>
#include <tapi_common.h>
#include <ITapiModem.h>
#include <ITapiSim.h>
#include <device.h>
#include <device/callback.h>
#include <device/device-error.h>
#include <sensor_internal.h>
#include <wifi.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/dbus_operation.h"
#include "common/scope_exit.h"

// TODO:: hardcoded value, only for IsBluetoothAlwaysOn
#define PROFILE_MOBILE 1

namespace extension {
namespace systeminfo {

namespace {

const std::string MEMORY_STATE_NORMAL = "NORMAL";
const std::string MEMORY_STATE_WARNING = "WARNING";
const int MEMORY_TO_BYTE = 1024;
const int BASE_GATHERING_INTERVAL = 100;
const double DISPLAY_INCH_TO_MILLIMETER = 2.54;

const int kDefaultPropertyCount = 1;

}  // namespace

using namespace common;

//Callback functions declarations
static void OnBatteryChangedCb(keynode_t* node, void* event_ptr);
static gboolean OnCpuChangedCb(gpointer event_ptr);
static gboolean OnStorageChangedCb(gpointer event_ptr);
static void OnMmcChangedCb(keynode_t* node, void* event_ptr);
static void OnDisplayChangedCb(keynode_t* node, void* event_ptr);
static void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr);
static void OnDeviceOrientationChangedCb(sensor_t sensor, unsigned int event_type,
                                         sensor_data_t *data, void *user_data);
static void OnLocaleChangedCb(system_settings_key_e key, void* event_ptr);
static void OnNetworkChangedCb(connection_type_e type, void* event_ptr);
static void OnNetworkValueChangedCb(const char* ipv4_address,
                                    const char* ipv6_address, void* event_ptr);
static void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr);
static void OnPeripheralChangedCb(keynode_t* node, void* event_ptr);
static void OnMemoryChangedCb(keynode_t* node, void* event_ptr);
static void OnBrightnessChangedCb(device_callback_e type, void *value, void *user_data);
static void OnWifiLevelChangedCb (wifi_rssi_level_e rssi_level, void *user_data);

static void SimCphsValueCallback(TapiHandle *handle, int result, void *data, void *user_data);
static void SimMsisdnValueCallback(TapiHandle *handle, int result, void *data, void *user_data);
static void SimSpnValueCallback(TapiHandle *handle, int result, void *data, void *user_data);

namespace {
const std::string kPropertyIdCpu = "CPU";
//Battery
const double kRemainingBatteryChargeMax = 100.0;
const int kVconfErrorNone = 0;
//Display
const double kDisplayBrightnessDivideValue = 100;
//Device Orientation
const std::string kOrientationPortraitPrimary = "PORTRAIT_PRIMARY";
const std::string kOrientationPortraitSecondary = "PORTRAIT_SECONDARY";
const std::string kOrientationLandscapePrimary = "LANDSCAPE_PRIMARY";
const std::string kOrientationLandscapeSecondary = "LANDSCAPE_SECONDARY";
//Peripheral
const std::string kVideoOutputString = "isVideoOutputOn";
//Storage
const char* kStorageInternalPath = "/opt/usr/media";
const char* kStorageSdcardPath = "/opt/storage/sdcard";
const std::string kPropertyIdStorage = "STORAGE";
const std::string kTypeUnknown = "UNKNOWN";
const std::string kTypeInternal = "INTERNAL";
const std::string kTypeUsbHost = "USB_HOST";
const std::string kTypeMmc = "MMC";
const double kPropertyWatcherTime = 1;
//Network
enum NetworkType {
  kNone,
  kType2G,
  kType2_5G,
  kType3G,
  kType4G,
  kWifi,
  kEthernet,
  kUnknown
};

const char* kNetworkTypeNone = "NONE";
const char* kNetworkType2G = "2G";
const char* kNetworkType2_5G = "2.5G";
const char* kNetworkType3G = "3G";
const char* kNetworkType4G = "4G";
const char* kNetworkTypeWifi = "WIFI";
const char* kNetworkTypeEthernet = "ETHERNET";
const char* kNetworkTypeUnknown = "UNKNOWN";
//Wifi Network
const std::string kWifiStatusOn = "ON";
const std::string kWifiStatusOff = "OFF";
const int kWifiSignalStrengthDivideValue = 100;
//Cellular Network
const unsigned short kMccDivider = 100;
const char* kConnectionOff = "OFF";
const char* kConnectionOn = "ON";

static std::string parseWifiNetworkError(int error) {
  switch (error) {
    case WIFI_ERROR_NONE : return "WIFI_ERROR_NONE";
    case WIFI_ERROR_INVALID_PARAMETER : return "WIFI_ERROR_INVALID_PARAMETER";
    case WIFI_ERROR_OUT_OF_MEMORY : return "WIFI_ERROR_OUT_OF_MEMORY";
    case WIFI_ERROR_INVALID_OPERATION : return "WIFI_ERROR_INVALID_OPERATION";
    case WIFI_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED : return "WIFI_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED";
    case WIFI_ERROR_OPERATION_FAILED : return "WIFI_ERROR_OPERATION_FAILED";
    case WIFI_ERROR_NO_CONNECTION : return "WIFI_ERROR_NO_CONNECTION";
    case WIFI_ERROR_NOW_IN_PROGRESS : return "WIFI_ERROR_NOW_IN_PROGRESS";
    case WIFI_ERROR_ALREADY_EXISTS : return "WIFI_ERROR_ALREADY_EXISTS";
    case WIFI_ERROR_OPERATION_ABORTED : return "WIFI_ERROR_OPERATION_ABORTED";
    case WIFI_ERROR_DHCP_FAILED : return "WIFI_ERROR_DHCP_FAILED";
    case WIFI_ERROR_INVALID_KEY : return "WIFI_ERROR_INVALID_KEY";
    case WIFI_ERROR_NO_REPLY : return "WIFI_ERROR_NO_REPLY";
    case WIFI_ERROR_SECURITY_RESTRICTED : return "WIFI_ERROR_SECURITY_RESTRICTED";
    case WIFI_ERROR_PERMISSION_DENIED : return "WIFI_ERROR_PERMISSION_DENIED";
    case WIFI_ERROR_NOT_SUPPORTED : return "WIFI_ERROR_NOT_SUPPORTED";
    default : return "Unknown Wi-Fi error";
  }
}

}

/////////////////////////// SystemInfoListeners ////////////////////////////////

class SystemInfoListeners {
 public:
  SystemInfoListeners();
  ~SystemInfoListeners();

  PlatformResult RegisterBatteryListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterBatteryListener();
  PlatformResult RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                     SysteminfoInstance& instance);
  PlatformResult UnregisterCpuListener();
  PlatformResult RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterStorageListener();
  PlatformResult RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterDisplayListener();
  PlatformResult RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                   SysteminfoInstance& instance);
  PlatformResult UnregisterDeviceOrientationListener();
  PlatformResult RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                        SysteminfoInstance& instance);
  PlatformResult UnregisterLocaleListener();
  PlatformResult RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterNetworkListener();
  PlatformResult RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                             SysteminfoInstance& instance);
  PlatformResult UnregisterWifiNetworkListener();
  PlatformResult RegisterEthernetNetworkListener(const SysteminfoUtilsCallback& callback,
                                                 SysteminfoInstance& instance);
  PlatformResult UnregisterEthernetNetworkListener();
  PlatformResult RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                 SysteminfoInstance& instance);
  PlatformResult UnregisterCellularNetworkListener();
  PlatformResult RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                            SysteminfoInstance& instance);
  PlatformResult UnregisterPeripheralListener();
  PlatformResult RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                        SysteminfoInstance& instance);
  PlatformResult UnregisterMemoryListener();
  PlatformResult RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                        SysteminfoInstance& instance);
  PlatformResult UnregisterCameraFlashListener();

  void SetCpuInfoLoad(double load);
  void SetAvailableCapacityInternal(unsigned long long capacity);
  void SetAvailableCapacityMmc(unsigned long long capacity);

  void OnBatteryChangedCallback(keynode_t* node, void* event_ptr);
  void OnCpuChangedCallback(void* event_ptr);
  void OnStorageChangedCallback(void* event_ptr);
  void OnMmcChangedCallback(keynode_t* node, void* event_ptr);
  void OnDisplayChangedCallback(keynode_t* node, void* event_ptr);
  void OnDeviceAutoRotationChangedCallback(keynode_t* node, void* event_ptr);
  void OnDeviceOrientationChangedCallback(sensor_t sensor, unsigned int event_type,
                                          sensor_data_t *data, void *user_data);
  void OnLocaleChangedCallback(system_settings_key_e key, void* event_ptr);
  void OnNetworkChangedCallback(connection_type_e type, void* event_ptr);
  void OnNetworkValueCallback(const char* ipv4_address,
                              const char* ipv6_address, void* event_ptr);
  void OnCellularNetworkValueCallback(keynode_t *node, void *event_ptr);
  void OnPeripheralChangedCallback(keynode_t* node, void* event_ptr);
  void OnMemoryChangedCallback(keynode_t* node, void* event_ptr);
  void OnBrightnessChangedCallback(device_callback_e type, void* value, void* user_data);

  TapiHandle* GetTapiHandle();
  TapiHandle** GetTapiHandles();
  PlatformResult GetConnectionHandle(connection_h&);
  int GetSensorHandle();
  PlatformResult ConnectSensor(int* result);
  void DisconnectSensor(int handle_orientation);
  wifi_rssi_level_e GetWifiLevel();
  void SetWifiLevel(wifi_rssi_level_e level);
  std::string GetCameraTypes(int index);
  int GetCameraTypesCount();
 private:
  static PlatformResult RegisterVconfCallback(const char *in_key, vconf_callback_fn cb,
                                              SysteminfoInstance& instance);
  static PlatformResult UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb);
  PlatformResult RegisterIpChangeCallback(SysteminfoInstance& instance);
  PlatformResult UnregisterIpChangeCallback();
  bool IsIpChangeCallbackInvalid();
  void InitTapiHandles();
  void InitCameraTypes();

  guint m_cpu_event_id;
  guint m_storage_event_id;

  double m_cpu_load;
  double m_last_cpu_load;
  unsigned long long m_available_capacity_internal;
  unsigned long long m_available_capacity_mmc;
  unsigned long long m_last_available_capacity_internal;
  unsigned long long m_last_available_capacity_mmc;
  wifi_rssi_level_e m_wifi_level;

  SysteminfoUtilsCallback m_battery_listener;
  SysteminfoUtilsCallback m_cpu_listener;
  SysteminfoUtilsCallback m_storage_listener;
  SysteminfoUtilsCallback m_display_listener;
  SysteminfoUtilsCallback m_device_orientation_listener;
  SysteminfoUtilsCallback m_locale_listener;
  SysteminfoUtilsCallback m_network_listener;
  SysteminfoUtilsCallback m_wifi_network_listener;
  SysteminfoUtilsCallback m_ethernet_network_listener;
  SysteminfoUtilsCallback m_cellular_network_listener;
  SysteminfoUtilsCallback m_peripheral_listener;
  SysteminfoUtilsCallback m_memory_listener;
  SysteminfoUtilsCallback m_camera_flash_listener;

  TapiHandle *m_tapi_handles[kTapiMaxHandle+1];
  //for ip change callback
  connection_h m_connection_handle;
  //! Sensor handle for DeviceOrientation purposes
  int m_sensor_handle;
  std::vector<std::string> m_camera_types;
};
SystemInfoListeners::SystemInfoListeners():
            m_cpu_event_id(0),
            m_storage_event_id(0),
            m_cpu_load(0),
            m_last_cpu_load(0),
            m_available_capacity_internal(0),
            m_available_capacity_mmc(0),
            m_last_available_capacity_internal(0),
            m_last_available_capacity_mmc(0),
            m_wifi_level(WIFI_RSSI_LEVEL_0),
            m_battery_listener(nullptr),
            m_cpu_listener(nullptr),
            m_storage_listener(nullptr),
            m_display_listener(nullptr),
            m_device_orientation_listener(nullptr),
            m_locale_listener(nullptr),
            m_network_listener(nullptr),
            m_wifi_network_listener(nullptr),
            m_ethernet_network_listener(nullptr),
            m_cellular_network_listener(nullptr),
            m_peripheral_listener(nullptr),
            m_memory_listener(nullptr),
            m_camera_flash_listener(nullptr),
            m_tapi_handles{nullptr},
            m_connection_handle(nullptr),
            m_sensor_handle(-1)
{
  LoggerD("Entered");
  int error = wifi_initialize();
  if (WIFI_ERROR_NONE != error) {
    std::string log_msg = "Initialize failed: " + parseWifiNetworkError(error);
    LoggerE("%s", log_msg.c_str());
  } else {
    LoggerD("WIFI initialization succeed");
  }

  error = wifi_set_rssi_level_changed_cb(OnWifiLevelChangedCb, nullptr);
  if (WIFI_ERROR_NONE != error) {
    std::string log_msg = "Setting wifi listener failed: " + parseWifiNetworkError(error);
    LoggerE("%s", log_msg.c_str());
  } else {
    LoggerD("Setting wifi listener succeed");
  }
  InitCameraTypes();
}

SystemInfoListeners::~SystemInfoListeners(){
  LoggerD("Entered");
  UnregisterBatteryListener();
  UnregisterCpuListener();
  UnregisterStorageListener();
  UnregisterDisplayListener();
  UnregisterDeviceOrientationListener();
  UnregisterLocaleListener();
  UnregisterNetworkListener();
  UnregisterWifiNetworkListener();
  UnregisterCellularNetworkListener();
  UnregisterPeripheralListener();
  UnregisterMemoryListener();

  DisconnectSensor(m_sensor_handle);

  unsigned int i = 0;
  while(m_tapi_handles[i]) {
    tel_deinit(m_tapi_handles[i]);
    i++;
  }
  if (nullptr != m_connection_handle) {
    connection_destroy(m_connection_handle);
  }

  wifi_deinitialize();
}

#define CHECK_LISTENER_ERROR(method) \
  ret = method; \
  if (ret.IsError()) { \
    return ret; \
  }

int SystemInfoListeners::GetSensorHandle() {
  if (m_sensor_handle < 0) {
    LoggerD("Connecting to sensor");
    ConnectSensor(&m_sensor_handle);
  } else {
    LoggerD("Sensor already connected");
  }
  return m_sensor_handle;
}

PlatformResult SystemInfoListeners::ConnectSensor(int* result) {
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

void SystemInfoListeners::DisconnectSensor(int handle_orientation)
{
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

wifi_rssi_level_e SystemInfoListeners::GetWifiLevel()
{
  return m_wifi_level;
}

void SystemInfoListeners::SetWifiLevel(wifi_rssi_level_e level)
{
  m_wifi_level = level;
}

std::string SystemInfoListeners::GetCameraTypes(int index) {
  if (index >= m_camera_types.size()) {
    return "";
  }
  return m_camera_types[index];
}

int SystemInfoListeners::GetCameraTypesCount() {
  return m_camera_types.size();
}

PlatformResult SystemInfoListeners::RegisterBatteryListener(
    const SysteminfoUtilsCallback& callback, SysteminfoInstance& instance) {
  LoggerD("Entered");
  if (nullptr == m_battery_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb, instance))
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb, instance))
    LoggerD("Added callback for BATTERY");
    m_battery_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterBatteryListener()
{
  if (nullptr != m_battery_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb))
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb))
    LoggerD("Removed callback for BATTERY");
    m_battery_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  if (nullptr == m_cpu_listener) {
    m_cpu_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnCpuChangedCb, static_cast<void*>(&instance));
    LoggerD("Added callback for CPU");
    m_cpu_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterCpuListener()
{
  if (nullptr != m_cpu_listener) {
    g_source_remove(m_cpu_event_id);
    m_cpu_event_id = 0;
    LoggerD("Removed callback for CPU");
    m_cpu_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  if (nullptr == m_storage_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb, instance))

    m_storage_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnStorageChangedCb, static_cast<void*>(&instance));
    LoggerD("Added callback for STORAGE");
    m_storage_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterStorageListener()
{
  if (nullptr != m_storage_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb))

    g_source_remove(m_storage_event_id);
    m_storage_event_id = 0;
    LoggerD("Removed callback for STORAGE");
    m_storage_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  if (nullptr == m_display_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb, instance))
    LoggerD("Added callback for DISPLAY");
    m_display_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterDisplayListener()
{
  if (nullptr != m_display_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb))
    LoggerD("Removed callback for DISPLAY");
    m_display_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                                      SysteminfoInstance& instance)
{
  if (nullptr == m_device_orientation_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, OnDeviceAutoRotationChangedCb, instance))

    bool sensor_ret = sensord_register_event(GetSensorHandle(), AUTO_ROTATION_EVENT_CHANGE_STATE,
                                     BASE_GATHERING_INTERVAL, 0,
                                     OnDeviceOrientationChangedCb, static_cast<void*>(&instance));
    if (!sensor_ret) {
      LoggerE("Failed to register orientation change event listener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to register orientation change event listener");
    }

    LoggerD("Added callback for DEVICE_ORIENTATION");
    m_device_orientation_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterDeviceOrientationListener()
{
  if (nullptr != m_device_orientation_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, OnDeviceAutoRotationChangedCb))
    bool sensor_ret = sensord_unregister_event(GetSensorHandle(), AUTO_ROTATION_EVENT_CHANGE_STATE);
    if (!sensor_ret) {
      LoggerE("Failed to unregister orientation change event listener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister"
          " orientation change event listener");
    }

    LoggerD("Removed callback for DEVICE_ORIENTATION");
    m_device_orientation_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  if (nullptr == m_locale_listener) {
    if (SYSTEM_SETTINGS_ERROR_NONE !=
        system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY,
                                    OnLocaleChangedCb, static_cast<void*>(&instance)) ) {
      LoggerE("Country change callback registration failed");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Country change callback registration failed");
    }
    if (SYSTEM_SETTINGS_ERROR_NONE !=
        system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
                                    OnLocaleChangedCb, static_cast<void*>(&instance)) ) {
      LoggerE("Language change callback registration failed");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Language change callback registration failed");
    }
    LoggerD("Added callback for LOCALE");
    m_locale_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterLocaleListener()
{
  if (nullptr != m_locale_listener) {
    if (SYSTEM_SETTINGS_ERROR_NONE !=
        system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE) ) {
      LoggerE("Unregistration of language change callback failed");
    }
    if (SYSTEM_SETTINGS_ERROR_NONE !=
        system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY) ) {
      LoggerE("Unregistration of country change callback failed");
    }
    LoggerD("Removed callback for LOCALE");
    m_locale_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  if (nullptr == m_network_listener) {
    connection_h handle;
    PlatformResult ret(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
    if (CONNECTION_ERROR_NONE !=
        connection_set_type_changed_cb(handle, OnNetworkChangedCb, static_cast<void*>(&instance))) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Registration of listener failed");
    }
    LoggerD("Added callback for NETWORK");
    m_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterNetworkListener()
{
  if (nullptr != m_network_listener) {
    connection_h handle;
    PlatformResult ret(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
    if (CONNECTION_ERROR_NONE != connection_unset_type_changed_cb(handle)) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unregistration of listener failed");
    }
    LoggerD("Removed callback for NETWORK");
    m_network_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance)
{
  LoggerD("Entered");

  if (IsIpChangeCallbackInvalid()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(RegisterIpChangeCallback(instance));
    LoggerD("Registered IP change listener");
  } else {
    LoggerD("No need to register IP listener on platform, already registered");
  }

  if (nullptr == m_wifi_network_listener) {
    LoggerD("Added callback for WIFI_NETWORK");
    m_wifi_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterWifiNetworkListener()
{
  LoggerD("Entered");

  m_wifi_network_listener = nullptr;

  if (IsIpChangeCallbackInvalid()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback());
    LoggerD("Removed IP change listener");
  } else {
    LoggerD("Removed callback for WIFI_NETWORK, but IP change listener still works");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CheckIfEthernetNetworkSupported()
{
  LoggerD("Entered");
  connection_h connection_handle = nullptr;
  connection_ethernet_state_e connection_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;

  int error = connection_create(&connection_handle);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot create connection: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  std::unique_ptr<std::remove_pointer<connection_h>::type, int (*)(connection_h)> connection_handle_ptr(
    connection_handle, &connection_destroy);  // automatically release the memory

  error = connection_get_ethernet_state(connection_handle, &connection_state);
  if (CONNECTION_ERROR_NOT_SUPPORTED == error) {
    std::string log_msg = "Cannot get ethernet connection state: Not supported";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, log_msg);
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterEthernetNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                    SysteminfoInstance& instance)
{
  LoggerD("Entered");
  PlatformResult ret = CheckIfEthernetNetworkSupported();
  if (ret.IsError()){
    return ret;
  }

  if (IsIpChangeCallbackInvalid()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(RegisterIpChangeCallback(instance));
    LoggerD("Registered IP change listener");
  } else {
    LoggerD("No need to register IP listener on platform, already registered");
  }

  if (nullptr == m_ethernet_network_listener) {
    LoggerD("Added callback for ETHERNET_NETWORK");
    m_ethernet_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterEthernetNetworkListener()
{
  LoggerD("Entered");

  m_ethernet_network_listener = nullptr;

  if (IsIpChangeCallbackInvalid()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback());
    LoggerD("Removed IP change listener");
  } else {
    LoggerD("Removed callback for ETHERNET_NETWORK, but IP change listener still works");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                    SysteminfoInstance& instance)
{
  LoggerD("Entered");
  PlatformResult ret = SysteminfoUtils::CheckTelephonySupport();
  if (ret.IsError()) {
      return ret;
  }

  if (IsIpChangeCallbackInvalid()) {
    CHECK_LISTENER_ERROR(RegisterIpChangeCallback(instance));
    LoggerD("Registered IP change listener");
  } else {
    LoggerD("No need to register IP listener on platform, already registered");
  }

  if (nullptr == m_cellular_network_listener) {
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                          OnCellularNetworkValueChangedCb, instance))
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                          OnCellularNetworkValueChangedCb, instance))
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                          OnCellularNetworkValueChangedCb, instance))
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                          OnCellularNetworkValueChangedCb, instance))
    LoggerD("Added callback for CELLULAR_NETWORK");
    m_cellular_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterCellularNetworkListener()
{
  LoggerD("Entered");

  if (nullptr != m_cellular_network_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                            OnCellularNetworkValueChangedCb))
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                            OnCellularNetworkValueChangedCb))
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                            OnCellularNetworkValueChangedCb))
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                            OnCellularNetworkValueChangedCb))
  }
  m_cellular_network_listener = nullptr;

  if (IsIpChangeCallbackInvalid()) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback());
    LoggerD("Removed IP change listener");
  } else {
    LoggerD("Removed callback for CELLULAR_NETWORK, but IP change listener still works");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                                               SysteminfoInstance& instance)
{
  if (nullptr == m_peripheral_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
/*    if (-1 != vconf_get_int(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &value)) {
      CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS,
                                                 OnPeripheralChangedCb, instance))
    }*/
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_HDMI, &value)) {
      CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_SYSMAN_HDMI,
                                                 OnPeripheralChangedCb, instance))
    }

    LoggerD("Added callback for PERIPHERAL");
    m_peripheral_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterPeripheralListener()
{
  if (nullptr != m_peripheral_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
/*    if (-1 != vconf_get_int(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &value)) {
      CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS,
                                                   OnPeripheralChangedCb))
    }*/
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_HDMI, &value)) {
      CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_SYSMAN_HDMI,
                                                   OnPeripheralChangedCb))
    }

    LoggerD("Removed callback for PERIPHERAL");
    m_peripheral_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  if (nullptr == m_memory_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
      CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY, OnMemoryChangedCb, instance))
    }
    LoggerD("Added callback for MEMORY");
    m_memory_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterMemoryListener()
{
  if (nullptr != m_memory_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
      CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY, OnMemoryChangedCb))
    }
    LoggerD("Removed callback for MEMORY");
    m_memory_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  if (nullptr == m_camera_flash_listener) {
    if (DEVICE_ERROR_NONE != device_add_callback(DEVICE_CALLBACK_FLASH_BRIGHTNESS,
                              OnBrightnessChangedCb, static_cast<void*>(&instance))) {
        return PlatformResult(ErrorCode::UNKNOWN_ERR);
      }
      m_camera_flash_listener = callback;
  }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterCameraFlashListener()
{
  if (nullptr != m_camera_flash_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    if (DEVICE_ERROR_NONE != device_remove_callback(DEVICE_CALLBACK_FLASH_BRIGHTNESS,
                                                 OnBrightnessChangedCb)) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR);
    }
    LoggerD("Removed callback for camera_flash");
    m_camera_flash_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}


void SystemInfoListeners::SetCpuInfoLoad(double load)
{
  m_cpu_load = load;
}

void SystemInfoListeners::SetAvailableCapacityInternal(unsigned long long capacity)
{
  m_available_capacity_internal = capacity;
}

void SystemInfoListeners::SetAvailableCapacityMmc(unsigned long long capacity)
{
  m_available_capacity_mmc = capacity;
}

void SystemInfoListeners::OnBatteryChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_battery_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_battery_listener(*instance);
  }
}

void SystemInfoListeners::OnCpuChangedCallback(void* event_ptr)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
//  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdCpu, false, result);
//  if (ret.IsSuccess()) {
//    if (m_cpu_load == m_last_cpu_load) {
//      return;
//    }
//    if (nullptr != m_cpu_listener) {
//      SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
//      m_last_cpu_load = m_cpu_load;
//      m_cpu_listener(*instance);
//    }
//  }
}

void SystemInfoListeners::OnStorageChangedCallback(void* event_ptr)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
//  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, false, result);
//  if (ret.IsSuccess()) {
//    if (m_available_capacity_internal == m_last_available_capacity_internal) {
//      return;
//    }
//
//    if (nullptr != m_storage_listener) {
//      SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
//      m_last_available_capacity_internal = m_available_capacity_internal;
//      m_storage_listener(*instance);
//    }
//  }
}

void SystemInfoListeners::OnMmcChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
//  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, false, result);
//  if (ret.IsSuccess()) {
//    if (m_available_capacity_mmc == m_last_available_capacity_mmc) {
//      return;
//    }
//    if (nullptr != m_storage_listener) {
//      SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
//      m_last_available_capacity_mmc = m_available_capacity_mmc;
//      m_storage_listener(*instance);
//    }
//  }
}


void SystemInfoListeners::OnDisplayChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_display_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_display_listener(*instance);
  }
}

void SystemInfoListeners::OnDeviceAutoRotationChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_device_orientation_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_device_orientation_listener(*instance);
  }
}

void SystemInfoListeners::OnDeviceOrientationChangedCallback(sensor_t sensor, unsigned int event_type,
                                                             sensor_data_t *data, void *user_data)
{
  if (nullptr != m_device_orientation_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(user_data);
    m_device_orientation_listener(*instance);
  }
}

void SystemInfoListeners::OnLocaleChangedCallback(system_settings_key_e /*key*/, void* event_ptr)
{
  if (nullptr != m_locale_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_locale_listener(*instance);
  }
}

void SystemInfoListeners::OnNetworkChangedCallback(connection_type_e /*type*/, void* event_ptr)
{
  if (nullptr != m_network_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_network_listener(*instance);
  }
}

void SystemInfoListeners::OnNetworkValueCallback(const char* /*ipv4_address*/,
                                                 const char* /*ipv6_address*/, void* event_ptr)
{
  LoggerD("Entered");

  SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
  if (nullptr != m_wifi_network_listener) {
    m_wifi_network_listener(*instance);
  }
  if (nullptr != m_ethernet_network_listener) {
    m_ethernet_network_listener(*instance);
  }
  if (nullptr != m_cellular_network_listener) {
    m_cellular_network_listener(*instance);
  }
}

void SystemInfoListeners::OnCellularNetworkValueCallback(keynode_t */*node*/, void *event_ptr)
{
  if (nullptr != m_cellular_network_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_cellular_network_listener(*instance);
  }
}

void SystemInfoListeners::OnPeripheralChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_peripheral_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_peripheral_listener(*instance);
  }
}

void SystemInfoListeners::OnMemoryChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_memory_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_memory_listener(*instance);
  }
}

void SystemInfoListeners::OnBrightnessChangedCallback(device_callback_e type, void* value, void* user_data)
{
  if (nullptr != m_camera_flash_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(user_data);
    m_camera_flash_listener(*instance);
  }
}

void SystemInfoListeners::InitTapiHandles()
{
  LoggerD("Entered");
  int sim_count = 0;
  if (nullptr == m_tapi_handles[0]){  //check if anything is in table
    char **cp_list = tel_get_cp_name_list();
    if (nullptr != cp_list) {
      while (cp_list[sim_count]) {
        m_tapi_handles[sim_count] = tel_init(cp_list[sim_count]);
        if (nullptr == m_tapi_handles[sim_count]) {
          LoggerE("Failed to connect with tapi, handle is null");
          break;
        }
        sim_count++;
        LoggerD("%d modem: %s", sim_count, cp_list[sim_count]);
      }
    } else {
      LoggerE("Failed to get cp list");
      sim_count = kTapiMaxHandle;
    }
    g_strfreev(cp_list);
  }
}

void SystemInfoListeners::InitCameraTypes() {
  bool supported = false;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
      "tizen.org/feature/camera.back.flash", &supported);
  if (ret.IsSuccess()) {
    if (supported) {
      m_camera_types.push_back("BACK");
    }
  }
  ret = SystemInfoDeviceCapability::GetValueBool(
      "tizen.org/feature/camera.front.flash", &supported);
  if (ret.IsSuccess()) {
    if (supported) {
      m_camera_types.push_back("FRONT");
    }
  }
}

TapiHandle* SystemInfoListeners::GetTapiHandle() {

  LoggerD("Entered");
  InitTapiHandles();
  return m_tapi_handles[0];
}

TapiHandle** SystemInfoListeners::GetTapiHandles()
{
  InitTapiHandles();
  return m_tapi_handles;
}

PlatformResult SystemInfoListeners::GetConnectionHandle(connection_h& handle)
{
  if (nullptr == m_connection_handle) {
    int error = connection_create(&m_connection_handle);
    if (CONNECTION_ERROR_NONE != error) {
      LoggerE("Failed to create connection: %d", error);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot create connection");
    }
  }
  handle = m_connection_handle;
  return PlatformResult(ErrorCode::NO_ERROR);
}

//////////////// Private ////////////////////

PlatformResult SystemInfoListeners::RegisterVconfCallback(const char *in_key, vconf_callback_fn cb,
                                                          SysteminfoInstance& instance)
{
  if (0 != vconf_notify_key_changed(in_key, cb, static_cast<void*>(&instance))) {
    LoggerE("Failed to register vconf callback: %s", in_key);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register vconf callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb)
{
  if (0 != vconf_ignore_key_changed(in_key, cb)) {
    LoggerE("Failed to unregister vconf callback: %s", in_key);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister vconf callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterIpChangeCallback(SysteminfoInstance& instance)
{
  LoggerD("Registering connection callback");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  int error = connection_set_ip_address_changed_cb(handle,
                                                   OnNetworkValueChangedCb,
                                                   static_cast<void*>(&instance));
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to register ip change callback: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot register ip change callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterIpChangeCallback()
{
  LoggerD("Unregistering connection callback");
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

bool SystemInfoListeners::IsIpChangeCallbackInvalid() {
  LoggerD("Entered");
  return (nullptr == m_wifi_network_listener &&
          nullptr == m_ethernet_network_listener &&
          nullptr == m_cellular_network_listener);
}

/////////////////////////// system_info_listeners object ////////////////////////

static SystemInfoListeners system_info_listeners;

//global sim manager - needed for async gathering informations
static SimDetailsManager sim_mgr;

/////////////////// Callbacks ///////////////////////////////////////////////////

void OnBatteryChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnBatteryChangedCallback(node, event_ptr);
}

gboolean OnCpuChangedCb(gpointer event_ptr)
{
  LoggerD("");
  system_info_listeners.OnCpuChangedCallback(event_ptr);
  return G_SOURCE_CONTINUE;
}
gboolean OnStorageChangedCb(gpointer event_ptr)
{
  LoggerD("");
  system_info_listeners.OnStorageChangedCallback(event_ptr);
  return G_SOURCE_CONTINUE;
}
void OnMmcChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnMmcChangedCallback(node, event_ptr);
}

void OnDisplayChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnDisplayChangedCallback(node, event_ptr);
}

void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnDeviceAutoRotationChangedCallback(node, event_ptr);
}

void OnDeviceOrientationChangedCb(sensor_t sensor, unsigned int event_type,
                                  sensor_data_t *data, void *user_data)
{
  LoggerD("");
  system_info_listeners.OnDeviceOrientationChangedCallback(sensor, event_type, data, user_data);
}

void OnLocaleChangedCb(system_settings_key_e key, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnLocaleChangedCallback(key, event_ptr);
}

void OnNetworkChangedCb(connection_type_e type, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnNetworkChangedCallback(type, event_ptr);
}

void OnNetworkValueChangedCb(const char* ipv4_address,
                             const char* ipv6_address, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnNetworkValueCallback(ipv4_address, ipv6_address, event_ptr);
}

void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr)
{
  LoggerD("");
  system_info_listeners.OnCellularNetworkValueCallback(node, event_ptr);
}

void OnPeripheralChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnPeripheralChangedCallback(node, event_ptr);
}

void OnMemoryChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnMemoryChangedCallback(node, event_ptr);
}

void OnBrightnessChangedCb(device_callback_e type, void* value, void* user_data)
{
  LoggerD("");
  if (type == DEVICE_CALLBACK_FLASH_BRIGHTNESS) {
    system_info_listeners.OnBrightnessChangedCallback(type, value, user_data);
  }
}

void OnWifiLevelChangedCb (wifi_rssi_level_e rssi_level, void *user_data)
{
  LoggerD("Entered");
  LoggerD("Level %d", rssi_level);
  system_info_listeners.SetWifiLevel(rssi_level);
}

/////////////////////////// SysteminfoUtils ////////////////////////////////
PlatformResult SysteminfoUtils::RegisterBatteryListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterBatteryListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterBatteryListener()
{
  return system_info_listeners.UnregisterBatteryListener();
}


PlatformResult SysteminfoUtils::RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                                    SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterCpuListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterCpuListener()
{
  return system_info_listeners.UnregisterCpuListener();
}


PlatformResult SysteminfoUtils::RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterStorageListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterStorageListener()
{
  return system_info_listeners.UnregisterStorageListener();
}

PlatformResult SysteminfoUtils::RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterDisplayListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterDisplayListener()
{
  return system_info_listeners.UnregisterDisplayListener();
}

PlatformResult SysteminfoUtils::RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                                  SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterDeviceOrientationListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterDeviceOrientationListener()
{
  return system_info_listeners.UnregisterDeviceOrientationListener();
}

PlatformResult SysteminfoUtils::RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterLocaleListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterLocaleListener()
{
  return system_info_listeners.UnregisterLocaleListener();
}

PlatformResult SysteminfoUtils::RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterNetworkListener()
{
  return system_info_listeners.UnregisterNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterWifiNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterWifiNetworkListener()
{
  return system_info_listeners.UnregisterWifiNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterEthernetNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance)
{
  LoggerD("Entered");
  return system_info_listeners.RegisterEthernetNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterEthernetNetworkListener()
{
  LoggerD("Entered");
  return system_info_listeners.UnregisterEthernetNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterCellularNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterCellularNetworkListener()
{
  return system_info_listeners.UnregisterCellularNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterPeripheralListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterPeripheralListener()
{
  return system_info_listeners.UnregisterPeripheralListener();
}

PlatformResult SysteminfoUtils::RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterMemoryListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterMemoryListener()
{
  return system_info_listeners.UnregisterMemoryListener();
}

PlatformResult SysteminfoUtils::RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterCameraFlashListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterCameraFlashListener()
{
  return system_info_listeners.UnregisterCameraFlashListener();
}

PlatformResult SysteminfoUtils::GetVconfInt(const char *key, int *value) {
  if (0 == vconf_get_int(key, value)) {
    LoggerD("value[%s]: %d", key, *value);
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  const std::string error_msg = "Could not get " + std::string(key);
  LoggerD("%s",error_msg.c_str());
  return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
}

PlatformResult SysteminfoUtils::GetRuntimeInfoString(system_settings_key_e key, std::string* platform_string) {
  char* platform_c_string;
  int err = system_settings_get_value_string(key, &platform_c_string);
  if (SYSTEM_SETTINGS_ERROR_NONE == err) {
    if (nullptr != platform_c_string) {
      *platform_string = platform_c_string;
      free(platform_c_string);
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  const std::string error_msg = "Error when retrieving system setting information: "
      + std::to_string(err);
  LoggerE("%s", error_msg.c_str());
  return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
}

PlatformResult SysteminfoUtils::CheckTelephonySupport() {
  bool supported = false;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
    "tizen.org/feature/network.telephony", &supported);
  if (ret.IsError()) {
    return ret;
  }
  if (!supported) {
    LoggerD("Telephony is not supported on this device");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR,
        "Telephony is not supported on this device");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::CheckCameraFlashSupport() {
  bool supported = false;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
    "tizen.org/feature/camera.back.flash", &supported);
  if (ret.IsError()) {
    return ret;
  }
  if (!supported) {
    LoggerD("Back-facing camera with a flash is not supported on this device");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR,
        "Back-facing camera with a flash is not supported on this device");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::CheckIfEthernetNetworkSupported()
{
  LoggerD("Entered");
  connection_h connection_handle = nullptr;
  connection_ethernet_state_e connection_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;

  int error = connection_create(&connection_handle);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot create connection: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  std::unique_ptr<std::remove_pointer<connection_h>::type, int (*)(connection_h)> connection_handle_ptr(
    connection_handle, &connection_destroy);  // automatically release the memory

  error = connection_get_ethernet_state(connection_handle, &connection_state);
  if (CONNECTION_ERROR_NOT_SUPPORTED == error) {
    std::string log_msg = "Cannot get ethernet connection state: Not supported";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, log_msg);
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetTotalMemory(long long* result)
{
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_total(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  *result = static_cast<long long>(value*MEMORY_TO_BYTE);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetAvailableMemory(long long* result)
{
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_available(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  *result = static_cast<long long>(value*MEMORY_TO_BYTE);
  return PlatformResult(ErrorCode::NO_ERROR);
}

} // namespace systeminfo
} // namespace webapi
