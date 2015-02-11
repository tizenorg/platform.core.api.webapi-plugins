// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SENSOR_SENSOR_EXTENSION_H_
#define SENSOR_SENSOR_EXTENSION_H_

#include "common/extension.h"

class SensorExtension : public common::Extension {
 public:
  SensorExtension();
  virtual ~SensorExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // SENSOR_SENSOR_EXTENSION_H_
