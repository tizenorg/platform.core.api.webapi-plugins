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

#ifndef SOUND_SOUND_INSTANCE_H_
#define SOUND_SOUND_INSTANCE_H_

#include "common/extension.h"
#include "sound_manager.h"

namespace extension {
namespace sound {

class SoundInstance : public common::ParsedInstance, public SoundManagerSoundModeChangedListener {
 public:
  SoundInstance();
  virtual ~SoundInstance();

 private:
  void SoundManagerSetVolume(const picojson::value& args, picojson::object& out);
  void SoundManagerUnsetSoundModeChangeListener(const picojson::value& args, picojson::object& out);
  void SoundManagerGetVolume(const picojson::value& args, picojson::object& out);
  void SoundManagerUnsetVolumeChangeListener(const picojson::value& args, picojson::object& out);
  void SoundManagerSetSoundModeChangeListener(const picojson::value& args, picojson::object& out);
  void SoundManagerSetVolumeChangeListener(const picojson::value& args, picojson::object& out);
  void SoundManagerGetSoundMode(const picojson::value& args, picojson::object& out);
  void SoundManagerGetConnectedDeviceList(const picojson::value& args, picojson::object& out);
  void SoundManagerGetActivatedDeviceList(const picojson::value& args, picojson::object& out);
  void SoundManagerAddDeviceStateChangeListener(const picojson::value& args, picojson::object& out);
  void SoundManagerRemoveDeviceStateChangeListener(
      const picojson::value& args, picojson::object& out);

  void OnSoundModeChange(const std::string& newmode);

  SoundManager manager_;
};

} // namespace sound
} // namespace extension

#endif // SOUND_SOUND_INSTANCE_H_
