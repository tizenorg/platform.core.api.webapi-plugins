// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOUND_SOUND_INSTANCE_H_
#define SOUND_SOUND_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace sound {

class SoundInstance : public common::ParsedInstance {
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
};

} // namespace sound
} // namespace extension

#endif // SOUND_SOUND_INSTANCE_H_