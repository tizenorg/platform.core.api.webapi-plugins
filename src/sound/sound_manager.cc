// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sound/sound_manager.h"

#include <tizen/tizen.h>
#include <vconf.h>
#include <vconf-keys.h>


//This constant was originally defined in vconf.h. However, in tizen 3, it
//appears, it is removed (or defined only in vconf-internals.h)
//It is not clear, if it is final solution, or not.
#ifndef VCONF_OK
#define VCONF_OK 0
#endif

#include "sound/sound_instance.h"
#include "common/logger.h"
#include "common/converter.h"

namespace extension {
namespace sound {

using namespace common;

const std::map<std::string, sound_type_e> SoundManager::platform_enum_map_ = {
    {"SYSTEM", SOUND_TYPE_SYSTEM},
    {"NOTIFICATION", SOUND_TYPE_NOTIFICATION},
    {"ALARM", SOUND_TYPE_ALARM},
    {"MEDIA", SOUND_TYPE_MEDIA},
    {"VOICE", SOUND_TYPE_VOICE},
    {"RINGTONE", SOUND_TYPE_RINGTONE}};

PlatformResult SoundManager::StrToPlatformEnum(const std::string& key,
                                               sound_type_e* sound_type) {
  if (platform_enum_map_.find(key) == platform_enum_map_.end()) {
    std::string message = "Platform enum value not found for key " + key;
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
  }

  *sound_type = platform_enum_map_.at(key);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::PlatformEnumToStr(const sound_type_e value,
                                               std::string* sound_type) {
  for (auto& item : platform_enum_map_) {
    if (item.second == value) {
      *sound_type = item.first;

      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  std::string message =
      "Platform enum value " + std::to_string(value) + " not found";

  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

SoundManager::SoundManager()
    : soundModeChangeListening(false), soundModeListener(nullptr) {
  FillMaxVolumeMap();
}

SoundManager::~SoundManager() {
  if (soundModeChangeListening) {
    int status = vconf_ignore_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, SoundManager::soundModeChangedCb);
    if (VCONF_OK != status) {
      LoggerE("Cannot disable listener!");
    }
  }
}

SoundManager* SoundManager::GetInstance() {
  static SoundManager instance;
  return &instance;
}

void SoundManager::FillMaxVolumeMap() {
  int max = 100;
  int ret;

  for (auto& item : platform_enum_map_) {
    max = 100;

    ret = sound_manager_get_max_volume(item.second, &max);
    if (ret != SOUND_MANAGER_ERROR_NONE) {
      LoggerE("SoundManagerGetMaxVolumeFailed : %d", ret);
    }

    LoggerD("maxVolume: %d - %d", item.second, max);

    max_volume_map_[item.second] = max;
  }
}

PlatformResult SoundManager::GetMaxVolume(sound_type_e type, int* max_volume) {
  auto it = max_volume_map_.find(type);
  if (it == max_volume_map_.end()) {
    std::string sound_type;
    PlatformResult status = PlatformEnumToStr(type, &sound_type);
    if (status.IsError()) return status;
    LoggerE("Failed to find maxVolume of type: %s", sound_type.c_str());

    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to find maxVolume");
  }

  *max_volume = it->second;

  return PlatformResult(ErrorCode::NO_ERROR);
}

double SoundManager::ConvertToSystemVolume(int max_volume, int volume) {
  return static_cast<double>(volume) / max_volume;
}

void SoundManager::VolumeChangeCallback(sound_type_e type, unsigned int value) {
  LoggerD("VolumeChangeCallback: type: %d, value: %d", type, value);

  // Prepare response
  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();

  response_obj.insert(
      std::make_pair("listenerId", picojson::value("VolumeChangeListener")));

  std::string sound_type;
  PlatformResult status = PlatformEnumToStr(type, &sound_type);
  if (status.IsError())
      return;

  response_obj.insert(
      std::make_pair("type", picojson::value(sound_type)));

  int max_volume;
  status = GetMaxVolume(type, &max_volume);
  if (status.IsError())
      return;

  response_obj.insert(std::make_pair(
      "volume",
      picojson::value(ConvertToSystemVolume(max_volume, value))));

  SoundInstance::GetInstance().PostMessage(response.serialize().c_str());
}

PlatformResult SoundManager::GetSoundMode(std::string* sound_mode_type) {
  int isEnableSound = 0;
  int isEnableVibrate = 0;

  *sound_mode_type = "MUTE";

  int ret = vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &isEnableSound);
  if (VCONF_OK != ret) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unknown error: " + std::to_string(ret));
  }

  ret =
      vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &isEnableVibrate);
  if (VCONF_OK != ret) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unknown error: " + std::to_string(ret));
  }

  if (isEnableSound && isEnableVibrate) {
    LoggerE("Wrong state (sound && vibration)");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform has wrong state.");
  }

  if (isEnableSound) {
    *sound_mode_type = "SOUND";
  } else if (isEnableVibrate) {
    *sound_mode_type = "VIBRATE";
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::SetVolume(const picojson::object& args) {
  const std::string& type = FromJson<std::string>(args, "type");
  double volume = FromJson<double>(args, "volume");

  LoggerD("SoundType: %s", type.c_str());
  LoggerD("volume: %f", volume);

  if (volume > 1.0 || volume < 0.0) {
    LoggerE("Volume should be the value between 0 and 1.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Volume should be the value between 0 and 1.");
  }

  sound_type_e sound_type;
  PlatformResult status = SoundManager::StrToPlatformEnum(type, &sound_type);
  if (status.IsError()) return status;

  auto it = max_volume_map_.find(sound_type);
  if (it == max_volume_map_.end()) {
    LoggerE("Failed to find maxVolume of type: %d", type.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to find maxVolume");
  }

  int max_volume = it->second;
  int value = round(volume * max_volume);
  LoggerD("volume: %lf, maxVolume: %d, value: %d", volume, max_volume, value);

  int ret = sound_manager_set_volume(sound_type, value);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to set volume: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to set volume");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::GetVolume(const picojson::object& args,
                                       double* volume) {
  const std::string& type = FromJson<std::string>(args, "type");
  int value;

  sound_type_e type_enum;
  PlatformResult status = SoundManager::StrToPlatformEnum(type, &type_enum);
  if (status.IsError()) return status;

  int max_volume;
  status = GetMaxVolume(type_enum, &max_volume);
  if (status.IsError()) return status;

  int ret = sound_manager_get_volume(type_enum, &value);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to get volume: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get volume");
  }

  *volume = ConvertToSystemVolume(max_volume, value);
  LoggerD("volume: %lf, maxVolume: %d, value: %d", volume, max_volume, value);

  return PlatformResult(ErrorCode::NO_ERROR);
}

void SoundManager::soundModeChangedCb(keynode_t*, void* user_data)
{
  if (user_data == nullptr) {
    LoggerE("Invalid callback data!");
    return;
  }
  SoundManager* self = static_cast<SoundManager*>(user_data);

  std::string soundModeType;
  PlatformResult status = self->GetSoundMode(&soundModeType);

  if (status.IsSuccess() && self->soundModeListener) {
    self->soundModeListener->OnSoundModeChange(soundModeType);
  } else {
    LoggerE("No SoundModeListener attached");
  }
}

PlatformResult SoundManager::SetSoundModeChangeListener(
    SoundManagerSoundModeChangedListener* listener) {
  soundModeListener = listener;
  if (soundModeChangeListening) return PlatformResult(ErrorCode::NO_ERROR);

  int status = vconf_notify_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL,
                                        SoundManager::soundModeChangedCb, this);
  if (VCONF_OK == status) {
    soundModeChangeListening = true;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  LoggerE("SoundModeChangeListener no setted");
  return PlatformResult(ErrorCode::UNKNOWN_ERR,
                        "SoundModeChangeListener no setted");
}

PlatformResult SoundManager::UnsetSoundModeChangeListener() {
  soundModeListener = nullptr;
  if (!soundModeChangeListening) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int status = vconf_ignore_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL,
                                        SoundManager::soundModeChangedCb);
  if (VCONF_OK == status) {
    soundModeChangeListening = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  LoggerE("SoundModeChangeListener no unsetted");
  return PlatformResult(ErrorCode::UNKNOWN_ERR,
                        "SoundModeChangeListener no unsetted");
}

PlatformResult SoundManager::SetVolumeChangeListener() {
  if (!is_volume_change_listener_) {
    int ret = sound_manager_set_volume_changed_cb(
        [](sound_type_e type, unsigned int value, void* ud) {
          return static_cast<SoundManager*>(ud)
              ->VolumeChangeCallback(type, value);
        },
        static_cast<void*>(this));

    if (ret != SOUND_MANAGER_ERROR_NONE) {
      LoggerE("Failed to set volume changed callback: error code: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to set volume changed callback");
    }

    is_volume_change_listener_ = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::UnsetVolumeChangeListener() {
  if (!is_volume_change_listener_) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int ret = sound_manager_unset_volume_changed_cb();
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to unset volume changed callback");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to unset volume changed callback");
  }

  is_volume_change_listener_ = false;

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace sound
}  // namespace extension
