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

#include "bluetooth/bluetooth_le_device.h"
#include <glib.h>

#include "bluetooth/bluetooth_instance.h"
#include "bluetooth/bluetooth_util.h"

#include "common/converter.h"
#include "common/logger.h"
#include "common/tools.h"

using common::ErrorCode;
using common::PlatformResult;
using common::tools::ReportError;
using common::tools::ReportSuccess;
using common::tools::BinToHex;

namespace extension {
namespace bluetooth {

namespace {
//le_device
const std::string kDeviceName = "name";
const std::string kDeviceAddress = "address";
const std::string kTxPowerLevel = "txpowerlevel";
const std::string kAppearance = "appearance";
const std::string kDeviceUuids = "uuids";
const std::string kSolicitationUuids = "solicitationuuids";
const std::string kServiceData = "serviceData";
const std::string kServiceUuid = "uuid";
const std::string kManufacturerData = "manufacturerData";
const std::string kId = "id";
const std::string kData = "data";
const std::string kAction = "action";

const std::string kOnConnected = "onconnected";
const std::string kOnDisconnected = "ondisconnected";
const std::string kConnectChangeEvent = "BluetoothLEConnectChangeCallback";

}

BluetoothLEDevice::BluetoothLEDevice(BluetoothInstance& instance,
                                     BluetoothGATTService& service)
    : instance_(instance),
      service_(service),
      is_listener_set_(false) {
  LoggerD("Entered");
  int ret = bt_gatt_set_connection_state_changed_cb(GattConnectionState, this);
  if (BT_ERROR_NONE != ret && BT_ERROR_ALREADY_DONE != ret) {
    LoggerE("Can't add connection state listener: %d", ret);
  }
}

BluetoothLEDevice::~BluetoothLEDevice() {
  LoggerD("Entered");
  int ret = bt_gatt_unset_connection_state_changed_cb();
  if (ret != BT_ERROR_NONE) {
    LoggerW("Failed to unset listener: %d", ret);
  }
}

static void UUIDsToJson(char **service_uuid, int service_count,
                        const std::string &field, picojson::object* le_device) {
  LoggerD("Entered");

  picojson::array& array = le_device->insert(
      std::make_pair(field, picojson::value(picojson::array()))).first->second
      .get<picojson::array>();

  for (int i = 0; i < service_count; i++) {
    array.push_back(picojson::value(service_uuid[i]));
  }
}

static void ServiceDataToJson(bt_adapter_le_service_data_s *service_data_list,
                              int service_data_list_count,
                              picojson::object* le_device) {
  LoggerD("Entered");

  picojson::array& array = le_device->insert(
      std::make_pair(kServiceData, picojson::value(picojson::array()))).first
      ->second.get<picojson::array>();

  for (int i = 0; i < service_data_list_count; i++) {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj[kServiceUuid] = picojson::value(
        std::string(service_data_list[i].service_uuid));
    response_obj[kData] = picojson::value(
        std::string(service_data_list[i].service_data,
                    service_data_list[i].service_data_len));

    array.push_back(response);
  }
}

static void ManufacturerToJson(int manufacturer_id,
                               char *manufacturer_data,
                               int manufacturer_count,
                               picojson::object* le_device) {
  LoggerD("Entered");

  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  response_obj[kId] = picojson::value(std::to_string(manufacturer_id));

  const int hex_count = manufacturer_count * 2;
  char* manuf_data_hex = new char[hex_count + 1];
  BinToHex((const unsigned char*) manufacturer_data,
           manufacturer_count,
           manuf_data_hex,
           hex_count);
  manuf_data_hex[hex_count] = '\0';
  response_obj[kData] = picojson::value(std::string(manuf_data_hex));
  delete [] manuf_data_hex;
  manuf_data_hex = nullptr;

  le_device->insert(std::make_pair(kManufacturerData, response));
}

PlatformResult BluetoothLEDevice::ToJson(
    bt_adapter_le_device_scan_result_info_s* info,
    picojson::object* le_device) {

  LoggerD("Entered");

  le_device->insert(
      std::make_pair(kDeviceAddress,
                     picojson::value(std::string(info->remote_address))));

  char *device_name = nullptr;

  int ret = BT_ERROR_NONE;

  std::vector<bt_adapter_le_packet_type_e> types = {
      BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, BT_ADAPTER_LE_PACKET_ADVERTISING };
  bool found = false;

  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_device_name(info, types[i],
                                                    &device_name);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get device name (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (found) {
    le_device->insert(
        std::make_pair(kDeviceName, picojson::value(std::string(device_name))));

    g_free(device_name);
  }

  int power_level = 0;
  found = false;
  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_tx_power_level(info, types[i],
                                                       &power_level);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get txpower (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (found) {
    le_device->insert(
        std::make_pair(kTxPowerLevel,
                       picojson::value(static_cast<double>(power_level))));
  }

  int appearance = 0;
  found = false;
  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_appearance(info, types[i], &appearance);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get appearance (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (found) {
    le_device->insert(
          std::make_pair(kAppearance,
                         picojson::value(static_cast<double>(appearance))));
  }

  char **uuids = nullptr;
  int count = 0;
  found = false;
  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_service_uuids(info, types[i], &uuids,
                                                      &count);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get uuids (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (found) {
    UUIDsToJson(uuids, count, kDeviceUuids, le_device);
    for (int i = 0; i < count; ++i) {
      g_free(uuids[i]);
    }
    g_free(uuids);
  }

  char** service_solicitation_uuids = nullptr;
  int service_solicitation_uuids_count = 0;
  found = false;

  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_service_solicitation_uuids(
        info, types[i], &service_solicitation_uuids,
        &service_solicitation_uuids_count);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get solicitation UUID (%d). Packet type: %d", ret,
              types[i]);
    }
  }

  if (found) {
    UUIDsToJson(service_solicitation_uuids, service_solicitation_uuids_count,
                kSolicitationUuids, le_device);
    for (int i = 0; i < service_solicitation_uuids_count; ++i) {
      g_free(service_solicitation_uuids[i]);
    }
    g_free(service_solicitation_uuids);
  }

  bt_adapter_le_service_data_s *serviceDataList = nullptr;
  int service_data_list_count = 0;
  found = false;

  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_service_data_list(
        info, types[i], &serviceDataList, &service_data_list_count);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get device service data (%d). Packet type: %d", ret,
              types[i]);
    }
  }

  if (found) {
    ServiceDataToJson(serviceDataList, service_data_list_count, le_device);
    ret = bt_adapter_le_free_service_data_list(serviceDataList,
                                               service_data_list_count);
    if (BT_ERROR_NONE != ret) {
      LoggerW("Failed to free service data list: %d", ret);
    }
  }

  int manufacturer_id = 0;
  char* manufacturer_data = nullptr;
  int manufacturer_data_count = 0;
  found = false;

  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_manufacturer_data(
        info, types[i], &manufacturer_id, &manufacturer_data,
        &manufacturer_data_count);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get device manufacturer (%d). Packet type: %d", ret,
              types[i]);
    }
  }

  if (found) {
    ManufacturerToJson(manufacturer_id, manufacturer_data,
                       manufacturer_data_count, le_device);
    g_free(manufacturer_data);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothLEDevice::Connect(const picojson::value& data,
                                picojson::object& out) {
  LoggerD("Entered");

  const auto callback_handle = util::GetAsyncCallbackHandle(data);
  const auto& args = util::GetArguments(data);

  const auto& address = common::FromJson<std::string>(args, "address");

  bool connected = false;
  int ret = bt_device_is_profile_connected(address.c_str(), BT_PROFILE_GATT, &connected);
  if (BT_ERROR_NONE != ret) {
    instance_.AsyncResponse(callback_handle,
                            PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to disconnect."));
    return;
  }

  if (connected) {
      instance_.AsyncResponse(callback_handle,
                              PlatformResult(ErrorCode::NO_ERROR));
  } else {  // not connected yet
    ret = bt_gatt_connect(address.c_str(), false);
    if (BT_ERROR_NONE != ret) {
      instance_.AsyncResponse(
          callback_handle,
          PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to connect."));
      return;
    }
    connecting_[address] = callback_handle;
  }

  ReportSuccess(out);
}

void BluetoothLEDevice::Disconnect(const picojson::value& data,
                                   picojson::object& out) {
  LoggerD("Entered");

  const auto callback_handle = util::GetAsyncCallbackHandle(data);
  const auto& args = util::GetArguments(data);
  const auto& address = common::FromJson<std::string>(args, "address");

  int ret = BT_ERROR_NONE;

  bool connected = false;
  ret = bt_device_is_profile_connected(address.c_str(), BT_PROFILE_GATT, &connected);
  if (BT_ERROR_NONE != ret) {
    instance_.AsyncResponse(
        callback_handle,
        PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to disconnect."));
    return;
  }
  if (!connected) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Bluetooth low energy device is not connected"),
                &out);
    return;
  }

  ret = bt_gatt_disconnect(address.c_str());
  if (BT_ERROR_NONE != ret) {
    instance_.AsyncResponse(
        callback_handle,
        PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to disconnect."));
    return;
  }

  connecting_[address] = callback_handle;

  ReportSuccess(out);
}

void BluetoothLEDevice::GetService(const picojson::value& data,
                                   picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);

  const auto& address = common::FromJson<std::string>(args, "address");
  const auto& uuid = common::FromJson<std::string>(args, "uuid");

  auto it = is_connected_.find(address);
  if (it == is_connected_.end()) {
    LoggerE("Bluetooth low energy device is not connected");
    ReportError(
        PlatformResult(ErrorCode::INVALID_STATE_ERR,
                       "Bluetooth low energy device is not connected"),
        &out);
    return;
  }

  picojson::value response = picojson::value(picojson::object());
  picojson::object *data_obj = &response.get<picojson::object>();

  PlatformResult result = service_.GetSpecifiedGATTService(address, uuid,
                                                           data_obj);

  if (result.IsError()) {
    ReportError(result, &out);
  } else {
    ReportSuccess(response, out);
  }
}

void BluetoothLEDevice::AddConnectStateChangeListener(
    const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  is_listener_set_ = true;

  ReportSuccess(out);
}

void BluetoothLEDevice::RemoveConnectStateChangeListener(
    const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  is_listener_set_ = false;

  ReportSuccess(out);
}

void BluetoothLEDevice::GetServiceUuids(const picojson::value& data,
                                        picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);
  const auto& address = common::FromJson<std::string>(args, "address");

  picojson::value response = picojson::value(picojson::array());
  picojson::array *data_obj = &response.get<picojson::array>();

  PlatformResult result = service_.GetServiceUuids(address, data_obj);

  if (result) {
    ReportSuccess(response, out);
  } else {
    ReportError(result, &out);
  }
}

void BluetoothLEDevice::GattConnectionState(int result, bool connected,
                                            const char* remote_address,
                                            void* user_data) {
  LoggerD("Entered: %s connected: %d", remote_address, connected);
  auto le_device = static_cast<BluetoothLEDevice *>(user_data);

  if (!le_device) {
    LoggerE("user_data is NULL");
    return;
  }

  if (connected) {
    le_device->is_connected_.insert(remote_address);
  } else {
    le_device->is_connected_.erase(remote_address);
    // inform that this device is not connected anymore
    le_device->service_.TryDestroyClient(remote_address);
  }

  if (le_device->is_listener_set_) {

    picojson::value value = picojson::value(picojson::object());
    picojson::object* data_obj = &value.get<picojson::object>();
    if (connected) {
      LoggerD("OnConnected");
      data_obj->insert(std::make_pair(kAction, picojson::value(kOnConnected)));
    } else {
      LoggerD("OnDisconnected");
      data_obj->insert(
          std::make_pair(kAction, picojson::value(kOnDisconnected)));
    }

    data_obj->insert(
        std::make_pair(kDeviceAddress, picojson::value(remote_address)));

    le_device->instance_.FireEvent(kConnectChangeEvent, value);
  }

  auto it = le_device->connecting_.find(remote_address);
  if (le_device->connecting_.end() == it) {
    LoggerW("Given address is not in waiting connections list");
    return;
  }

  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  if (BT_ERROR_NONE != result) {
    ret = PlatformResult(ErrorCode::UNKNOWN_ERR,
                         "Failed to get connection state");
  }

  le_device->instance_.AsyncResponse(it->second, ret);

  le_device->connecting_.erase(it);
}

}  // namespace bluetooth
}  // namespace extension
