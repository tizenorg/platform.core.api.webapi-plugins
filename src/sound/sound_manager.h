// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOUND_SOUND_MANAGER_H_
#define SOUND_SOUND_MANAGER_H_

#include <sound_manager.h>

#include "common/picojson.h"
#include <vconf.h>

namespace extension {
namespace sound {

class SoundManagerSoundModeChangedListener
{
public:
  virtual void OnSoundModeChange(const std::string& newmode) = 0;
};

class SoundManager {
 public:
  static SoundManager* GetInstance();

  std::string GetSoundMode();
  void SetVolume(const picojson::object& args);
  double GetVolume(const picojson::object& args);
  bool SetSoundModeChangeListener(SoundManagerSoundModeChangedListener* listener);
  bool UnsetSoundModeChangeListener();
  void SetVolumeChangeListener();
  void UnsetVolumeChangeListener();

 private:
  SoundManager();
  virtual ~SoundManager();

  std::map<sound_type_e, int> max_volume_map_;
  bool is_volume_change_listener_;

  static const std::map<std::string, sound_type_e> platform_enum_map_;

  void FillMaxVolumeMap();
  int GetMaxVolume(sound_type_e type);
  void VolumeChangeCallback(sound_type_e type, unsigned int value);

  static sound_type_e StrToPlatformEnum(const std::string& key);
  static std::string PlatformEnumToStr(const sound_type_e value);
  static double ConvertToSystemVolume(int max_volume, int volume);
  static void soundModeChangedCb(keynode_t *node, void *user_data);
  bool soundModeChangeListening;
  SoundManagerSoundModeChangedListener* soundModeListener;
};

}  // namespace sound
}  // namespace extension

#endif  // SOUND_SOUND_MANAGER_H_
