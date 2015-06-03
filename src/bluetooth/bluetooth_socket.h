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

#ifndef BLUETOOTH_BLUETOOTH_SOCKET_H__
#define BLUETOOTH_BLUETOOTH_SOCKET_H__

#include <bluetooth.h>

#include "common/picojson.h"

namespace extension {
namespace bluetooth {

class BluetoothAdapter;

class BluetoothSocket {
 public:
  explicit BluetoothSocket(BluetoothAdapter& adapter);

  /**
   * Signature: @code unsigned long writeData(data[]); @endcode
   * JSON: @code data: {method: 'BluetoothSocket_writeData', args: {data: data}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {bytes_sent}}
   * @endcode
   */
  void WriteData(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code byte[] readData(); @endcode
   * JSON: @code data: {method: 'BluetoothSocket_readData', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {bytes_read}}
   * @endcode
   */
  void ReadData(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void close(); @endcode
   * JSON: @code data: {method: 'BluetoothSocket_close', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void Close(const picojson::value& data, picojson::object& out);

  static picojson::value ToJson(bt_socket_connection_s* connection);

 private:
  BluetoothAdapter& adapter_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_SOCKET_H__
