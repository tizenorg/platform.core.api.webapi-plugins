// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOUND_SOUND_MANAGER_H_
#define SOUND_SOUND_MANAGER_H_

#include <sound_manager.h>

#include "common/picojson.h"
#include "common/platform_result.h"
#include <vconf.h>

namespace extension {
namespace sound {

class SoundManagerSoundModeChangedListener {
 public:
  virtual void OnSoundModeChange(const std::string& newmode) = 0;
};

class SoundManager {
 public:
  static SoundManager* GetInstance();

  common::PlatformResult GetSoundMode(std::string* sound_mode_type);
  common::PlatformResult SetVolume(const picojson::object& args);
  common::PlatformResult GetVolume(const picojson::object& args,
                                   double* volume);
  common::PlatformResult SetSoundModeChangeListener(
      SoundManagerSoundModeChangedListener* listener);
  common::PlatformResult UnsetSoundModeChangeListener();
  common::PlatformResult SetVolumeChangeListener();
  common::PlatformResult UnsetVolumeChangeListener();
  void GetDeviceList(sound_device_mask_e mask, picojson::object& out);
  void DeviceChangeCB(sound_device_h device, bool is_connected, bool check_connection);
  common::PlatformResult AddDeviceStateChangeListener();
  common::PlatformResult RemoveDeviceStateChangeListener();

 private:
  SoundManager();
  virtual ~SoundManager();

  std::map<sound_type_e, int> max_volume_map_;
  bool is_volume_change_listener_;

  static const std::map<std::string, sound_type_e> platform_enum_map_;

  void FillMaxVolumeMap();
  common::PlatformResult GetMaxVolume(sound_type_e type, int* max_volume);
  void VolumeChangeCallback(sound_type_e type, unsigned int value);

  static common::PlatformResult StrToPlatformEnum(const std::string& key,
                                                  sound_type_e* sound_type);
  static common::PlatformResult PlatformEnumToStr(const sound_type_e value,
                                                  std::string* sound_type);
                                                  
  common::PlatformResult GetDeviceInfo(sound_device_h device,
                                       bool is_connected,
                                       bool check_connection,
                                       picojson::object* obj);
  common::PlatformResult IsDeviceConnected(sound_device_type_e type,
                                           sound_device_io_direction_e direction,
                                           picojson::object* obj);
  static std::string SoundDeviceTypeToString(sound_device_type_e type);
  static std::string SoundIOTypeToString(sound_device_io_direction_e type);
  static double ConvertToSystemVolume(int max_volume, int volume);
  static void soundModeChangedCb(keynode_t* node, void* user_data);
  bool soundModeChangeListening;
  bool sound_device_change_listener_;
  SoundManagerSoundModeChangedListener* soundModeListener;
};

}  // namespace sound
}  // namespace extension

#endif  // SOUND_SOUND_MANAGER_H_
