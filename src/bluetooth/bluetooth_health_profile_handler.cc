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

#include "bluetooth_health_profile_handler.h"

#include "common/converter.h"
#include "common/logger.h"
#include "common/extension.h"
#include "common/task-queue.h"

#include "bluetooth/bluetooth_adapter.h"
#include "bluetooth/bluetooth_instance.h"
#include "bluetooth/bluetooth_health_application.h"
#include "bluetooth/bluetooth_health_channel.h"
#include "bluetooth/bluetooth_util.h"

namespace extension {
namespace bluetooth {

using namespace common;
using namespace common::tools;

namespace {
const std::string kId = "id";
const std::string kEvent = "event";
const std::string kData = "data";
const std::string kOnConnect = "onconnect";
const std::string kOnClose = "onclose";
const std::string kOnMessage = "onmessage";
const std::string kChangeCallback = "BluetoothHealthChannelChangeCallback";
} //namespace

BluetoothHealthProfileHandler::BluetoothHealthProfileHandler(BluetoothInstance& instance)
    : instance_(instance) {
  // initialize listeners
  LoggerD("Entered");
  if (BT_ERROR_NONE != bt_hdp_set_connection_state_changed_cb(OnConnected, OnDisconnected, this)) {
    LoggerE("bt_hdp_set_connection_state_changed_cb() failed");
  }

  if (BT_ERROR_NONE != bt_hdp_set_data_received_cb(OnDataReceived, this)) {
    LoggerE("bt_hdp_set_data_received_cb() failed");
  }
}

BluetoothHealthProfileHandler::~BluetoothHealthProfileHandler() {
  LoggerD("Entered");
  bt_hdp_unset_connection_state_changed_cb();
  bt_hdp_unset_data_received_cb();

  for (auto& it : registered_health_apps_) {
    bt_hdp_unregister_sink_app(it.c_str());
  }
}

void BluetoothHealthProfileHandler::OnConnected(int result,
                                                const char* remote_address,
                                                const char* app_id,
                                                bt_hdp_channel_type_e type,
                                                unsigned int channel,
                                                void* user_data) {
  LoggerD("Entered");

  BluetoothHealthProfileHandler* object = static_cast<BluetoothHealthProfileHandler*>(user_data);

  if (!object) {
    LoggerW("user_data is NULL");
    return;
  }

  if (BT_ERROR_NONE != result) {
    LoggerD("Not BT_ERROR_NONE: %d", result);
  }

  LoggerD("Connected app: %s", app_id);
  LoggerD("Connected channel: %u", channel);

  const auto iter = object->registered_health_apps_.find(app_id);

  if (iter == object->registered_health_apps_.end()) {
    LoggerW("This app is not registered: %s", app_id);
    return;
  }

  bool channel_added = false;

  if (BT_ERROR_NONE == result) {
    // send BluetoothHealthApplication.onconnect notification
    bt_device_info_s* device_info = nullptr;

    if (BT_ERROR_NONE == bt_adapter_get_bonded_device_info(remote_address, &device_info) &&
        nullptr != device_info) {
      LoggerD("invoke BluetoothHealthApplication.onconnect");

      object->connected_channels_.insert(channel);
      channel_added = true;

      picojson::value response{picojson::object()};
      picojson::object& response_obj = response.get<picojson::object>();
      response_obj.insert(std::make_pair(kId, picojson::value(app_id)));
      response_obj.insert(std::make_pair(kEvent, picojson::value(kOnConnect)));

      picojson::value result = picojson::value(picojson::object());

      BluetoothHealthChannel::ToJson(channel,
                                     type,
                                     device_info,
                                     app_id,
                                     &result.get<picojson::object>());

      ReportSuccess(result, response_obj);
      bt_adapter_free_device_info(device_info);

      object->instance_.FireEvent("BLUETOOTH_HEALTH_APPLICATION_CHANGED", response);
    } else {
      LoggerE("Failed to get device info");
    }
  }

  // Handle requests from  BluetoothHealthProfileHandler.connectToSource()
  const auto request = object->connection_requests_.find(app_id);

  if (request != object->connection_requests_.end()) {
    LoggerD("Requested connection");

    std::shared_ptr<picojson::value> response{new picojson::value(picojson::object())};

    if (BT_ERROR_NONE == result) {
      if (!channel_added) {
        object->connected_channels_.insert(channel);
      }

      picojson::value result = picojson::value(picojson::object());

      BluetoothHealthChannel::ToJson(channel,
                                     type,
                                     &result.get<picojson::object>());

      ReportSuccess(result, response->get<picojson::object>());
    } else {
      LoggerE("Failed to establish a connection with health profile");
      ReportError(PlatformResult(
          ErrorCode::UNKNOWN_ERR, "Failed to establish a connection with health profile"),
                  &response->get<picojson::object>());
    }

    object->instance_.AsyncResponse(request->second, response);

    // request was handled, remove
    object->connection_requests_.erase(request);
  }
  else {
    LoggerD("This connection was not requested.");
  }
}

void BluetoothHealthProfileHandler::OnDisconnected(int result,
                                                   const char* /* remote_address */,
                                                   unsigned int channel,
                                                   void* user_data) {
  LoggerD("Entered");

  BluetoothHealthProfileHandler* object = static_cast<BluetoothHealthProfileHandler*>(user_data);

  if (!object) {
    LoggerE("user_data is NULL");
    return;
  }

  auto it = object->connected_channels_.find(channel);
  if (BT_ERROR_NONE == result && object->connected_channels_.end() != it) {
    object->connected_channels_.erase(it);
    picojson::value value = picojson::value(picojson::object());
    picojson::object* data_obj = &value.get<picojson::object>();

    data_obj->insert(std::make_pair(kEvent, picojson::value(kOnClose)));
    data_obj->insert(std::make_pair(kId, picojson::value(std::to_string(channel))));
    object->instance_.FireEvent(kChangeCallback, value);
  }
}

void BluetoothHealthProfileHandler::OnDataReceived(unsigned int channel,
                                                   const char* data,
                                                   unsigned int size,
                                                   void* user_data) {
  LoggerD("Entered");

  BluetoothHealthProfileHandler* object = static_cast<BluetoothHealthProfileHandler*>(user_data);

  if (!object) {
    LoggerE("user_data is NULL");
    return;
  }

  auto it = object->connected_channels_.find(channel);
  if (object->connected_channels_.end() != it) {
    picojson::value value = picojson::value(picojson::object());
    picojson::object* data_obj = &value.get<picojson::object>();

    data_obj->insert(std::make_pair(kEvent, picojson::value(kOnMessage)));
    data_obj->insert(std::make_pair(kId, picojson::value(std::to_string(channel))));

    picojson::array& array = data_obj->insert(std::make_pair(kData, picojson::value(
        picojson::array()))).first->second.get<picojson::array>();

    for (unsigned int i = 0; i < size; i++) {
      array.push_back(picojson::value(static_cast<double>(data[i])));
    }

    object->instance_.FireEvent(kChangeCallback, value);
  }
}

void BluetoothHealthProfileHandler::RegisterSinkApp(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);
  const auto data_type = static_cast<short>(FromJson<double>(args, "dataType"));
  const auto& name = FromJson<std::string>(args, "name");

  const auto callback_handle = util::GetAsyncCallbackHandle(data);

  auto register_app = [data_type, name, this](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Entered");

    PlatformResult platform_result = PlatformResult(ErrorCode::NO_ERROR);
    char* app_id = nullptr;
    const int ret = bt_hdp_register_sink_app(data_type, &app_id);

    switch (ret) {
      case BT_ERROR_NONE:
      {
        LoggerD("Registered app: %s", app_id);

        this->registered_health_apps_.insert(app_id);

        picojson::value result = picojson::value(picojson::object());
        BluetoothHealthApplication::ToJson(data_type,
                                           name,
                                           app_id,
                                           &result.get<picojson::object>());
        ReportSuccess(result, response->get<picojson::object>());
        return;
      }

      case BT_ERROR_NOT_ENABLED:
        LoggerE("Bluetooth device is turned off");
        platform_result = PlatformResult(
            ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
        break;

      default:
        LoggerE("bt_hdp_register_sink_app() failed: %d", ret);
        platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
        break;
    }

    ReportError(platform_result, &response->get<picojson::object>());
  };

  auto register_app_response = [this, callback_handle](const std::shared_ptr<picojson::value>& response) -> void {
    instance_.SyncResponse(callback_handle, response);
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      register_app,
      register_app_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));

  ReportSuccess(out);
}

void BluetoothHealthProfileHandler::ConnectToSource(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  const auto& args = util::GetArguments(data);
  const auto& address = FromJson<std::string>(args, "address");
  const auto& app_id = FromJson<std::string>(args, "appId");

  LoggerD("address: %s", address.c_str());
  LoggerD("app ID: %s", app_id.c_str());

  const auto callback_handle = util::GetAsyncCallbackHandle(data);

  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
  const int ret = bt_hdp_connect_to_source(address.c_str(), app_id.c_str());

  switch (ret) {
    case BT_ERROR_NONE: {
      LoggerD("bt_hdp_connect_to_source() succeeded");

      connection_requests_.insert(std::make_pair(app_id, callback_handle));
      break;
    }

    case BT_ERROR_NOT_ENABLED:
      result = PlatformResult(
          ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
      break;

    case BT_ERROR_INVALID_PARAMETER:
    case BT_ERROR_REMOTE_DEVICE_NOT_BONDED:
      result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid value");
      break;

    default:
      result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
      break;
  }

  if (result.IsError()) {
    instance_.AsyncResponse(callback_handle, result);
  }

  ReportSuccess(out);
}

void BluetoothHealthProfileHandler::UnregisterSinkAppAsync(const std::string& app_id,
                                                           int callback_handle) {
  LoggerD("Entered");

  auto unregister_app = [app_id, this](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Entered");

    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
    auto iter = this->registered_health_apps_.find(app_id);

    if (iter != this->registered_health_apps_.end()) {
      LoggerD("Found registered Health Application: %s", app_id.c_str());

      const int ret = bt_hdp_unregister_sink_app(app_id.c_str());

      switch(ret) {
        case BT_ERROR_NONE:
          this->registered_health_apps_.erase(iter);
          break;

        case BT_ERROR_NOT_ENABLED:
          LoggerE("Bluetooth device is turned off");
          result = PlatformResult(
              ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
          break;

        default:
          LoggerE("bt_hdp_unregister_sink_app() failed: %d", ret);
          result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
          break;
      }
    } else {
      LoggerD("Already unregistered");
    }

    if (result.IsSuccess()) {
      ReportSuccess(response->get<picojson::object>());
    } else {
      ReportError(result, &response->get<picojson::object>());
    }
  };

  auto unregister_app_response = [this, callback_handle](const std::shared_ptr<picojson::value>& response) -> void {
    instance_.SyncResponse(callback_handle, response);
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      unregister_app,
      unregister_app_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

} // namespace bluetooth
} // namespace extension
