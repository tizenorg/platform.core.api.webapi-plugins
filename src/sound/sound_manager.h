// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOUND_SOUND_MANAGER_H_
#define SOUND_SOUND_MANAGER_H_

#include <sound_manager.h>

#include "common/picojson.h"

namespace extension {
namespace sound {

class SoundManager {
 public:
  static SoundManager* GetInstance();

  int GetSoundMode();
  void SetVolume(const picojson::object& args);
  double GetVolume(const picojson::object& args);
  void SetSoundModeChangeListener(const picojson::object& args);
  void UnsetSoundModeChangeListener();
  void SetVolumeChangeListener(const picojson::object& args);
  void UnsetVolumeChangeListener();

 private:
  SoundManager();
  virtual ~SoundManager();

  static const std::map<std::string, sound_type_e> platform_enum_map_;

  static sound_type_e StrToPlatformEnum(const std::string& key);
  static std::string PlatformEnumToStr(const sound_type_e value);
};

}  // namespace sound
}  // namespace extension

#endif  // SOUND_SOUND_MANAGER_H_
