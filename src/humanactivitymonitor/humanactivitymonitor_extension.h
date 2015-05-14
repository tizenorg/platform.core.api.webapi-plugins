// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_EXTENSION_H_
#define HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_EXTENSION_H_

#include "common/extension.h"

class HumanActivityMonitorExtension : public common::Extension {
 public:
  HumanActivityMonitorExtension();
  virtual ~HumanActivityMonitorExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // HUMANACTIVITYMONITOR_HUMANACTIVITYMONITOR_EXTENSION_H_
