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
#include "common/platform_exception.h"

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

sound_type_e SoundManager::StrToPlatformEnum(const std::string& key) {
  if (platform_enum_map_.find(key) == platform_enum_map_.end()) {
    std::string message = "Platform enum value not found for key " + key;
    // TODO:  throw InvalidValuesException(message);
  }

  return platform_enum_map_.at(key);
}

std::string SoundManager::PlatformEnumToStr(const sound_type_e value) {
  for (auto& item : platform_enum_map_) {
    if (item.second == value) return item.first;
  }

  std::string message =
      "Platform enum value " + std::to_string(value) + " not found";
  // TODO:  throw InvalidValuesException(message);
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

int SoundManager::GetMaxVolume(sound_type_e type) {
  auto it = max_volume_map_.find(type);
  if (it == max_volume_map_.end()) {
    LoggerE("Failed to find maxVolume of type: %s",
            PlatformEnumToStr(type).c_str());
    // TODO: throw UnknownException("Failed to find maxVolume");
  }

  return it->second;
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
  response_obj.insert(
      std::make_pair("type", picojson::value(PlatformEnumToStr(type))));
  response_obj.insert(std::make_pair(
      "volume",
      picojson::value(ConvertToSystemVolume(GetMaxVolume(type), value))));

  SoundInstance::GetInstance().PostMessage(response.serialize().c_str());
}

std::string SoundManager::GetSoundMode() {
  std::string sound_mode_type = "MUTE";
  int isEnableSound = 0;
  int isEnableVibrate = 0;

  int ret = vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &isEnableSound);
  if (VCONF_OK != ret) {
    LoggerE("Unknown error : %d", ret);
    // TODO: throw UnknownException(("Unknown error:
    // logSoundModeError(ret)).c_str());
  }

  ret =
      vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &isEnableVibrate);
  if (VCONF_OK != ret) {
    LoggerE("Unknown error : %d", ret);
    // TODO: throw UnknownException(("Unknown error:
    // logSoundModeError(ret)).c_str());
  }

  if (isEnableSound && isEnableVibrate) {
    LoggerE("Wrong state (sound && vibration)");
    // TODO: throw UnknownException("Platform has wrong state.");
  }

  if (isEnableSound) {
    sound_mode_type = "SOUND";
  } else if (isEnableVibrate) {
    sound_mode_type = "VIBRATE";
  }

  return sound_mode_type;
}

void SoundManager::SetVolume(const picojson::object& args) {
  const std::string& type = FromJson<std::string>(args, "type");
  double volume = FromJson<double>(args, "volume");

  LoggerD("SoundType: %s", type.c_str());
  LoggerD("volume: %f", volume);

  if (volume > 1.0 || volume < 0.0) {
    LoggerE("Volume should be the value between 0 and 1.");
    // TODO: throw InvalidValuesException("Volume should be the value between 0
    // and 1.");
  }

  auto it = max_volume_map_.find(SoundManager::StrToPlatformEnum(type));
  if (it == max_volume_map_.end()) {
    LoggerE("Failed to find maxVolume of type: %d", type.c_str());
    // TODO: throw UnknownException("Failed to find maxVolume");
  }

  int max_volume = it->second;
  int value = round(volume * max_volume);
  LoggerD("volume: %lf, maxVolume: %d, value: %d", volume, max_volume, value);

  int ret =
      sound_manager_set_volume(SoundManager::StrToPlatformEnum(type), value);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to set volume: %d", ret);
    // TODO: throw UnknownException("Failed to set volume");
  }
}

double SoundManager::GetVolume(const picojson::object& args) {
  const std::string& type = FromJson<std::string>(args, "type");
  const sound_type_e type_enum = SoundManager::StrToPlatformEnum(type);
  int max_volume = GetMaxVolume(type_enum);
  int value;

  int ret = sound_manager_get_volume(type_enum, &value);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to get volume: %d", ret);
    // TODO: throw UnknownException("Failed to get volume");
  }

  double volume = ConvertToSystemVolume(max_volume, value);
  LoggerD("volume: %lf, maxVolume: %d, value: %d", volume, max_volume, value);

  return volume;
}

void SoundManager::soundModeChangedCb(keynode_t*, void* user_data)
{
  LOGD("enter");
  if (user_data == nullptr) {
    LoggerE("Invalid callback data!");
    return;
  }
  SoundManager* self = static_cast<SoundManager*>(user_data);
  std::string soundModeType = self->GetSoundMode();
  //TODO: ERROR CHECK
  if (self->soundModeListener) {
    self->soundModeListener->OnSoundModeChange(soundModeType);
  } else {
    LOGE("No SoundModeListener attached");
  }
}

bool SoundManager::SetSoundModeChangeListener(SoundManagerSoundModeChangedListener* listener) {
  soundModeListener = listener;
  if (soundModeChangeListening)
    return true;
  int status = vconf_notify_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, SoundManager::soundModeChangedCb, this);
  if (VCONF_OK == status) {
    soundModeChangeListening = true;
    return true;
  }
  return false;
}

bool SoundManager::UnsetSoundModeChangeListener() {
  soundModeListener = nullptr;
  if (!soundModeChangeListening) {
    return true;
  }
  int status = vconf_ignore_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, SoundManager::soundModeChangedCb);
  if (VCONF_OK == status) {
    soundModeChangeListening = false;
    return true;
  }
  return false;
}

void SoundManager::SetVolumeChangeListener() {
  if (!is_volume_change_listener_) {
    int ret = sound_manager_set_volume_changed_cb(
        [](sound_type_e type, unsigned int value, void* ud) {
          return static_cast<SoundManager*>(ud)
              ->VolumeChangeCallback(type, value);
        },
        static_cast<void*>(this));

    if (ret != SOUND_MANAGER_ERROR_NONE) {
      LoggerE("Failed to set volume changed callback: error code: %d", ret);
      // TODO: SoundUtil::throwSoundException(ret, "Failed to set volume changed
      // callback");
    }

    is_volume_change_listener_ = true;
  }
}

void SoundManager::UnsetVolumeChangeListener() {
  if (!is_volume_change_listener_) {
    return;
  }

  int ret = sound_manager_unset_volume_changed_cb();
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to unset volume changed callback");
    // TODO: SoundUtil::throwSoundException(ret, "Failed to unset volume changed
    // callback");
  }

  is_volume_change_listener_ = false;
}

}  // namespace sound
}  // namespace extension
