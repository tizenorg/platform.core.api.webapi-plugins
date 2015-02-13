// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensor_service.h"

#include <string>
#include <memory>

#include "common/task-queue.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "sensor_instance.h"

using namespace common;
using namespace common::tools;

namespace extension {
namespace sensor {

namespace {
#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

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
  if (light_sensor_.listener) {
    sensor_destroy_listener(light_sensor_.listener);
  }
  if (magnetic_sensor_.listener) {
    sensor_destroy_listener(magnetic_sensor_.listener);
  }
  if (pressure_sensor_.listener) {
    sensor_destroy_listener(pressure_sensor_.listener);
  }
  if (proximity_sensor_.listener) {
    sensor_destroy_listener(proximity_sensor_.listener);
  }
  if (ultraviolet_sensor_.listener) {
    sensor_destroy_listener(ultraviolet_sensor_.listener);
  }
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

void SensorService::SensorStart(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "callbackId", out)
  int callback_id = static_cast<int>(args.get("callbackId").get<double>());
  const std::string type_str =
      args.contains("sensorType") ? args.get("sensorType").get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  auto start = [this, type_enum, type_str](const std::shared_ptr<picojson::value>& result) {
    PlatformResult res = CheckSensorInitialization(type_enum);
    if (res.IsError()) {
      LoggerE("Sensor initialization for sensor %s failed", type_str.c_str());
      ReportError(res, &(result->get<picojson::object>()));
      return;
    }
    SensorData* sensor_data = GetSensorStruct(type_enum);

    if (!sensor_data) {
      LoggerD("Sensor data is null");
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Sensor data is null"),
                         &(result->get<picojson::object>()));
      return;
    }

    int ret = sensor_listener_start(sensor_data->listener);
    if (ret != SENSOR_ERROR_NONE) {
      LoggerE("ret : %d", ret);
      ReportError(GetSensorPlatformResult(ret, "sensor_listener_start"),
                  &(result->get<picojson::object>()));
      return;
    }

    ReportSuccess(result->get<picojson::object>());
  };
  auto start_result = [callback_id](const std::shared_ptr<picojson::value>& result) {
    result->get<picojson::object>()["callbackId"] = picojson::value{static_cast<double>(callback_id)};
    SensorInstance::GetInstance().PostMessage(result->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>(
      start,
      start_result,
      std::shared_ptr<picojson::value>{new picojson::value{picojson::object()}});
  ReportSuccess(out);
}

void SensorService::SensorStop(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const std::string type_str =
      args.contains("sensorType") ? args.get("sensorType").get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  PlatformResult res = CheckSensorInitialization(type_enum);
  SensorData* sensor_data = GetSensorStruct(type_enum);
  //TODO fill
}

void SensorService::SensorSetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const std::string type_str =
      args.contains("sensorType") ? args.get("sensorType").get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  PlatformResult res = CheckSensorInitialization(type_enum);
  SensorData* sensor_data = GetSensorStruct(type_enum);
  //TODO fill
}

void SensorService::SensorUnsetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const std::string type_str =
      args.contains("sensorType") ? args.get("sensorType").get<std::string>() : "";
  LoggerD("input type: %s" , type_str.c_str());

  sensor_type_e type_enum = string_to_type_map[type_str];

  PlatformResult res = CheckSensorInitialization(type_enum);
  SensorData* sensor_data = GetSensorStruct(type_enum);
  //TODO fill
}

PlatformResult SensorService::CheckSensorInitialization(sensor_type_e type_enum) {
  LoggerD("Entered");

  SensorData* sensor_data = NULL;
  switch(type_enum) {
    case SENSOR_LIGHT :
      sensor_data = &light_sensor_;
      break;
    case SENSOR_MAGNETIC :
      sensor_data = &magnetic_sensor_;
      break;
    case SENSOR_PRESSURE :
      sensor_data = &pressure_sensor_;
      break;
    case SENSOR_PROXIMITY :
      sensor_data = &proximity_sensor_;
      break;
    case SENSOR_ULTRAVIOLET :
      sensor_data = &ultraviolet_sensor_;
      break;
  }
  if (!(sensor_data->handle)) {
    LoggerD("initialization of handle and listener");
    int ret = sensor_get_default_sensor(type_enum, &(sensor_data->handle));
    if (ret != SENSOR_ERROR_NONE) {
      LoggerE("ret : %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "sensor_get_default_sensor");
    }

    ret = sensor_create_listener(sensor_data->handle, &(sensor_data->listener));
    if (ret != SENSOR_ERROR_NONE) {
      LoggerE("ret : %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "sensor_create_listener");
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

SensorService::SensorData* SensorService::GetSensorStruct(sensor_type_e type_enum) {
  LoggerD("Entered");
  switch(type_enum) {
    case SENSOR_LIGHT :
      return &light_sensor_;
    case SENSOR_MAGNETIC :
      return &magnetic_sensor_;
    case SENSOR_PRESSURE :
      return &pressure_sensor_;
    case SENSOR_PROXIMITY :
      return &proximity_sensor_;
    case SENSOR_ULTRAVIOLET :
      return &ultraviolet_sensor_;
    default :
      return nullptr;
  }
}

void SensorLightCallback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    float lux = event->values[0];
    LoggerD("enter %f", lux);
    //TODO fill
}

void SensorMagneticCallback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    sensor_data_accuracy_e accuracy = static_cast<sensor_data_accuracy_e>(event->accuracy);
    float x = event ->values[0];
    float y = event ->values[1];
    float z = event ->values[2];
    LoggerD("enter [ %f , %f , %f ] [ %d ]",x, y, z, accuracy);
    //TODO fill
}

void SensorPressureCallback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    float pressure = event->values[0];
    LoggerD("enter %f", pressure);
    //TODO fill
}

void SensorProximityCallback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    float distance = event->values[0];
    LoggerD("enter %f", distance);
    //TODO fill
}

void SensorUltravioletCallback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    float index = event->values[0];
    LoggerD("enter %f", index);
    //TODO fill
}

CallbackPtr SensorService::GetCallbackFunction(sensor_type_e type_enum) {
  LoggerD("Entered");
  switch(type_enum) {
    case SENSOR_LIGHT :
      return &SensorLightCallback;
    case SENSOR_MAGNETIC :
      return &SensorMagneticCallback;
    case SENSOR_PRESSURE :
      return &SensorPressureCallback;
    case SENSOR_PROXIMITY :
      return &SensorProximityCallback;
    case SENSOR_ULTRAVIOLET :
      return &SensorUltravioletCallback;
    default :
      return nullptr;
  }
}

} // namespace sensor
} // namespace extension
