// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sound/sound_manager.h"

#include <tizen/tizen.h>

#include "sound/sound_instance.h"
#include "common/logger.h"
#include "common/converter.h"
#include "common/platform_exception.h"

using namespace common;

namespace extension {
namespace sound {

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

SoundManager::SoundManager() {}

SoundManager::~SoundManager() {}

SoundManager* SoundManager::GetInstance() {
  static SoundManager instance;
  return &instance;
}

int SoundManager::GetSoundMode() {}

void SoundManager::SetVolume(const picojson::object& args) {}

double SoundManager::GetVolume(const picojson::object& args) {}

void SoundManager::SetSoundModeChangeListener(const picojson::object& args) {}

void SoundManager::UnsetSoundModeChangeListener() {}

void SoundManager::SetVolumeChangeListener(const picojson::object& args) {}

void SoundManager::UnsetVolumeChangeListener() {}

}  // namespace sound
}  // namespace extension
