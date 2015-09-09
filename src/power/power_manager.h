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

#ifndef POWER_POWER_MANAGER_H_
#define POWER_POWER_MANAGER_H_

#include <string>
#include <list>
#include <device/callback.h>

#include "common/platform_result.h"

namespace extension {
namespace power {

enum PowerResource {
  POWER_RESOURCE_SCREEN = 0,
  POWER_RESOURCE_CPU = 1,
};

enum PowerState {
  POWER_STATE_NONE = 0,
  POWER_STATE_SCREEN_OFF,
  POWER_STATE_SCREEN_DIM,
  POWER_STATE_SCREEN_NORMAL,
  POWER_STATE_SCREEN_BRIGHT,
  POWER_STATE_CPU_AWAKE,
};

class PowerManagerListener {
 public:
  virtual void OnScreenStateChanged(PowerState prev_state, PowerState new_state)=0;
};

class PowerManager {
 public:
  void AddListener(PowerManagerListener* listener);
  void RemoveListener(PowerManagerListener* listener);
  common::PlatformResult Request(PowerResource resource, PowerState state);
  common::PlatformResult Release(PowerResource resource);
  common::PlatformResult GetScreenBrightness(double* output);
  common::PlatformResult SetScreenBrightness(double brightness);
  common::PlatformResult RestoreScreenBrightness();
  common::PlatformResult IsScreenOn(bool* state);
  common::PlatformResult SetScreenState(bool onoff);

  static PowerManager* GetInstance();
 private:
  int GetPlatformBrightness();
  common::PlatformResult SetPlatformBrightness(int brightness);
  common::PlatformResult RestoreSettedBrightness();

  PowerManager();
  virtual ~PowerManager();

  void BroadcastScreenState(PowerState current);

  static void OnPlatformStateChangedCB(device_callback_e type, void* value, void* user_data);

  std::list<PowerManagerListener*> listeners_;

  PowerState current_state_;
  bool bright_state_enabled_;
  int current_brightness_;
  bool should_be_read_from_cache_;
  bool set_custom_brightness_;
  PowerState current_requested_state_;
};

} // namespace power
} // namespace extension

#endif // POWER_POWER_MANAGER_H_

