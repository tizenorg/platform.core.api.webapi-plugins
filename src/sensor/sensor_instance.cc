// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensor/sensor_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace sensor {

using namespace common;

SensorInstance::SensorInstance()
    : service_(*this) {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SensorInstance::x, this, _1, _2));
  REGISTER_SYNC("SensorService_getAvailableSensors", GetAvailableSensors);
  REGISTER_SYNC("Sensor_stop", SensorStop);
  REGISTER_SYNC("Sensor_setChangeListener", SensorSetChangeListener);
  REGISTER_SYNC("Sensor_unsetChangeListener", SensorUnsetChangeListener);
#undef REGISTER_SYNC
#define REGISTER_ASYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SensorInstance::x, this, _1, _2));
  REGISTER_ASYNC("Sensor_start", SensorStart);
  REGISTER_ASYNC("Sensor_getData", SensorGetData);
#undef REGISTER_ASYNC
}

SensorInstance::~SensorInstance() {
}

void SensorInstance::GetAvailableSensors(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.GetAvailableSensors(out);
}

void SensorInstance::SensorStop(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.SensorStop(args, out);
}

void SensorInstance::SensorSetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.SensorSetChangeListener(args, out);
}

void SensorInstance::SensorUnsetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.SensorUnsetChangeListener(args, out);
}

void SensorInstance::SensorStart(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.SensorStart(args, out);
}

void SensorInstance::SensorGetData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  service_.GetSensorData(args, out);
}

} // namespace sensor
} // namespace extension
