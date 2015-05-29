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

#include "bluetooth_device.h"

#include "common/converter.h"
#include "common/logger.h"
#include "common/extension.h"

#include "bluetooth_adapter.h"
#include "bluetooth_class.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

using namespace common;
using namespace common::tools;

namespace {
//device
const std::string kDeviceName = "name";
const std::string kDeviceAddress = "address";
const std::string kDeviceClass = "deviceClass";
const std::string kDeviceClassMajor = "major";
const std::string kDeviceClassMinor = "minor";
const std::string kDeviceClassService = "services";
const std::string kDeviceUuids = "uuids";
const std::string kDeviceIsBonded = "isBonded";
const std::string kDeviceIsTrusted = "isTrusted";
const std::string kDeviceIsConnected = "isConnected";
}

static void ToJsonFromBTClass(bt_class_s bluetooth_class, picojson::object* device) {
  LoggerD("Entered");

  picojson::object& bt = device->insert(std::make_pair(kDeviceClass, picojson::value(picojson::object())))
                                 .first->second.get<picojson::object>();

  bt.insert(std::make_pair(kDeviceClassMajor, picojson::value(static_cast<double>(
      BluetoothClass::GetMajorValue(bluetooth_class.major_device_class)))));
  bt.insert(std::make_pair(kDeviceClassMinor, picojson::value(static_cast<double>(
      BluetoothClass::GetMinorValue(bluetooth_class.minor_device_class)))));

  picojson::array& array = bt.insert(std::make_pair(
      kDeviceClassService, picojson::value(picojson::array()))).first->second.get<picojson::array>();

  std::vector<unsigned long> services_vector = BluetoothClass::getServiceValues(
      bluetooth_class.major_service_class_mask);

  for (auto v : services_vector) {
    array.push_back(picojson::value(static_cast<double>(v)));
  }
}

static void ToJsonFromUUID(char **service_uuid, int service_count, picojson::object* device) {
  LoggerD("Entered");

  picojson::array& array = device->insert(std::make_pair(kDeviceUuids, picojson::value(picojson::array())))
                             .first->second.get<picojson::array>();

  for (int i = 0; i < service_count; i++) {
    array.push_back(picojson::value(service_uuid[i]));
  }
}

BluetoothDevice::BluetoothDevice(BluetoothAdapter& adapter)
    : adapter_(adapter) {
}

void BluetoothDevice::ToJson(bt_device_info_s* info, picojson::object* device) {
  LoggerD("Entered");
  device->insert(std::make_pair(kDeviceName, picojson::value(std::string(info->remote_name))));
  device->insert(std::make_pair(kDeviceAddress, picojson::value(std::string(info->remote_address))));

  ToJsonFromBTClass(info->bt_class, device);
  ToJsonFromUUID(info->service_uuid, info->service_count, device);
}

void BluetoothDevice::ToJson(bt_adapter_device_discovery_info_s *info, picojson::object* device) {
  LoggerD("Entered");

  device->insert(std::make_pair(kDeviceName, picojson::value(info->remote_name)));
  device->insert(std::make_pair(kDeviceAddress, picojson::value(info->remote_address)));

  ToJsonFromBTClass(info->bt_class, device);
  ToJsonFromUUID(info->service_uuid, info->service_count, device);
}

void BluetoothDevice::ConnectToServiceByUUID(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);

  adapter_.ConnectToServiceByUUID(FromJson<std::string>(args, "address"),
                                  FromJson<std::string>(args, "uuid"),
                                  util::GetAsyncCallbackHandle(data));

  ReportSuccess(out);
}

void BluetoothDevice::GetBoolValue(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);
  const auto& address = FromJson<std::string>(args, "address");
  const auto& field = FromJson<std::string>(args, "field");

  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
  bool value = false;
  bt_device_info_s *info = nullptr;
  if (bt_adapter_get_bonded_device_info(address.c_str(), &info) == BT_ERROR_NONE &&
      info != nullptr) {
    if (kDeviceIsBonded == field) {
      value = info->is_bonded;
    } else if (kDeviceIsTrusted == field) {
      value = info->is_authorized;
    } else if (kDeviceIsConnected == field) {
      value = info->is_connected;
    } else {
      result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Wrong field passed.");
    }
    bt_adapter_free_device_info(info);
  } else {
    result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(value), out);
  } else {
    ReportError(result, &out);
  }
}

} // namespace bluetooth
} // namespace extension
