// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef POWER_POWER_PLATFORM_PROXY_H_
#define POWER_POWER_PLATFORM_PROXY_H_

#include "common/dbus_operation.h"

namespace extension {
namespace power {

class PowerPlatformProxy {
 public:
	int LockState();
	int UnlockState();
	int SetBrightnessFromSettings();
	int SetBrightness(int val);

  static PowerPlatformProxy& GetInstance();

 private:
  PowerPlatformProxy();
  virtual ~PowerPlatformProxy();

	common::DBusOperation dbus_op_;
};

} // namespace power
} // namespace extension

#endif // POWER_POWER_PLATFORM_PROXY_H_
