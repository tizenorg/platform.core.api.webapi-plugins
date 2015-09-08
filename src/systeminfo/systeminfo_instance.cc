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

#include "systeminfo/systeminfo_instance.h"

#include <device/led.h>
#include <functional>
#include <memory>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include "systeminfo-utils.h"
#include "systeminfo_device_capability.h"

namespace extension {
namespace systeminfo {

using common::PlatformResult;
using common::ErrorCode;
using common::TypeMismatchException;

namespace {
const std::string kPropertyIdString = "propertyId";
const std::string kListenerIdString = "listenerId";

#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }
}

SysteminfoInstance::SysteminfoInstance() : manager_(this) {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&SysteminfoInstance::x, this, _1, _2));
  REGISTER_SYNC("SystemInfo_getCapabilities", GetCapabilities);
  REGISTER_SYNC("SystemInfo_getCapability", GetCapability);
  REGISTER_SYNC("SystemInfo_addPropertyValueChangeListener", AddPropertyValueChangeListener);
  REGISTER_SYNC("SystemInfo_removePropertyValueChangeListener", RemovePropertyValueChangeListener);
  REGISTER_SYNC("SystemInfo_getTotalMemory", GetTotalMemory);
  REGISTER_SYNC("SystemInfo_getAvailableMemory", GetAvailableMemory);
  REGISTER_SYNC("SystemInfo_getCount", GetCount);
  REGISTER_SYNC("SystemInfo_setBrightness", SetBrightness);
  REGISTER_SYNC("SystemInfo_getBrightness", GetBrightness);
  REGISTER_SYNC("SystemInfo_getMaxBrightness", GetMaxBrightness);

#undef REGISTER_SYNC
#define REGISTER_ASYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&SysteminfoInstance::x, this, _1, _2));
  REGISTER_ASYNC("SystemInfo_getPropertyValue", GetPropertyValue);
  REGISTER_ASYNC("SystemInfo_getPropertyValueArray", GetPropertyValueArray);
#undef REGISTER_ASYNC
}

SysteminfoInstance::~SysteminfoInstance() {
  LoggerD("Entered");
}

void SysteminfoInstance::GetCapabilities(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.GetCapabilities(args, &out);
}

void SysteminfoInstance::GetCapability(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.GetCapability(args, &out);
}

void SysteminfoInstance::GetPropertyValue(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.GetPropertyValue(args, &out);
}

void SysteminfoInstance::GetPropertyValueArray(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.GetPropertyValueArray(args, &out);
}

void SysteminfoInstance::AddPropertyValueChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.AddPropertyValueChangeListener(args, &out);
}

void SysteminfoInstance::RemovePropertyValueChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.RemovePropertyValueChangeListener(args, &out);
}

void SysteminfoInstance::GetTotalMemory(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.GetTotalMemory(args, &out);
}

void SysteminfoInstance::GetAvailableMemory(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  manager_.GetAvailableMemory(args, &out);
}

void SysteminfoInstance::GetCount(const picojson::value& args, picojson::object& out) {

  LoggerD("Enter");
  manager_.GetCount(args, &out);
}

void SysteminfoInstance::SetBrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");

  CHECK_EXIST(args, "brightness", out)

  const double brightness = args.get("brightness").get<double>();
  int result = device_flash_set_brightness(brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    if (DEVICE_ERROR_INVALID_PARAMETER == result) {
      ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Error occured"), &out);
    } else {
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error occured"), &out);
    }
    return;
  }

  ReportSuccess(out);
}

void SysteminfoInstance::GetBrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");

  int brightness = 0;
  int result = device_flash_get_brightness(&brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error occured"), &out);
    return;
  }

  ReportSuccess(picojson::value(std::to_string(brightness)), out);
}

void SysteminfoInstance::GetMaxBrightness(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");

  int brightness = 0;
  int result = device_flash_get_max_brightness(&brightness);
  if (result != DEVICE_ERROR_NONE) {
    LoggerE("Error occured");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Not supported property"), &out);
    return;
  }
  ReportSuccess(picojson::value(std::to_string(brightness)), out);
}


} // namespace systeminfo
} // namespace extension
