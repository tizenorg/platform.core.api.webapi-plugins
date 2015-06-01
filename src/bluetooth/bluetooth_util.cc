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
#include "bluetooth/bluetooth_util.h"

#include <bluetooth.h>

namespace extension {
namespace bluetooth {
namespace util {

using common::ErrorCode;
using common::PlatformResult;

namespace {
const char* JSON_CALLBACK_ID = "callbackId";
} // namespace

double GetAsyncCallbackHandle(const picojson::value& data) {
  return data.get(JSON_CALLBACK_ID).get<double>();
}

const picojson::object& GetArguments(const picojson::value& data) {
  return data.get<picojson::object>();
}

PlatformResult GetBluetoothError(int error_code,
                                         const std::string& hint) {
  common::ErrorCode error = ErrorCode::UNKNOWN_ERR;

  switch (error_code) {
    case BT_ERROR_RESOURCE_BUSY:
    case BT_ERROR_NOW_IN_PROGRESS:
    case BT_ERROR_NOT_ENABLED:
      error = ErrorCode::SERVICE_NOT_AVAILABLE_ERR;
      break;

    case BT_ERROR_REMOTE_DEVICE_NOT_FOUND:
      error = ErrorCode::NOT_FOUND_ERR;
      break;

    case BT_ERROR_INVALID_PARAMETER:
      error = ErrorCode::INVALID_VALUES_ERR;
      break;

    case BT_ERROR_QUOTA_EXCEEDED:
       error = ErrorCode::QUOTA_EXCEEDED_ERR;
       break;

    default:
      error = ErrorCode::UNKNOWN_ERR;
      break;
  }

  return PlatformResult(error,
                        hint + " : " + GetBluetoothErrorMessage(error_code));
}

std::string GetBluetoothErrorMessage(int error_code) {
  switch (error_code) {
    case BT_ERROR_CANCELLED:
      return "Operation cancelled";
    case BT_ERROR_INVALID_PARAMETER:
      return "Invalid parameter";
    case BT_ERROR_OUT_OF_MEMORY:
      return "Out of memory";
    case BT_ERROR_RESOURCE_BUSY:
      return "Bluetooth device is busy";
    case BT_ERROR_TIMED_OUT:
      return "Timeout error";
    case BT_ERROR_NOW_IN_PROGRESS:
      return "Operation now in progress";
    case BT_ERROR_NOT_SUPPORTED:
      return "Not Supported";
    case BT_ERROR_PERMISSION_DENIED:
      return "Permission denied";
    case BT_ERROR_QUOTA_EXCEEDED:
      return "Quota exceeded";
    case BT_ERROR_NOT_INITIALIZED:
      return "Local adapter not initialized";
    case BT_ERROR_NOT_ENABLED:
      return "Local adapter not enabled";
    case BT_ERROR_ALREADY_DONE:
      return "Operation already done";
    case BT_ERROR_OPERATION_FAILED:
      return "Operation failed";
    case BT_ERROR_NOT_IN_PROGRESS:
      return "Operation not in progress";
    case BT_ERROR_REMOTE_DEVICE_NOT_BONDED:
      return "Remote device not bonded";
    case BT_ERROR_AUTH_REJECTED:
      return "Authentication rejected";
    case BT_ERROR_AUTH_FAILED:
      return "Authentication failed";
    case BT_ERROR_REMOTE_DEVICE_NOT_FOUND:
      return "Remote device not found";
    case BT_ERROR_SERVICE_SEARCH_FAILED:
      return "Service search failed";
    case BT_ERROR_REMOTE_DEVICE_NOT_CONNECTED:
      return "Remote device is not connected";
    case BT_ERROR_AGAIN:
      return "Resource temporarily unavailable";
    case BT_ERROR_SERVICE_NOT_FOUND:
      return "Service Not Found";
    default:
      return "Unknown Error";
  }
}

} // util
} // bluetooth
} // extension
