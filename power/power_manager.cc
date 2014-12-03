// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "power_manager.h"

#include <unistd.h>

#include <cstring>
#include <algorithm>

#include <vconf.h>
#include <device/display.h>
#include <device/power.h>
#include <device/callback.h>

#include "common/logger.h"
#include "common/platform_exception.h"
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
      current_requested_state_(POWER_STATE_NONE) {

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
  int ret = device_remove_callback(DEVICE_CALLBACK_DISPLAY_STATE,
                                   PowerManager::OnPlatformStateChangedCB);
  if (DEVICE_ERROR_NONE != ret)
    LoggerE("device_remove_callback failed (%d)", ret);
}

PowerManager* PowerManager::GetInstance(){
  static PowerManager instance;
  return &instance;
}

void PowerManager::OnPlatformStateChangedCB(device_callback_e type, void* value, void* user_data) {
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
        object->RestoreScreenBrightness();
        object->set_custom_brightness_ = false;
      }
      break;
    }
  }
  object->BroadcastScreenState(current);
}

void PowerManager::AddListener(PowerManagerListener* listener) {
  auto it = std::find(listeners_.begin(), listeners_.end(), listener);
  if (it == listeners_.end())
    listeners_.push_back(listener);
}

void PowerManager::Request(PowerResource resource, PowerState state) {
  if (resource == POWER_RESOURCE_SCREEN && state == POWER_STATE_CPU_AWAKE)
    throw InvalidValuesException("invalid PowerState");
  if (resource == POWER_RESOURCE_CPU && state != POWER_STATE_CPU_AWAKE)
    throw InvalidValuesException("invalid PowerState");

  if(current_requested_state_ == POWER_STATE_SCREEN_DIM) {
    int ret = PowerPlatformProxy::GetInstance().UnlockState();
    if (ret < 0) {
      LoggerE("deviceUnlockState error %d", ret);
      throw UnknownException("device_power_request_unlock error");
    }
  }

  int ret = 0;
  switch (state) {
    case POWER_STATE_CPU_AWAKE:
    {
      ret = device_power_request_lock(POWER_LOCK_CPU, 0);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_power_request_lock error %d", ret);
        throw UnknownException("device_power_request_lock error");
      }
      break;
    }
    case POWER_STATE_SCREEN_DIM:
    {
      ret = PowerPlatformProxy::GetInstance().LockState();
      if (ret < 0) {
        LoggerE("device_power_request_lock error %d", ret);
        throw UnknownException("device_power_request_lock error");
      }
      break;
    }
    case POWER_STATE_SCREEN_NORMAL:
    {
      ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_power_request_lock error %d", ret);
        throw UnknownException("device_power_request_lock error");
      }
      break;
    }
    case POWER_STATE_SCREEN_BRIGHT:
    {
      int max_brightness;
      ret = device_display_get_max_brightness(0, &max_brightness);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("Platform error while getting max brightness: %d", ret);
        throw UnknownException("Platform error while getting max brightness");
      }

      SetPlatformBrightness(max_brightness);
      LoggerD("Succeeded setting the brightness to a max level: %d", max_brightness);

      ret = device_display_change_state(DISPLAY_STATE_NORMAL);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_display_change_state(DISPLAY_STATE_NORMAL) error %d", ret);
        throw UnknownException("device_display_change_state error");
      }

      ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("device_power_request_lock error %d", ret);
        throw UnknownException("device_power_request_lock error");
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
      throw InvalidValuesException("SCREEN_OFF state cannot be requested");
    default:
      throw UnknownException("Platform error while locking state");
  }

  current_requested_state_ = state;
}

void PowerManager::Release(PowerResource resource) {
  int ret;
  if (POWER_RESOURCE_SCREEN == resource) {
    ret = device_power_release_lock(POWER_LOCK_DISPLAY);
    if (DEVICE_ERROR_NONE != ret)
      LoggerE("Platform return value from dim unlock: %d", ret);

    if (bright_state_enabled_) {
      ret = PowerPlatformProxy::GetInstance().SetBrightnessFromSettings();
      if (DEVICE_ERROR_NONE != ret) {
        LoggerE("Platform error while setting restore brightness %d", ret);
        throw UnknownException("Platform error while setting restore brightness");
      }
    }
    bright_state_enabled_ = false;

    display_state_e platform_state = DISPLAY_STATE_NORMAL;
    if(current_requested_state_ == POWER_STATE_SCREEN_DIM) {
      ret = PowerPlatformProxy::GetInstance().UnlockState();
    } else {
      ret = device_display_get_state(&platform_state);
    }
    if (DEVICE_ERROR_NONE != ret)
      LoggerE("device_display_get_state failed (%d)", ret);

    if (DISPLAY_STATE_NORMAL == platform_state)
      BroadcastScreenState(POWER_STATE_SCREEN_NORMAL);

    current_requested_state_ = POWER_STATE_NONE;
  } else if (POWER_RESOURCE_CPU == resource) {
    ret = device_power_release_lock(POWER_LOCK_CPU);
    if (DEVICE_ERROR_NONE != ret)
      LoggerE("Platform return value from off unlock: %d", ret);
  }
}

double PowerManager::GetScreenBrightness() {
  int brightness = GetPlatformBrightness();
  LoggerD("Brightness value: %d", brightness);

  int max_brightness;
  int ret = device_display_get_max_brightness(0, &max_brightness);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while getting brightness: %d", ret);
    throw UnknownException("Platform error while getting max brightness");
  }
  return (double)brightness/(double)max_brightness;
}

void PowerManager::SetScreenBrightness(double brightness) {
  if (brightness > 1 || brightness < 0)
    throw InvalidValuesException("brightness should be 0 <= brightness <= 1");
  int max_brightness;
  int ret = device_display_get_max_brightness(0, &max_brightness);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while setting restore brightness: %d", ret);
    throw UnknownException("Platform error while getting max brightness");
  }

  int platform_brightness = (int)(brightness * max_brightness);
  if (platform_brightness == 0)
    platform_brightness = 1;
  SetPlatformBrightness(platform_brightness);
  LoggerD("Set the brightness value: %d", platform_brightness);
}

bool PowerManager::IsScreenOn() {
  display_state_e platform_state = DISPLAY_STATE_NORMAL;
  int ret = device_display_get_state(&platform_state);
  if (DEVICE_ERROR_NONE != ret)
    LoggerE("device_display_get_state failed (%d)", ret);
  return DISPLAY_STATE_SCREEN_OFF != platform_state;
}

void PowerManager::SetScreenState(bool onoff) {
  int ret = device_display_change_state(onoff ? DISPLAY_STATE_NORMAL : DISPLAY_STATE_SCREEN_OFF);
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while changing screen state %d", ret);
    throw UnknownException("Platform error while changing screen state");
  }

  int timeout = 100;
  while (timeout--) {
    if (IsScreenOn() == onoff)
      break;
    usleep(100000);
  }
}

void PowerManager::RestoreScreenBrightness() {
  int ret = PowerPlatformProxy::GetInstance().SetBrightnessFromSettings();
  if (DEVICE_ERROR_NONE != ret) {
    LoggerE("Platform error while restoring brightness %d", ret);
    throw UnknownException("Platform error while restoring brightness");
  }
}

void PowerManager::SetPlatformBrightness(int brightness) {
  if (current_state_ == POWER_STATE_SCREEN_DIM) {
    current_brightness_ = brightness;
    LoggerD("Current state is not normal state the value is saved in cache: %d", brightness);
    should_be_read_from_cache_ = true;
    return;
  } else if (current_state_ == POWER_STATE_SCREEN_BRIGHT) {
    current_brightness_ = brightness;
    LoggerD("Current state is not normal state the value is saved in cache: %d", brightness);
    should_be_read_from_cache_ = true;
    return;
  } else {
    should_be_read_from_cache_ = false;
  }

  int ret = PowerPlatformProxy::GetInstance().SetBrightness(brightness);
  if (ret != 0) {
    LoggerE("Platform error while setting %d brightness: %d", brightness, ret);
    throw UnknownException("Platform error while setting brightness.");
  }
  set_custom_brightness_ = true;
  current_brightness_ = brightness;
}

int PowerManager::GetPlatformBrightness(){
  int brightness = 0;

  int current_power_state = 1;
  vconf_get_int(VCONFKEY_PM_STATE, &current_power_state);
  if (current_power_state == VCONFKEY_PM_STATE_NORMAL) {
    vconf_get_int(VCONFKEY_PM_CURRENT_BRIGHTNESS, &brightness);
    LoggerD("[PM_STATE_NORMAL] return VCONFKEY_PM_CURRENT_BRIGHTNESS %d", brightness);
    return brightness;
  }

  int is_custom_mode = 0;
  vconf_get_int(VCONFKEY_PM_CUSTOM_BRIGHTNESS_STATUS, &is_custom_mode);
  if ((is_custom_mode && current_brightness_ != -1) || should_be_read_from_cache_) {
    LoggerD("return custom brightness %d", current_brightness_);
    return current_brightness_;
  }

  int is_auto_brightness = 0;
  vconf_get_int(VCONFKEY_SETAPPL_BRIGHTNESS_AUTOMATIC_INT, &is_auto_brightness);
  if (is_auto_brightness == 1) {
    int ret = vconf_get_int(VCONFKEY_SETAPPL_PREFIX"/automatic_brightness_level" /*prevent RSA build error*/, &brightness);
    if (ret != 0) //RSA binary has no AUTOMATIC_BRIGHTNESS
      vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness);
  } else {
    vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness);
  }
  LoggerD("BRIGHTNESS(%s) %d", is_auto_brightness == 1 ? "auto" : "fix" , brightness);

  return brightness;
}


void PowerManager::RestoreSettedBrightness() {
  int is_custom_mode = 0;
  vconf_get_int(VCONFKEY_PM_CUSTOM_BRIGHTNESS_STATUS, &is_custom_mode);
  if (is_custom_mode || should_be_read_from_cache_) {
    if (current_brightness_ == -1) {
      // brightness was changed in other process
      RestoreScreenBrightness();
    } else {
      SetPlatformBrightness(current_brightness_);
    }
  }
  should_be_read_from_cache_ = false;
}

void PowerManager::BroadcastScreenState(PowerState current){
  if (current_state_ == current)
    return;

  PowerState prev_state = current_state_;
  current_state_ = current;

  if (current_state_ == POWER_STATE_SCREEN_NORMAL) {
    if (prev_state == POWER_STATE_SCREEN_DIM) {
      try {
        RestoreSettedBrightness();
      } catch (const PlatformException& err) {
        LoggerE("Error restore custom brightness %s", err.getMessage().c_str());
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

