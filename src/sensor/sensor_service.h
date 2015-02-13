// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SENSOR_SENSOR_SERVICE_H_
#define SENSOR_SENSOR_SERVICE_H_

#include <sensor.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace sensor {

typedef void (*CallbackPtr)(sensor_h sensor, sensor_event_s *event, void *user_data);

class SensorService {
  typedef struct {
    sensor_h handle;
    sensor_listener_h listener;
  } SensorData;

 public:
  static SensorService* GetInstance();
  void GetAvailableSensors(picojson::object& out);
  void SensorStart(const picojson::value& args, picojson::object& out);
  void SensorStop(const picojson::value& args, picojson::object& out);
  void SensorSetChangeListener(const picojson::value& args, picojson::object& out);
  void SensorUnsetChangeListener(const picojson::value& args, picojson::object& out);

 private:
  SensorService();
  ~SensorService();
  std::string GetSensorErrorMessage(const int error_code);
  common::PlatformResult GetSensorPlatformResult(const int error_code, const std::string &hint);

  common::PlatformResult CheckSensorInitialization(sensor_type_e type_enum);
  SensorData* GetSensorStruct(sensor_type_e type_enum);
  CallbackPtr GetCallbackFunction(sensor_type_e type_enum);

  SensorData light_sensor_;
  SensorData magnetic_sensor_;
  SensorData pressure_sensor_;
  SensorData proximity_sensor_;
  SensorData ultraviolet_sensor_;
};

} // namespace sensor
} // namespace extension

#endif // SENSOR_SENSOR_SERVICE_H_
