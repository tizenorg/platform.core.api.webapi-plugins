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

#ifndef BLUETOOTH_BLUETOOTH_HEALTH_PROFILE_HANDLER_H_
#define BLUETOOTH_BLUETOOTH_HEALTH_PROFILE_HANDLER_H_

#include <set>

#include <bluetooth.h>

#include "common/picojson.h"

namespace extension {
namespace bluetooth {

class BluetoothInstance;

class BluetoothHealthProfileHandler {
 public:
  /**
   * Signature: @code void registerSinkApp(dataType, name, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothHealthProfileHandler_registerSinkApp',
   *                    args: {dataType: dataType, name: name}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {application}}
   * @endcode
   */
  void RegisterSinkApp(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void connectToSource(peer, application, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothHealthProfileHandler_connectToSource',
   *                    args: {peer: peer, application: application}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {channel}}
   * @endcode
   */
  void ConnectToSource(const picojson::value& data, picojson::object& out);

  explicit BluetoothHealthProfileHandler(BluetoothInstance& instance);
  ~BluetoothHealthProfileHandler();

  void UnregisterSinkAppAsync(const std::string& app_id, int callback_handle);

 private:
  BluetoothHealthProfileHandler(const BluetoothHealthProfileHandler&) = delete;
  BluetoothHealthProfileHandler& operator=(const BluetoothHealthProfileHandler&) = delete;

  static void OnConnected(int result,
                          const char* remote_address,
                          const char* app_id,
                          bt_hdp_channel_type_e type,
                          unsigned int channel,
                          void* user_data);

  static void OnDisconnected(int result,
                             const char* remote_address,
                             unsigned int channel,
                             void* user_data);

  static void OnDataReceived(unsigned int channel,
                             const char* data,
                             unsigned int size,
                             void* user_data);

  std::set<std::string> registered_health_apps_;
  std::map<std::string, double> connection_requests_;
  std::set<unsigned int> connected_channels_;

  BluetoothInstance& instance_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_HEALTH_PROFILE_HANDLER_H_
