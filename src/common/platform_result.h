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
 
#ifndef SRC_COMMON_PLATFORM_RESULT_H_
#define SRC_COMMON_PLATFORM_RESULT_H_

#include <string>

#include "common/picojson.h"

namespace common {

// WARNING! This list should be in sync with the equivalent list
// located at tizen/tizen_api.js. Remember to update tizen/tizen_api.js if you
// change something here.
enum class ErrorCode : int {
  // NO_ERROR is not really a valid error, but can be used
  // to indicate that no error occoured instead of having an
  // extra field in the message protocol just for that.
  NO_ERROR = 0,

  INDEX_SIZE_ERR = 1,
  DOMSTRING_SIZE_ERR = 2,
  HIERARCHY_REQUEST_ERR = 3,
  WRONG_DOCUMENT_ERR = 4,
  INVALID_CHARACTER_ERR = 5,
  NO_DATA_ALLOWED_ERR = 6,
  NO_MODIFICATION_ALLOWED_ERR = 7,
  NOT_FOUND_ERR = 8,
  NOT_SUPPORTED_ERR = 9,
  INUSE_ATTRIBUTE_ERR = 10,
  INVALID_STATE_ERR = 11,
  SYNTAX_ERR = 12,
  INVALID_MODIFICATION_ERR = 13,
  NAMESPACE_ERR = 14,
  INVALID_ACCESS_ERR = 15,
  VALIDATION_ERR = 16,
  TYPE_MISMATCH_ERR = 17,
  SECURITY_ERR = 18,
  NETWORK_ERR = 19,
  ABORT_ERR = 20,
  URL_MISMATCH_ERR = 21,
  QUOTA_EXCEEDED_ERR = 22,
  TIMEOUT_ERR = 23,
  INVALID_NODE_TYPE_ERR = 24,
  DATA_CLONE_ERR = 25,

  // Error codes for these errors are not really defined anywhere.
  INVALID_VALUES_ERR = 100,
  IO_ERR = 101,
  PERMISSION_DENIED_ERR = 102,
  SERVICE_NOT_AVAILABLE_ERR = 103,
  DATABASE_ERR = 104,

  // Error codes specific for plugins
  OPERATION_CANCELED_ERR = 200,

  UNKNOWN_ERR = -1
};

class PlatformResult {
 public:
  explicit PlatformResult(const ErrorCode& error_code,
                          const std::string& message = "");

  ErrorCode error_code() const { return error_code_; }
  std::string message() const { return message_; }

  bool IsSuccess() const { return error_code() == ErrorCode::NO_ERROR; }
  bool IsError() const { return !IsSuccess(); }
  explicit operator bool() const { return IsSuccess(); }

  picojson::value ToJSON() const;

 protected:
  ErrorCode error_code_;
  std::string message_;
};

}  // namespace common

#endif  // SRC_COMMON_PLATFORM_RESULT_H_
