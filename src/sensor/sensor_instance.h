// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SENSOR_SENSOR_INSTANCE_H_
#define SENSOR_SENSOR_INSTANCE_H_

#include "common/extension.h"

#include "sensor/sensor_service.h"

namespace extension {
namespace sensor {

class SensorInstance : public common::ParsedInstance {
 public:
  SensorInstance();
  virtual ~SensorInstance();

 private:
  void GetAvailableSensors(const picojson::value& args, picojson::object& out);
  void SensorStop(const picojson::value& args, picojson::object& out);
  void SensorSetChangeListener(const picojson::value& args, picojson::object& out);
  void SensorUnsetChangeListener(const picojson::value& args, picojson::object& out);
  void SensorStart(const picojson::value& args, picojson::object& out);
  void SensorGetData(const picojson::value& args, picojson::object& out);

  SensorService service_;
};

} // namespace sensor
} // namespace extension

#endif // SENSOR_SENSOR_INSTANCE_H_
