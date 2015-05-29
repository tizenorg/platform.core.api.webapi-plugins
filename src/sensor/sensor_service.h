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
