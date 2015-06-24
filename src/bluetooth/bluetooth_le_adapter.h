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

#ifndef BLUETOOTH_BLUETOOTH_LE_ADAPTER_H_
#define BLUETOOTH_BLUETOOTH_LE_ADAPTER_H_

#include <bluetooth.h>
#include <bluetooth_internal.h>
#include "common/picojson.h"

namespace extension {
namespace bluetooth {

class BluetoothInstance;

class BluetoothLEAdapter {
 public:
  explicit BluetoothLEAdapter(BluetoothInstance& instance);
  ~BluetoothLEAdapter();

  void StartScan(const picojson::value& data, picojson::object& out);
  void StopScan(const picojson::value& data, picojson::object& out);

  void StartAdvertise(const picojson::value& data, picojson::object& out);
  void StopAdvertise(const picojson::value& data, picojson::object& out);

 private:
  BluetoothLEAdapter() = delete;
  BluetoothLEAdapter(const BluetoothLEAdapter&) = delete;
  BluetoothLEAdapter(const BluetoothLEAdapter&&) = delete;
  BluetoothLEAdapter& operator=(const BluetoothLEAdapter&) = delete;
  BluetoothLEAdapter& operator=(const BluetoothLEAdapter&&) = delete;

  static void OnStateChanged(int result, bt_adapter_le_state_e adapter_le_state,
                             void* user_data);
  static void OnScanResult(int result,
                           bt_adapter_le_device_scan_result_info_s* info,
                           void* user_data);
  static void OnAdvertiseResult(int result, bt_advertiser_h advertiser,
                                bt_adapter_le_advertising_state_e adv_state,
                                void* user_data);

  BluetoothInstance& instance_;
  bool enabled_;
  bool scanning_;
  bt_advertiser_h bt_advertiser_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_LE_ADAPTER_H_
