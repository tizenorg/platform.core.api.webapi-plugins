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

#ifndef BLUETOOTH_BLUETOOTH_DEVICE_H_
#define BLUETOOTH_BLUETOOTH_DEVICE_H_

#include <bluetooth.h>

#include "common/picojson.h"

namespace extension {
namespace bluetooth {

class BluetoothAdapter;

class BluetoothDevice {
 public:
  explicit BluetoothDevice(BluetoothAdapter& adapter);

  /**
   * Signature: @code void connectToServiceByUUID(uuid, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothDevice_connectToServiceByUUID', args: {uuid: uuid}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {socket}}
   * @endcode
   */
  void ConnectToServiceByUUID(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothDevice.is(Bonded, Trusted, Connected); @endcode
   * JSON: @code data: {method: 'BluetoothDevice_GetBoolValue', args: {address: address,
   *                    field: field}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: name}
   * @endcode
   */
  void GetBoolValue(const picojson::value& data, picojson::object& out);

  static void ToJson(bt_device_info_s* info,
                     picojson::object* device);
  static void ToJson(bt_adapter_device_discovery_info_s *info,
                     picojson::object* device);

 private:
  BluetoothAdapter& adapter_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_DEVICE_H_
