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

#include "power_manager.h"

#include <unistd.h>

#include <cstring>
#include <algorithm>

#include <vconf.h>
#include <device/display.h>
#include <device/power.h>
#include <device/callback.h>

#include "common/logger.h"
#include "power_platform_proxy.h"

using namespace common;
using namespace std;

namespace extension {
namespace power {

PowerManager::PowerManager()
    : current_state_(POWER_STATE_SCREEN_NORMAL),
      bright_state_enabled_(false),
      current_brightness_(-1),
      should_be_read_from_cache_(false),
      set_custom_brightness_(false),
      current_requested_state_(POWER_STATE_NONE) {

  LoggerD("Enter");

  display_state_e platform_state = DISPLAY_STATE_NORMAL;
  int ret = device_display_get_state(&platform_state);
  if (DEVICE_ERROR_NONE != ret)
    LoggerE("device_display_get_state failed (%d)", ret);

  switch (platform_state) {
    case DISPLAY_STATE_NORMAL :
      current_state_ = POWER_STATE_SCREEN_NORMAL;
      break;
    case DISPLAY_STATE_SCREEN_DIM :
      current_state_ = POWER_STATE_SCREEN_DIM;
      break;
    case DISPLAY_STATE_SCREEN_OFF :
      current_state_ = POWER_STATE_SCREEN_OFF;
      break;
    default:
      current_state_ = POWER_STATE_NONE;
      break;
  }

  ret = device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE,
                            PowerManager::OnPlatformStateChangedCB,
                            static_cast<void*>(this));
  if (DEVICE_ERROR_NONE != ret)
    LoggerE("device_add_callback failed (%d)", ret);
}

PowerManager::~PowerManager() {
  LoggerD("Enter");
  int ret = device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE,
                                   PowerManager::OnPlatformStateChangedCB);
  if (DEVICE_ERROR_NONE != ret)
    LoggerE("device_remove_callback failed (%d)", ret);
}

PowerManager* PowerManager::GetInstance(){
  LoggerD("Enter");
  static PowerManager instance;
  return &instance;
}

void PowerManager::OnPlatformStateChangedCB(device_callback_e type, void* value, void* user_data) {
  LoggerD("Enter");
  PowerManager* object = static_cast<PowerManager*>(user_data);
  if (object == NULL){
    LoggerE("User data is NULL");
    return;
  }
  if (type != DEVICE_CALLBACK_DISPLAY_STATE){
    LoggerE("type is not DISPLAY_STATE");
    return;
  }
  display_state_e state = static_cast<display_state_e>(reinterpret_cast<int>(value));
  PowerState current = POWER_STATE_SCREEN_OFF;
  switch (state) {
    case DISPLAY_STATE_NORMAL :
      current = object->bright_state_enabled_ ? POWER_STATE_SCREEN_BRIGHT : POWER_STATE_SCREEN_NORMAL;
      break;
    case DISPLAY_STATE_SCREEN_DIM :
      current = POWER_STATE_SCREEN_DIM;
      break;
    case DISPLAY_STATE_SCREEN_OFF :
    {
      current = POWER_STATE_SCREEN_OFF;
      if (object->set_custom_brightness_ == true) {
        PlatformResult result = object->RestoreScreenBrightness();
        if (result.IsError()) {
          LOGGER(ERROR) << "RestoreScreenBrightness failed";
          return;
        }
        object->set_custom_brightness_ = false;
      }
      break;
    }
  }
  object->BroadcastScreenState(current);
}

void PowerManager::AddListener(PowerManagerListener* listener) {
  LoggerD("Enter");
  auto it = std::find(listeners_.begin(), listeners_.end(), listener);
  if (it == listeners_.end())
    listeners_.push_back(listener);
}

void PowerManager::RemoveListener(PowerManagerListener* listener) {
  LoggerD("Enter");
  listeners_.remove(listener);
}

PlatformResult PowerManager::Request(PowerResource resource, PowerState state) {
  LoggerD("Enter");
  if (resource == POWER_RESOURCE_SCREEN && state == POWER_STATE_CPU_AWAKE)
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "invalid PowerState");
  if (resource == POWER_RESOURCE_CPU && state != POWER_STATE_CPU_AWAKE)
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "invalid PowerState");

  if(current_requested_state_ == POWER_STATE_SCREEN_DIM) {
    int ret = PowerPlatformProxy::GetInstance().UnlockState();
    if (ret < 0) {
      LoggerE("deviceUnlockState error %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "device_power_request_unlock error");
    }
  }

  int ret = 0;
  switch (state) {
    case POWER_STATE_CPU_AWAKE:
    {
      ret = device_power_request_lock(POWER_LOCK_CPU, 0);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_power_request_lock error %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "device_power_request_lock error");
      }
      break;
    }
    case POWER_STATE_SCREEN_DIM:
    {
      ret = PowerPlatformProxy::GetInstance().LockState();
      if (ret < 0) {
        LoggerE("device_power_request_lock error %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "device_power_request_lock error");
      }
      break;
    }
    case POWER_STATE_SCREEN_NORMAL:
    {
      ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_power_request_lock error %d", ret);
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                              "device_power_request_lock error");
      }
      break;
    }
    case POWER_STATE_SCREEN_BRIGHT:
    {
      int max_brightness;
      ret = device_display_get_max_brightness(0, &max_brightness);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("Platform error while getting max brightness: %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Platform error while getting max brightness");
      }

      PlatformResult set_result = SetPlatformBrightness(max_brightness);
      if (set_result.IsError())
        return set_result;
      LoggerD("Succeeded setting the brightness to a max level: %d", max_brightness);

      ret = device_display_change_state(DISPLAY_STATE_NORMAL);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_display_change_state(DISPLAY_STATE_NORMAL) error %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "device_display_change_state error");
      }

      ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_power_request_lock error %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "device_power_request_lock error");
      }

      bright_state_enabled_ = true;

      display_state_e platform_state = DISPLAY_STATE_NORMAL;
      ret = device_display_get_state(&platform_state);
      if (DEVICE_ERROR_NONE != ret)
        LoggerE("device_display_get_state failed (%d)", ret);
      if (platform_state == DISPLAY_STATE_NORMAL)
        BroadcastScreenState(POWER_STATE_SCREEN_BRIGHT);
      break;
    }
    case POWER_STATE_SCREEN_OFF:
      LoggerE("SCREEN_OFF state cannot be requested");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "SCREEN_OFF state cannot be requested");
    default:
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Platform error while locking state");
  }

  current_requested_state_ = state;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PowerManager::Release(PowerResource resource) {
  LoggerD("Enter");
  int ret;
  if (POWER_RESOURCE_SCREEN == resource) {
    ret = device_power_release_lock(POWER_LOCK_DISPLAY);
    if (DEVICE_ERROR_NONE != ret)
      LoggerE("Platform return value from dim unlock: %d", ret);

    if (bright_state_enabled_) {
      ret = PowerPlatformProxy::GetInstance().SetBrightnessFromSettings();
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("Platform error while setting restore brightness %d", ret);
        return  PlatformResult(ErrorCode::UNKNOWN_ERR,
                    "Platform error while setting restore brightness");
      }
    }
    bright_state_enabled_ = false;

    display_state_e platform_state = DISPLAY_STATE_NORMAL;
    if(current_requested_state_ == POWER_STATE_SCREEN_DIM) {
      ret = PowerPlatformProxy::GetInstance().UnlockState();
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("Failed to UnlockState (%d)", ret);
      }
    }
    ret = device_display_get_state(&platform_state);
    if (DEVICE_ERROR_NONE != ret) {
      LoggerE("device_display_get_state failed (%d)", ret);
    } else {
      if (DISPLAY_STATE_NORMAL == platform_state) {
        BroadcastScreenState(POWER_STATE_SCREEN_NORMAL);
      }
    }

    current_requested_state_ = POWER_STATE_NONE;
  } else if (POWER_RESOURCE_CPU == resource) {
    ret = device_power_release_lock(POWER_LOCK_CPU);
    if (DEVICE_ERROR_NONE != ret)
      LoggerE("Platform return value from off unlock: %d", ret);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PowerManager::GetScreenBrightness(double* output) {
  LoggerD("Enter");
  int brightness = GetPlatformBrightness();
  LoggerD("Brightness value: %d", brightness);

  int max_brightness;
  int ret = device_display_get_max_brightness(0, &max_brightness);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while getting brightness: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting max brightness");
  }
  *output = (double)brightness/(double)max_brightness;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PowerManager::SetScreenBrightness(double brightness) {
  LoggerD("Enter");
  if (brightness > 1 || brightness < 0)
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "brightness should be 0 <= brightness <= 1");
  int max_brightness;
  int ret = device_display_get_max_brightness(0, &max_brightness);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while setting restore brightness: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting max brightness");
  }

  int platform_brightness = (int)(brightness * max_brightness);
  if (platform_brightness == 0) {
    platform_brightness = 1;
  }
  PlatformResult set_result = SetPlatformBrightness(platform_brightness);
  if (set_result.IsError())
    return set_result;
  LoggerD("Set the brightness value: %d", platform_brightness);
  return set_result;
}

PlatformResult PowerManager::IsScreenOn(bool* state) {
  LoggerD("Enter");
  display_state_e platform_state = DISPLAY_STATE_NORMAL;

  int ret = device_display_get_state(&platform_state);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("device_display_get_state failed (%d)", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while getting screen state.");
  }

  *state = (DISPLAY_STATE_SCREEN_OFF != platform_state);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PowerManager::SetScreenState(bool onoff) {
  LoggerD("Enter");
  int ret = device_display_change_state(onoff ? DISPLAY_STATE_NORMAL : DISPLAY_STATE_SCREEN_OFF);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while changing screen state %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while changing screen state");
  }

  int timeout = 100;
  bool state = false;
  while (timeout--) {
    PlatformResult result = IsScreenOn(&state);
    if (result.IsError()) {
      return result;
    }

    if (state == onoff) {
      break;
    }

    struct timespec sleep_time = { 0, 100L * 1000L * 1000L };
    nanosleep(&sleep_time, nullptr);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PowerManager::RestoreScreenBrightness() {
  LoggerD("Enter");
  int ret = PowerPlatformProxy::GetInstance().SetBrightnessFromSettings();
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while restoring brightness %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while restoring brightness");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PowerManager::SetPlatformBrightness(int brightness) {
  LoggerD("Enter");
  if (current_state_ == POWER_STATE_SCREEN_DIM) {
    current_brightness_ = brightness;
    LoggerD("Current state is not normal state the value is saved in cache: %d", brightness);
    should_be_read_from_cache_ = true;
    return PlatformResult(ErrorCode::NO_ERROR);
  } else if (current_state_ == POWER_STATE_SCREEN_BRIGHT) {
    current_brightness_ = brightness;
    LoggerD("Current state is not normal state the value is saved in cache: %d", brightness);
    should_be_read_from_cache_ = true;
    return PlatformResult(ErrorCode::NO_ERROR);
  } else {
    should_be_read_from_cache_ = false;
  }

  int ret = PowerPlatformProxy::GetInstance().SetBrightness(brightness);
  if (ret != 0) {
    LoggerE("Platform error while setting %d brightness: %d", brightness, ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while setting brightness.");
  }
  set_custom_brightness_ = true;
  current_brightness_ = brightness;
  return PlatformResult(ErrorCode::NO_ERROR);
}

int PowerManager::GetPlatformBrightness(){
  LoggerD("Entered");

  int brightness = 0;

  int is_custom_mode = PowerPlatformProxy::GetInstance().IsCustomBrightness();
  if ((is_custom_mode && current_brightness_ != -1) || should_be_read_from_cache_) {
    LoggerD("return custom brightness %d", current_brightness_);
    return current_brightness_;
  }

  int is_auto_brightness = 0;
  vconf_get_int(VCONFKEY_SETAPPL_BRIGHTNESS_AUTOMATIC_INT, &is_auto_brightness);
  if (is_auto_brightness == 1) {
    int ret = vconf_get_int(VCONFKEY_SETAPPL_PREFIX"/automatic_brightness_level" /*prevent RSA build error*/, &brightness);
    if (ret != 0) {
      // RSA binary has no AUTOMATIC_BRIGHTNESS
      vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness);
    }
  } else {
    LoggerD("Brightness via DBUS");
    brightness = PowerPlatformProxy::GetInstance().GetBrightness();
  }
  LoggerD("BRIGHTNESS(%s) %d", is_auto_brightness == 1 ? "auto" : "fix" , brightness);

  return brightness;
}


PlatformResult PowerManager::RestoreSettedBrightness() {
  LoggerD("Enter");
  PlatformResult result(ErrorCode::NO_ERROR);
  int is_custom_mode = 0;
  vconf_get_int(VCONFKEY_PM_CUSTOM_BRIGHTNESS_STATUS, &is_custom_mode);
  if (is_custom_mode || should_be_read_from_cache_) {
    if (current_brightness_ == -1) {
      // brightness was changed in other process
      result = RestoreScreenBrightness();
    } else {
      result = SetPlatformBrightness(current_brightness_);
    }
  }
  should_be_read_from_cache_ = false;
  return result;
}

void PowerManager::BroadcastScreenState(PowerState current) {
  LoggerD("Enter");
  if (current_state_ == current)
    return;

  PowerState prev_state = current_state_;
  current_state_ = current;

  if (current_state_ == POWER_STATE_SCREEN_NORMAL) {
    if (prev_state == POWER_STATE_SCREEN_DIM) {
      PlatformResult result = RestoreSettedBrightness();
      if (result.IsError()) {
        LOGGER(ERROR) << "Error restore custom brightness " << result.message();
      }
    } else if (prev_state == POWER_STATE_SCREEN_OFF) {
      should_be_read_from_cache_ = false;
    }
  }

  for (auto it = listeners_.begin(); it != listeners_.end(); ++it) {
    (*it)->OnScreenStateChanged(prev_state, current_state_);
  }
}

} // namespace power
} // namespace extension
