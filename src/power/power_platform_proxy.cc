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

#include "power_platform_proxy.h"

#include <algorithm>

#include "common/logger.h"

using namespace common;

namespace extension {
namespace power {

PowerPlatformProxy::PowerPlatformProxy()
    : gdbus_op_(GDBusWrapper::DEFAULT_BUS_NAME,
                GDBusWrapper::DEFAULT_OBJECT_PATH) {
  LoggerD("Entered");
}

PowerPlatformProxy::~PowerPlatformProxy() { LoggerD("Entered"); }

PowerPlatformProxy& PowerPlatformProxy::GetInstance() {
  LoggerD("Entered");
  static PowerPlatformProxy instance;
  return instance;
}

common::PlatformResult PowerPlatformProxy::LockState(int* result) {
  if (false ==
      gdbus_op_.LockState("lcddim", "staycurstate", "NULL", 0, result)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get reply from gdbus");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PowerPlatformProxy::UnlockState(int* result) {
  LoggerD("Entered");
  if (false == gdbus_op_.UnlockState("lcddim", "keeptimer", result)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get reply from gdbus");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PowerPlatformProxy::SetBrightnessFromSettings(
    int* result) {
  LoggerD("Entered");
  if (false == gdbus_op_.ReleaseBrightness(result)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get reply from gdbus");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PowerPlatformProxy::SetBrightness(int val, int* result) {
  LoggerD("Entered");
  if (false == gdbus_op_.HoldBrightness(val, result)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get reply from gdbus");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PowerPlatformProxy::GetBrightness(int* result) {
  LoggerD("Entered");
  if (false == gdbus_op_.CurrentBrightness(result)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get reply from gdbus");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PowerPlatformProxy::IsCustomBrightness(int* result) {
  LoggerD("Entered");
  if (false == gdbus_op_.CustomBrightness(result)) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get reply from gdbus");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace power
}  // namespace extension
