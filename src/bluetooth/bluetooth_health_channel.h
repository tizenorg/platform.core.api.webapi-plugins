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

#ifndef BLUETOOTH_BLUETOOTH_HEALTH_CHANNEL_H_
#define BLUETOOTH_BLUETOOTH_HEALTH_CHANNEL_H_

#include <bluetooth.h>

#include "common/picojson.h"

namespace extension {
namespace bluetooth {

class BluetoothHealthChannel {
 public:
  /**
   * Signature: @code void close(); @endcode
   * JSON: @code data: {method: 'BluetoothHealthChannel_close', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void Close(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code unsigned long sendData(data[]); @endcode
   * JSON: @code data: {method: 'BluetoothHealthChannel_sendData', args: {data: data}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {bytes_sent}}
   */
  void SendData(const picojson::value& data, picojson::object& out);

  static void ToJson(unsigned int channel,
                     bt_hdp_channel_type_e type,
                     picojson::object* out);

  static void ToJson(unsigned int channel,
                     bt_hdp_channel_type_e type,
                     bt_device_info_s* device_info,
                     const char* app_id,
                     picojson::object* out);
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_HEALTH_CHANNEL_H_
