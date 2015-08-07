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

#ifndef BLUETOOTH_BLUETOOTH_GATT_SERVICE_H_
#define BLUETOOTH_BLUETOOTH_GATT_SERVICE_H_

#include <map>

#include <bluetooth.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace bluetooth {

class BluetoothInstance;

class BluetoothGATTService {
 public:
  BluetoothGATTService(BluetoothInstance& instance);
  ~BluetoothGATTService();

  common::PlatformResult GetSpecifiedGATTService(const std::string &address,
                                                 const std::string &uuid,
                                                 picojson::object* result);

  void TryDestroyClient(const std::string &address);
  void GetServices(const picojson::value& data, picojson::object& out);
  void GetCharacteristics(const picojson::value& data, picojson::object& out);
  void ReadValue(const picojson::value& args, picojson::object& out);
  void WriteValue(const picojson::value& args, picojson::object& out);
  void AddValueChangeListener(const picojson::value& args,
                              picojson::object& out);
  void RemoveValueChangeListener(const picojson::value& args,
                                 picojson::object& out);

  common::PlatformResult GetServiceUuids(const std::string& address,
                                         picojson::array* array);

 private:
  bool IsStillConnected(const std::string& address);

  bt_gatt_client_h GetGattClient(const std::string& address);

  common::PlatformResult GetServicesHelper(bt_gatt_h handle, const std::string& address,
                                                  picojson::array* array);
  common::PlatformResult GetCharacteristicsHelper(bt_gatt_h handle,
                                                  const std::string& address,
                                                  const std::string& uuid,
                                                  picojson::array* array);

  static void OnCharacteristicValueChanged(bt_gatt_h characteristic,
                                           char* value, int len,
                                           void* user_data);

  std::map<std::string, bt_gatt_client_h> gatt_clients_;
  std::vector<bt_gatt_h> gatt_characteristic_;

  BluetoothInstance& instance_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_GATT_SERVICE_H_
