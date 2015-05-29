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

#ifndef BLUETOOTH_BLUETOOTH_ADAPTER_H_
#define BLUETOOTH_BLUETOOTH_ADAPTER_H_

#include <set>
#include <list>
#include <memory>
#include <unordered_map>
#include <map>

#include <bluetooth.h>
#include "bluetooth_internal.h"

#include "common/picojson.h"

namespace extension {
namespace bluetooth {

enum AdapterAsyncEvent {
  SET_POWERED = 0,
  SET_NAME,
  SET_VISIBLE,
  DISCOVER_DEVICES,
  STOP_DISCOVERY
};

class BluetoothInstance;

class BluetoothAdapter {
 public:
  /**
   * Signature: @code void setName(name, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_setName', args: {name: name}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void SetName(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void setPowered(state, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_setPowered', args: {state: state}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void SetPowered(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void setVisible(mode, successCallback, errorCallback, timeout); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_setVisible', args: {mode: mode, timeout: timeout}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void SetVisible(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void discoverDevices(discoveryCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_discoverDevices', args: {discoveryCallbackId: id}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Discovery callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {event, eventData}}
   * @endcode
   */
  void DiscoverDevices(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void stopDiscovery(successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_stopDiscovery', args: {}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void StopDiscovery(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void getKnownDevices(successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getKnownDevices', args: {}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {devices[]}}
   * @endcode
   */
  void GetKnownDevices(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void getDevice(address, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getDevice', args: {address: address}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {device}}
   * @endcode
   */
  void GetDevice(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void createBonding(address, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_createBonding', args: {address: address}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {device}}
   * @endcode
   */
  void CreateBonding(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void destroyBonding(address, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_destroyBonding', args: {address: address}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void DestroyBonding(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code void registerRFCOMMServiceByUUID(uuid, name, successCallback, errorCallback); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_registerRFCOMMServiceByUUID',
   *                    args: {uuid: uuid, name: name}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {handler}}
   * @endcode
   */
  void RegisterRFCOMMServiceByUUID(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothProfileHandler getBluetoothProfileHandler(profileType); @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getBluetoothProfileHandler',
   *                    args: {profileType: profileType}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {BluetoothProfileHandler}}
   * @endcode
   */
  void GetBluetoothProfileHandler(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothAdapter.name; @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getName', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: name}
   * @endcode
   */
  void GetName(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothAdapter.address; @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getAddress', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: address}
   * @endcode
   */
  void GetAddress(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothAdapter.powered; @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getPowered', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: powered}
   * @endcode
   */
  void GetPowered(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothAdapter.visible; @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_getVisible', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: visible}
   * @endcode
   */
  void GetVisible(const picojson::value& data, picojson::object& out);

  /**
   * Signature: @code BluetoothServiceHandler.isConnected; @endcode
   * JSON: @code data: {method: 'BluetoothAdapter_isConnected', args: {}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: isConnected}
   * @endcode
   */
  void IsServiceConnected(const picojson::value& data, picojson::object& out);

  explicit BluetoothAdapter(BluetoothInstance& instance);
  virtual ~BluetoothAdapter();

  std::string get_name() const;
  bool get_visible() const;
  void set_visible(bool visible);
  bool get_powered();
  void set_powered(bool powered);
  bool is_initialized() const;

  void ConnectToServiceByUUID(const std::string& address,
                              const std::string& uuid,
                              double callback_handle);

  const std::list<char>& ReadSocketData(int socket);

  void ClearSocketData(int socket);

  void UnregisterUUID(const std::string& uuid, int callback_handle);

 private:
  BluetoothAdapter(const BluetoothAdapter&) = delete;
  BluetoothAdapter& operator=(const BluetoothAdapter&) = delete;

  static void StateChangedCB(int result,
                             bt_adapter_state_e state,
                             void *user_data);
  static void NameChangedCB(char *name,
                            void *user_data);
  static void VisibilityChangedCB(int result,
                                  bt_adapter_visibility_mode_e mode,
                                  void *user_data);
  static void DiscoveryStateChangedCB(int result,
                                      bt_adapter_device_discovery_state_e discovery_state,
                                      bt_adapter_device_discovery_info_s *discovery_info,
                                      void *user_data);

  void StoreSocketData(bt_socket_received_data_s* data);

  void RemoveSocket(int socket);

  static void OnSocketConnected(int result,
                                bt_socket_connection_state_e state,
                                bt_socket_connection_s* connection,
                                void* user_data);

  static void OnSocketReceivedData(bt_socket_received_data_s* data,
                                   void* user_data);

  void InvokeSocketEvent(int id, const char* event);
  void InvokeSocketOnMessageEvent(int id);
  void InvokeSocketOnCloseEvent(int id);

  bool is_visible_;
  bool is_powered_;
  bool is_initialized_;
  bool user_request_list_ [STOP_DISCOVERY + 1];
  double user_request_callback_ [STOP_DISCOVERY + 1];

  bool requested_powered_;
  bt_adapter_visibility_mode_e requested_visibility_;
  std::string requested_name_;

  picojson::array discovered_devices_;
  std::set<std::string> discovered_addresses_;
  std::set<std::string> disappeared_addresses_;

  struct ConnectionRequest {
    std::string uuid_;
    double callback_handle_;
  };

  typedef std::shared_ptr<ConnectionRequest> ConnectionRequestPtr;
  typedef std::multimap<std::string, ConnectionRequestPtr> ConnectionRequestMap;

  ConnectionRequestMap connection_requests_;

  typedef std::list<int> ConnectedSocketList;

  ConnectedSocketList connected_sockets_;

  typedef std::pair<int, bool> BluetoothServicePair; //registered socket - connection state
  typedef std::map<std::string, BluetoothServicePair> RegisteredUuidMap;

  RegisteredUuidMap registered_uuids_;

  std::unordered_map<int, std::list<char>> socket_data_;

  BluetoothInstance& instance_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_ADAPTER_H_
