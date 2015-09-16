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

#include "systeminfo-utils.h"
#include "systeminfo/systeminfo_instance.h"

#include <memory>

#include <net_connection.h>
#include <device.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/dbus_operation.h"
#include "common/scope_exit.h"

namespace extension {
namespace systeminfo {

namespace {
const int MEMORY_TO_BYTE = 1024;
}  // namespace

using common::PlatformResult;
using common::ErrorCode;

PlatformResult SysteminfoUtils::GetVconfInt(const char *key, int *value) {
  if (0 == vconf_get_int(key, value)) {
    LoggerD("value[%s]: %d", key, *value);
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  const std::string error_msg = "Could not get " + std::string(key);
  LoggerD("%s",error_msg.c_str());
  return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
}

PlatformResult SysteminfoUtils::GetRuntimeInfoString(system_settings_key_e key, std::string* platform_string) {
  char* platform_c_string;
  int err = system_settings_get_value_string(key, &platform_c_string);
  if (SYSTEM_SETTINGS_ERROR_NONE == err) {
    if (nullptr != platform_c_string) {
      *platform_string = platform_c_string;
      free(platform_c_string);
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  const std::string error_msg = "Error when retrieving system setting information: "
      + std::to_string(err);
  LoggerE("%s", error_msg.c_str());
  return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
}

PlatformResult SysteminfoUtils::CheckTelephonySupport() {
  bool supported = false;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
    "tizen.org/feature/network.telephony", &supported);
  if (ret.IsError()) {
    return ret;
  }
  if (!supported) {
    LoggerD("Telephony is not supported on this device");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR,
        "Telephony is not supported on this device");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::CheckCameraFlashSupport() {
  bool supported = false;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
    "tizen.org/feature/camera.back.flash", &supported);
  if (ret.IsError()) {
    return ret;
  }
  if (!supported) {
    LoggerD("Back-facing camera with a flash is not supported on this device");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR,
        "Back-facing camera with a flash is not supported on this device");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::CheckIfEthernetNetworkSupported() {
  LoggerD("Entered");
  connection_h connection_handle = nullptr;
  connection_ethernet_state_e connection_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;

  int error = connection_create(&connection_handle);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot create connection: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  std::unique_ptr<std::remove_pointer<connection_h>::type, int (*)(connection_h)> connection_handle_ptr(
    connection_handle, &connection_destroy);  // automatically release the memory

  error = connection_get_ethernet_state(connection_handle, &connection_state);
  if (CONNECTION_ERROR_NOT_SUPPORTED == error) {
    std::string log_msg = "Cannot get ethernet connection state: Not supported";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, log_msg);
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetTotalMemory(long long* result) {
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_total(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  *result = static_cast<long long>(value*MEMORY_TO_BYTE);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetAvailableMemory(long long* result) {
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_available(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  *result = static_cast<long long>(value*MEMORY_TO_BYTE);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::RegisterVconfCallback(const char *in_key, vconf_callback_fn cb,
                                                          void* event_ptr) {
  if (0 != vconf_notify_key_changed(in_key, cb, event_ptr)) {
    LoggerE("Failed to register vconf callback: %s", in_key);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register vconf callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb) {
  if (0 != vconf_ignore_key_changed(in_key, cb)) {
    LoggerE("Failed to unregister vconf callback: %s", in_key);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister vconf callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::RegisterTapiChangeCallback(TapiHandle *handle,
                                                           const char *noti_id,
                                                           tapi_notification_cb callback,
                                                           void *user_data) {
  if (TAPI_API_SUCCESS != tel_register_noti_event(handle, noti_id, callback, user_data)) {
    LoggerE("Failed to register tapi callback with key: %s", noti_id);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register tapi callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::UnregisterTapiChangeCallback(TapiHandle *handle,
                                                             const char *noti_id) {
  if (TAPI_API_SUCCESS != tel_deregister_noti_event(handle, noti_id)) {
    LoggerE("Failed to unregister tapi callback with key: %s", noti_id);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister tapi callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

} // namespace systeminfo
} // namespace webapi
