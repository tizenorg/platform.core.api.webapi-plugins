// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensor_service.h"

#include "common/logger.h"
#include "common/extension.h"

using namespace common;
using namespace common::tools;

namespace extension {
namespace sensor {

namespace {
static std::map<sensor_type_e, std::string> type_to_string_map = {
    {SENSOR_LIGHT, "LIGHT"},
    {SENSOR_MAGNETIC, "MAGNETIC"},
    {SENSOR_PRESSURE, "PRESSURE"},
    {SENSOR_PROXIMITY, "PROXIMITY"},
    {SENSOR_ULTRAVIOLET, "ULTRAVIOLET"}
};

static std::map<std::string, sensor_type_e> string_to_type_map = {
    {"LIGHT", SENSOR_LIGHT},
    {"MAGNETIC", SENSOR_MAGNETIC},
    {"PRESSURE", SENSOR_PRESSURE},
    {"PROXIMITY", SENSOR_PROXIMITY},
    {"ULTRAVIOLET", SENSOR_ULTRAVIOLET}
};
}

SensorService::SensorService() {

}

SensorService::~SensorService() {

}

SensorService* SensorService::GetInstance() {
  static SensorService instance_;
  return &instance_;
}

std::string SensorService::GetSensorErrorMessage(const int error_code) {
  LoggerD("Entered");

  switch (error_code) {
    case SENSOR_ERROR_IO_ERROR:
      return "IO error";
    case SENSOR_ERROR_INVALID_PARAMETER:
      return "Invalid parameter";
    case SENSOR_ERROR_NOT_SUPPORTED:
      return "Not supported";
    case SENSOR_ERROR_PERMISSION_DENIED:
      return "Permission denied";
    case SENSOR_ERROR_OUT_OF_MEMORY:
      return "Out of memory";
    case SENSOR_ERROR_NOT_NEED_CALIBRATION:
      return "Need calibration";
    case SENSOR_ERROR_OPERATION_FAILED:
      return "Operation failed";
    default:
      return "Unknown Error";
  }
}

PlatformResult SensorService::GetSensorPlatformResult(const int error_code, const std::string &hint) {
  LoggerD("Entered");

  std::string message = hint + " : " + GetSensorErrorMessage(error_code);

  switch (error_code) {
    case SENSOR_ERROR_NOT_SUPPORTED:
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, message);
    default:
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }
}

void SensorService::GetAvailableSensors(picojson::object& out) {
  LoggerD("Entered");

  bool is_supported = false;
  int ret = SENSOR_ERROR_NONE;

  picojson::value result = picojson::value(picojson::array());
  picojson::array& result_array = result.get<picojson::array>();

  for (auto it = type_to_string_map.begin(); it != type_to_string_map.end(); ++it) {
    ret = sensor_is_supported(it->first, &is_supported);
    if (SENSOR_ERROR_NONE != ret) {
      ReportError(GetSensorPlatformResult(ret, it->second), &out);
      return;
    }

    if (is_supported) {
      result_array.push_back(picojson::value(it->second));
    }
  }

  ReportSuccess(result, out);
}

} // namespace sensor
} // namespace extension
