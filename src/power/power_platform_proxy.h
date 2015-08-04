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

  int GetBrightness();
  int IsCustomBrightness();

  static PowerPlatformProxy& GetInstance();

 private:
  PowerPlatformProxy();
  virtual ~PowerPlatformProxy();

	common::DBusOperation dbus_op_;
};

} // namespace power
} // namespace extension

#endif // POWER_POWER_PLATFORM_PROXY_H_
