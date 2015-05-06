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

#include "bluetooth/bluetooth_util.h"

#include "common/logger.h"

using common::PlatformResult;

namespace extension {
namespace bluetooth {

namespace {
//le_device
const std::string kDeviceName = "name";
const std::string kDeviceAddress = "address";
const std::string kTxPowerLevel = "txpowerLevel";
const std::string kAppearance = "appearance";
const std::string kDeviceUuids = "uuids";
const std::string kSolicitationUuids = "solicitationuuids";
const std::string kServiceData = "serviceData";
const std::string kServiceUuids = "serviceuuids";
const std::string kManufacturerData = "manufacturerData";
const std::string kId = "id";
const std::string kData = "data";

}

BluetoothLEDevice::BluetoothLEDevice(BluetoothInstance& instance)
    : instance_(instance) {
}

BluetoothLEDevice::~BluetoothLEDevice() {
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
    response_obj[kServiceUuids] = picojson::value(
        std::string(service_data_list[i].service_uuid));
    response_obj[kData] = picojson::value(
        std::string(service_data_list[i].service_data,
                    service_data_list[i].service_data_len));

    array.push_back(response);
  }
}

static void ManufacturerToJson(int manufacturer_id, char *manufacturer_data,
                               int manufacturer_count,
                               picojson::object* le_device) {
  LoggerD("Entered");

  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  response_obj[kId] = picojson::value(std::to_string(manufacturer_id));
  response_obj[kData] = picojson::value(
      std::string(manufacturer_data, manufacturer_count));
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

  std::vector<bt_adapter_le_packet_type_e> types = {BT_ADAPTER_LE_PACKET_SCAN_RESPONSE,
    BT_ADAPTER_LE_PACKET_ADVERTISING};
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

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get device name.");
  }

  le_device->insert(
      std::make_pair(kDeviceName, picojson::value(std::string(device_name))));

  g_free(device_name);

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

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get txpower.");
  }

  le_device->insert(
      std::make_pair(kTxPowerLevel,
                     picojson::value(static_cast<double>(power_level))));

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

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get appearance.");
  }

  le_device->insert(
      std::make_pair(kAppearance,
                     picojson::value(static_cast<double>(appearance))));

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

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get uuids.");
  }

  UUIDsToJson(uuids, count, kDeviceUuids, le_device);
  for (int i = 0; i < count; ++i) {
    g_free(uuids[i]);
  }
  g_free(uuids);

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
      LoggerE("Failed to get solicitation UUID (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get solicitation UUID.");
  }

  UUIDsToJson(service_solicitation_uuids, service_solicitation_uuids_count,
              kSolicitationUuids, le_device);
  for (int i = 0; i < service_solicitation_uuids_count; ++i) {
    g_free(service_solicitation_uuids[i]);
  }
  g_free(service_solicitation_uuids);

  bt_adapter_le_service_data_s *serviceDataList = nullptr;
  int service_data_list_count = 0;
  found = false;

  for (size_t i = 0; i < types.size() && !found; ++i) {
    ret = bt_adapter_le_get_scan_result_service_data_list(
        info, types[i], &serviceDataList, &service_data_list_count);
    if (BT_ERROR_NONE == ret) {
      found = true;
    } else {
      LoggerE("Failed to get device service data (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get device service data.");
  }

  ServiceDataToJson(serviceDataList, service_data_list_count, le_device);

  ret = bt_adapter_le_free_service_data_list(serviceDataList,
                                             service_data_list_count);
  if (BT_ERROR_NONE != ret) {
    LoggerW("Failed to free service data list: %d", ret);
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
      LoggerE("Failed to get device manufacturer (%d). Packet type: %d", ret, types[i]);
    }
  }

  if (!found) {
    return util::GetBluetoothError(ret, "Failed to get device manufacturer.");
  }

  ManufacturerToJson(manufacturer_id, manufacturer_data,
                     manufacturer_data_count, le_device);
  g_free(manufacturer_data);
  return common::PlatformResult(common::ErrorCode::NO_ERROR);
}

void BluetoothLEDevice::Connect(const picojson::value& data,
                                picojson::object& out) {
  LoggerD("Entered");
}

void BluetoothLEDevice::Disconnect(const picojson::value& data,
                                   picojson::object& out) {
  LoggerD("Entered");
}

void BluetoothLEDevice::GetService(const picojson::value& data,
                                   picojson::object& out) {
  LoggerD("Entered");
}

void BluetoothLEDevice::AddConnectStateChangeListener(
    const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
}

void BluetoothLEDevice::RemoveConnectStateChangeListener(
    const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
}

}  // namespace bluetooth
}  // namespace extension
