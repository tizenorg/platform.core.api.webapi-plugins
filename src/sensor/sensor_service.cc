// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensor_service.h"

#include "common/logger.h"
#include "common/platform_exception.h"

using namespace common;

namespace extension {
namespace sensor {

SensorService::SensorService() {

}

SensorService::~SensorService() {

}

SensorService* SensorService::GetInstance() {
  static SensorService instance_;
  return &instance_;
}

void SensorService::GetAvailableSensors(picojson::object& out) {
  LoggerD("Entered");

}

} // namespace sensor
} // namespace extension
