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

#ifndef WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_MANAGER_H__
#define WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_MANAGER_H__

#include <memory>
#include <mutex>
#include <set>

#include <ITapiModem.h>
#include <net_connection.h>
#include <wifi.h>

#include "common/picojson.h"
#include "common/platform_result.h"
#include "systeminfo/systeminfo_properties_manager.h"

namespace extension {
namespace systeminfo {

class SysteminfoInstance;

class SysteminfoManager {
 public:
  SysteminfoManager(SysteminfoInstance* instance);
  ~SysteminfoManager();

  void GetCapabilities(const picojson::value& args, picojson::object* out);
  void GetCapability(const picojson::value& args, picojson::object* out);
  void GetPropertyValue(const picojson::value& args, picojson::object* out);
  void GetPropertyValueArray(const picojson::value& args, picojson::object* out);
  void AddPropertyValueChangeListener(const picojson::value& args, picojson::object* out);
  void RemovePropertyValueChangeListener(const picojson::value& args, picojson::object* out);
  void GetMaxBrightness(const picojson::value& args, picojson::object* out);
  void GetBrightness(const picojson::value& args, picojson::object* out);
  void SetBrightness(const picojson::value& args, picojson::object* out);
  void GetTotalMemory(const picojson::value& args, picojson::object* out);
  void GetAvailableMemory(const picojson::value& args, picojson::object* out);
  void GetCount(const picojson::value& args, picojson::object* out);

  common::PlatformResult GetPropertyCount(const std::string& property, unsigned long* count);
  wifi_rssi_level_e GetWifiLevel();
  void SetWifiLevel(wifi_rssi_level_e level);
  int GetSensorHandle();

  int GetChangedTapiIndex(TapiHandle* tapi);
  common::PlatformResult GetNetworkType(std::size_t index, int* network_type);
  common::PlatformResult GatherSimInformation(std::size_t index, picojson::object* out);
  common::PlatformResult FetchBasicSimProperties(std::size_t index,
                                                unsigned short* result_mcc,
                                                unsigned short* result_mnc,
                                                unsigned short* result_cell_id,
                                                unsigned short* result_lac,
                                                bool* result_is_roaming,
                                                bool* result_is_flight_mode,
                                                std::string* result_imei);

  common::PlatformResult GetConnectionHandle(connection_h& handle);

  SysteminfoInstance* GetInstance() { return instance_;};
  SysteminfoPropertiesManager& GetPropertiesManager() { return prop_manager_;};

  void SetCpuInfoLoad(double load);
  void SetAvailableCapacityInternal(unsigned long long capacity);
  void SetAvailableCapacityMmc(unsigned long long capacity);
  std::string GetCameraTypes(unsigned int index);
  unsigned int GetCameraTypesCount();

  bool IsListenerRegistered(const std::string& property_id);
  void CallListenerCallback(const std::string& property_id, int property_index = 0);
  void CallCpuListenerCallback();
  void CallStorageListenerCallback();
 private:
  class TapiManager;

  void PostListenerResponse(const std::string& property_id, const picojson::value& result,
                            int property_index = 0);
  common::PlatformResult ConnectSensor(int* result);
  void DisconnectSensor(int handle_orientation);
  void InitCameraTypes();

  bool IsIpChangeCallbackNotRegistered();
  common::PlatformResult RegisterIpChangeCallback();
  common::PlatformResult UnregisterIpChangeCallback();

  common::PlatformResult RegisterBatteryListener();
  common::PlatformResult UnregisterBatteryListener();
  common::PlatformResult RegisterCpuListener();
  common::PlatformResult UnregisterCpuListener();
  common::PlatformResult RegisterStorageListener();
  common::PlatformResult UnregisterStorageListener();
  common::PlatformResult RegisterDisplayListener();
  common::PlatformResult UnregisterDisplayListener();
  common::PlatformResult RegisterDeviceOrientationListener();
  common::PlatformResult UnregisterDeviceOrientationListener();
  common::PlatformResult RegisterLocaleListener();
  common::PlatformResult UnregisterLocaleListener();
  common::PlatformResult RegisterNetworkListener();
  common::PlatformResult UnregisterNetworkListener();
  common::PlatformResult RegisterWifiNetworkListener();
  common::PlatformResult UnregisterWifiNetworkListener();
  common::PlatformResult RegisterEthernetNetworkListener();
  common::PlatformResult UnregisterEthernetNetworkListener();
  common::PlatformResult RegisterCellularNetworkListener();
  common::PlatformResult UnregisterCellularNetworkListener();
  common::PlatformResult RegisterPeripheralListener();
  common::PlatformResult UnregisterPeripheralListener();
  common::PlatformResult RegisterMemoryListener();
  common::PlatformResult UnregisterMemoryListener();
  common::PlatformResult RegisterCameraFlashListener();
  common::PlatformResult UnregisterCameraFlashListener();

  SysteminfoInstance* instance_;
  SysteminfoPropertiesManager prop_manager_;

  //! Sensor handle for DeviceOrientation purposes
  int sensor_handle_;
  std::mutex sensor_mutex_;

  std::vector<std::string> camera_types_;
  wifi_rssi_level_e wifi_level_;
  double cpu_load_;
  double last_cpu_load_;
  unsigned long long available_capacity_internal_;
  unsigned long long last_available_capacity_internal_;
  unsigned long long available_capacity_mmc_;
  unsigned long long last_available_capacity_mmc_;

  std::unique_ptr<TapiManager> tapi_manager_;

  std::set<std::string> registered_listeners_;

  guint cpu_event_id_;
  guint storage_event_id_;

  connection_h connection_handle_;
  std::mutex connection_mutex_;

};
} // namespace systeminfo
} // namespace webapi

#endif // WEBAPI_PLUGINS_SYSTEMINFO_SYSTEMINFO_MANAGER_H__
