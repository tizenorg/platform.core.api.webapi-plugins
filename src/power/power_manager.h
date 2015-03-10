// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  common::PlatformResult Request(PowerResource resource, PowerState state);
  common::PlatformResult Release(PowerResource resource);
  common::PlatformResult GetScreenBrightness(double* output);
  common::PlatformResult SetScreenBrightness(double brightness);
  common::PlatformResult RestoreScreenBrightness();
  bool IsScreenOn();
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

