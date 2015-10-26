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

#ifndef BLUETOOTH_BLUETOOTH_LE_DEVICE_H_
#define BLUETOOTH_BLUETOOTH_LE_DEVICE_H_

#include <bluetooth.h>

#include <set>
#include <unordered_map>

#include "bluetooth/bluetooth_gatt_service.h"

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace bluetooth {

class BluetoothInstance;

class BluetoothLEDevice {
 public:
  explicit BluetoothLEDevice(BluetoothInstance& instance, BluetoothGATTService& service);
  ~BluetoothLEDevice();

  void Connect(const picojson::value& data, picojson::object& out);
  void Disconnect(const picojson::value& data, picojson::object& out);

  void GetService(const picojson::value& data, picojson::object& out);

  void AddConnectStateChangeListener(const picojson::value& data,
                                     picojson::object& out);
  void RemoveConnectStateChangeListener(const picojson::value& data,
                                        picojson::object& out);

  void GetServiceUuids(const picojson::value& data, picojson::object& out);

  static common::PlatformResult ToJson(
      bt_adapter_le_device_scan_result_info_s* info,
      picojson::object* le_device);

 private:
  static void GattConnectionState(int result, bool connected,
                                  const char *remote_address, void *user_data);

  BluetoothInstance& instance_;
  BluetoothGATTService& service_;
  std::unordered_map<std::string, double> connecting_;
  bool is_listener_set_;
  std::set<std::string> is_connected_;
};

}  // namespace bluetooth
}  // namespace extension

#endif // BLUETOOTH_BLUETOOTH_LE_DEVICE_H_
