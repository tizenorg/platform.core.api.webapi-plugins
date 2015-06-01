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

#include "bluetooth_socket.h"

#include <memory>

#include "common/converter.h"
#include "common/logger.h"
#include "common/extension.h"

#include "bluetooth_adapter.h"
#include "bluetooth_device.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

namespace {
const std::string kBluetoothSocketId = "id";
const std::string kBluetoothSocketUuid = "uuid";
const std::string kBluetoothSocketState = "state";
const std::string kBluetoothSocketPeer = "peer";
const std::string kBluetoothSocketStateOpen = "OPEN";
// error
const int kBluetoothError = -1;
}

using namespace common;
using namespace common::tools;

BluetoothSocket::BluetoothSocket(BluetoothAdapter& adapter)
    : adapter_(adapter) {
}

void BluetoothSocket::WriteData(const picojson::value& data, picojson::object& out) {
  LoggerD("Enter");

  const auto& args = util::GetArguments(data);

  int socket = common::stol(FromJson<std::string>(args, "id"));
  const auto& binary_data = FromJson<picojson::array>(args, "data");
  const auto data_size = binary_data.size();

  std::unique_ptr<char[]> data_ptr{new char[data_size]};

  for (std::size_t i = 0; i < data_size; ++i) {
    data_ptr[i] = static_cast<char>(binary_data[i].get<double>());
  }

  if (kBluetoothError == bt_socket_send_data(socket, data_ptr.get(), data_size)) {
    LoggerE("bt_socket_send_data() failed");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error"), &out);
    return;
  }

  ReportSuccess(picojson::value(static_cast<double>(data_size)), out);
}

void BluetoothSocket::ReadData(const picojson::value& data, picojson::object& out) {
  LoggerD("Enter");

  const auto& args = util::GetArguments(data);

  int socket = common::stol(FromJson<std::string>(args, "id"));

  auto binary_data = adapter_.ReadSocketData(socket);
  picojson::value ret = picojson::value(picojson::array());
  picojson::array& array = ret.get<picojson::array>();

  for (auto val : binary_data) {
    array.push_back(picojson::value(static_cast<double>(val)));
  }

  adapter_.ClearSocketData(socket);

  ReportSuccess(ret, out);
}

void BluetoothSocket::Close(const picojson::value& data, picojson::object& out) {
  LoggerD("Enter");

  const auto& args = util::GetArguments(data);

  int socket = common::stol(FromJson<std::string>(args, "id"));

  if (BT_ERROR_NONE != bt_socket_disconnect_rfcomm(socket)) {
    LoggerE("bt_socket_disconnect_rfcomm() failed");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error"), &out);
    return;
  }

  ReportSuccess(out);
}

picojson::value BluetoothSocket::ToJson(bt_socket_connection_s* connection) {
  LoggerD("Enter");

  picojson::value ret = picojson::value(picojson::object());
  auto& ret_obj = ret.get<picojson::object>();

  ret_obj.insert(std::make_pair(kBluetoothSocketId,
                                picojson::value(std::to_string(connection->socket_fd))));
  ret_obj.insert(std::make_pair(kBluetoothSocketUuid,
                                picojson::value(connection->service_uuid)));
  ret_obj.insert(std::make_pair(kBluetoothSocketState,
                                picojson::value(kBluetoothSocketStateOpen)));

  bt_device_info_s* device_info = nullptr;

  if (BT_ERROR_NONE == bt_adapter_get_bonded_device_info(connection->remote_address, &device_info) &&
      nullptr != device_info) {
    picojson::value& device = ret_obj.insert(std::make_pair(
        kBluetoothSocketPeer, picojson::value(picojson::object()))).first->second;

    BluetoothDevice::ToJson(device_info, &device.get<picojson::object>());
    bt_adapter_free_device_info(device_info);
  } else {
    LoggerE("Peer not found");
  }

  return ret;
}

} // namespace bluetooth
} // namespace extension
