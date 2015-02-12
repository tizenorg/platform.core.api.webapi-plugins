// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensor/sensor_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "sensor_service.h"

namespace extension {
namespace sensor {

using namespace common;

SensorInstance& SensorInstance::getInstance() {
  static SensorInstance instance;
  return instance;
}

SensorInstance::SensorInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SensorInstance::x, this, _1, _2));
  REGISTER_SYNC("SensorService_getAvailableSensors", GetAvailableSensors);
  REGISTER_SYNC("Sensor_stop", SensorStop);
  REGISTER_SYNC("Sensor_setChangeListener", SensorSetChangeListener);
  REGISTER_SYNC("Sensor_unsetChangeListener", SensorUnsetChangeListener);
#undef REGISTER_SYNC
#define REGISTER_ASYNC(c,x) \
    RegisterHandler(c, std::bind(&SensorInstance::x, this, _1, _2));
  REGISTER_ASYNC("Sensor_start", SensorStart);
  REGISTER_ASYNC("LightSensor_getData", LightSensorGetData);
  REGISTER_ASYNC("MagneticSensor_getData", MagneticSensorGetData);
  REGISTER_ASYNC("PressureSensor_getData", PressureSensorGetData);
  REGISTER_ASYNC("ProximitySensor_getData", ProximitySensorGetData);
  REGISTER_ASYNC("UltravioletSensor_getData", UltravioletSensorGetData);

#undef REGISTER_ASYNC
}

SensorInstance::~SensorInstance() {
}

void SensorInstance::GetAvailableSensors(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  SensorService::GetInstance()->GetAvailableSensors(out);
}

void SensorInstance::SensorStop(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::SensorSetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::SensorUnsetChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::SensorStart(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::LightSensorGetData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::MagneticSensorGetData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::PressureSensorGetData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::ProximitySensorGetData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

void SensorInstance::UltravioletSensorGetData(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  //empty stub
}

} // namespace sensor
} // namespace extension
