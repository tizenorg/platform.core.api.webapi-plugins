// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sound/sound_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "sound_manager.h"

namespace extension {
namespace sound {

namespace {
// The privileges that required in Sound API
const std::string kPrivilegeSound = "";

} // namespace

using namespace common;
using namespace extension::sound;

SoundInstance::SoundInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SoundInstance::x, this, _1, _2));
  REGISTER_SYNC("SoundManager_setVolume", SoundManagerSetVolume);
  REGISTER_SYNC("SoundManager_unsetSoundModeChangeListener", SoundManagerUnsetSoundModeChangeListener);
  REGISTER_SYNC("SoundManager_getVolume", SoundManagerGetVolume);
  REGISTER_SYNC("SoundManager_unsetVolumeChangeListener", SoundManagerUnsetVolumeChangeListener);
  REGISTER_SYNC("SoundManager_setSoundModeChangeListener", SoundManagerSetSoundModeChangeListener);
  REGISTER_SYNC("SoundManager_setVolumeChangeListener", SoundManagerSetVolumeChangeListener);
  REGISTER_SYNC("SoundManager_getSoundMode", SoundManagerGetSoundMode);
  #undef REGISTER_SYNC

  manager_ = SoundManager::GetInstance();
}

SoundInstance::~SoundInstance() {
}



#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }


void SoundInstance::SoundManagerGetSoundMode(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void SoundInstance::SoundManagerSetVolume(const picojson::value& args,
                                          picojson::object& out) {
  manager_->SetVolume(args.get<picojson::object>());

  ReportSuccess(out);
}

void SoundInstance::SoundManagerGetVolume(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void SoundInstance::SoundManagerSetSoundModeChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void SoundInstance::SoundManagerUnsetSoundModeChangeListener(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void SoundInstance::SoundManagerSetVolumeChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void SoundInstance::SoundManagerUnsetVolumeChangeListener(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}


#undef CHECK_EXIST

} // namespace sound
} // namespace extension
