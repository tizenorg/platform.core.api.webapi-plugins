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

SoundInstance &SoundInstance::GetInstance() {
  static SoundInstance instance;
  return instance;
}

void SoundInstance::SoundManagerGetSoundMode(const picojson::value& args,
                                             picojson::object& out) {
  ReportSuccess(picojson::value(manager_->GetSoundMode()), out);
}

void SoundInstance::SoundManagerSetVolume(const picojson::value& args,
                                          picojson::object& out) {
  manager_->SetVolume(args.get<picojson::object>());

  ReportSuccess(out);
}


void SoundInstance::SoundManagerGetVolume(const picojson::value& args,
                                          picojson::object& out) {
  ReportSuccess(picojson::value(static_cast<double>(
                    manager_->GetVolume(args.get<picojson::object>()))),
                out);
}

void SoundInstance::SoundManagerSetSoundModeChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("enter");
  bool status = manager_->SetSoundModeChangeListener(this);

  if (status)
    ReportSuccess(out);
  else
    ReportError(out);
}

void SoundInstance::SoundManagerUnsetSoundModeChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("enter");
  bool status = manager_->UnsetSoundModeChangeListener();

  if (status)
    ReportSuccess(out);
  else
    ReportError(out);
}

void SoundInstance::OnSoundModeChange(const std::string& newmode)
{
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  picojson::value result = picojson::value(newmode);
  ReportSuccess(result, obj);
  obj["listenerId"] = picojson::value("SoundModeChangeListener");
  LoggerD("Posting: %s", event.serialize().c_str());
  PostMessage(event.serialize().c_str());
}


void SoundInstance::SoundManagerSetVolumeChangeListener(
    const picojson::value& args, picojson::object& out) {
  manager_->SetVolumeChangeListener();

  ReportSuccess(out);
}

void SoundInstance::SoundManagerUnsetVolumeChangeListener(
    const picojson::value& args, picojson::object& out) {
  manager_->UnsetVolumeChangeListener();

  ReportSuccess(out);
}

#undef CHECK_EXIST

} // namespace sound
} // namespace extension
