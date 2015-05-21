// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SENSOR_SENSOR_SERVICE_H_
#define SENSOR_SENSOR_SERVICE_H_

#include <memory>

#include <sensor.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace sensor {

class SensorData;
class SensorInstance;

class SensorService {
 public:
  explicit SensorService(SensorInstance& instance);
  ~SensorService();

  void GetAvailableSensors(picojson::object& out);
  void SensorStart(const picojson::value& args, picojson::object& out);
  void SensorStop(const picojson::value& args, picojson::object& out);
  void SensorSetChangeListener(const picojson::value& args, picojson::object& out);
  void SensorUnsetChangeListener(const picojson::value& args, picojson::object& out);
  void GetSensorData(const picojson::value& args, picojson::object& out);

 private:
  std::shared_ptr<SensorData> GetSensor(sensor_type_e type_enum);
  void AddSensor(SensorData* sensor);

  std::map<sensor_type_e, std::shared_ptr<SensorData>> sensors_;
  SensorInstance& instance_;
};

} // namespace sensor
} // namespace extension

#endif // SENSOR_SENSOR_SERVICE_H_
