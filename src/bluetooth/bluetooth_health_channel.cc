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

#include "bluetooth_health_channel.h"

#include <memory>

#include "common/converter.h"
#include "common/logger.h"
#include "common/extension.h"

#include "bluetooth_device.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

using namespace common;
using namespace common::tools;

namespace {
const std::string kPeer = "peer";
const std::string kChannelType = "channelType";
const std::string kApplication = "appId";
const std::string kIsConnected = "isConnected";
const std::string kId = "_id";
} // namespace

void BluetoothHealthChannel::Close(const picojson::value& data , picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);

  unsigned int channel = common::stol(FromJson<std::string>(args, "channel"));
  const auto& address = FromJson<std::string>(args, "address");

  if (BT_ERROR_NONE != bt_hdp_disconnect(address.c_str(), channel)) {
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error"), &out);
    return;
  }

  ReportSuccess(out);
}

void BluetoothHealthChannel::SendData(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);

  unsigned int channel = common::stol(FromJson<std::string>(args, "channel"));
  const auto& binary_data = FromJson<picojson::array>(args, "data");
  const auto data_size = binary_data.size();

  std::unique_ptr<char[]> data_ptr{new char[data_size]};

  for (std::size_t i = 0; i < data_size; ++i) {
    data_ptr[i] = static_cast<char>(binary_data[i].get<double>());
  }

  if (BT_ERROR_NONE != bt_hdp_send_data(channel, data_ptr.get(), data_size)) {
    LoggerE("bt_hdp_send_data() failed");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error"), &out);
    return;
  }

  ReportSuccess(picojson::value(static_cast<double>(data_size)), out);
}

void BluetoothHealthChannel::ToJson(unsigned int channel,
                                    bt_hdp_channel_type_e type,
                                    picojson::object* out) {
  LoggerD("Enter");
  const char* type_str = "UNKNOWN";

  switch (type) {
    case BT_HDP_CHANNEL_TYPE_RELIABLE:
      type_str = "RELIABLE";
      break;

    case BT_HDP_CHANNEL_TYPE_STREAMING:
      type_str = "STREAMING";
      break;

    default:
      LoggerE("Unknown HDP channel type: %d", type);
      break;
  }

  out->insert(std::make_pair(kId, picojson::value(std::to_string(channel))));
  out->insert(std::make_pair(kChannelType, picojson::value(type_str)));
  out->insert(std::make_pair(kIsConnected, picojson::value(true)));
}

void BluetoothHealthChannel::ToJson(unsigned int channel,
                                    bt_hdp_channel_type_e type,
                                    bt_device_info_s* device_info,
                                    const char* app_id,
                                    picojson::object* out) {
  LoggerD("Enter");
  ToJson(channel, type, out);
  auto& device = out->insert(
      std::make_pair(kPeer, picojson::value(picojson::object()))) .first->second.get<picojson::object>();

  BluetoothDevice::ToJson(device_info, &device);
  out->insert(std::make_pair(kApplication, picojson::value(app_id)));
}

} // namespace bluetooth
} // namespace extension
