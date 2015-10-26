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

#include "sound/sound_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "sound_manager.h"

namespace extension {
namespace sound {

using namespace common;
using namespace extension::sound;

SoundInstance::SoundInstance(): manager_(*this) {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SoundInstance::x, this, _1, _2));
  REGISTER_SYNC("SoundManager_setVolume", SoundManagerSetVolume);
  REGISTER_SYNC("SoundManager_unsetSoundModeChangeListener", SoundManagerUnsetSoundModeChangeListener);
  REGISTER_SYNC("SoundManager_getVolume", SoundManagerGetVolume);
  REGISTER_SYNC("SoundManager_unsetVolumeChangeListener", SoundManagerUnsetVolumeChangeListener);
  REGISTER_SYNC("SoundManager_setSoundModeChangeListener", SoundManagerSetSoundModeChangeListener);
  REGISTER_SYNC("SoundManager_setVolumeChangeListener", SoundManagerSetVolumeChangeListener);
  REGISTER_SYNC("SoundManager_getSoundMode", SoundManagerGetSoundMode);
  REGISTER_SYNC("SoundManager_getConnectedDeviceList", SoundManagerGetConnectedDeviceList);
  REGISTER_SYNC("SoundManager_getActivatedDeviceList", SoundManagerGetActivatedDeviceList);
  REGISTER_SYNC("SoundManager_addDeviceStateChangeListener",
                SoundManagerAddDeviceStateChangeListener);
  REGISTER_SYNC("SoundManager_removeDeviceStateChangeListener",
                SoundManagerRemoveDeviceStateChangeListener);
  #undef REGISTER_SYNC
}

SoundInstance::~SoundInstance() {
  LoggerD("Enter");
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void SoundInstance::SoundManagerGetSoundMode(const picojson::value& args,
                                             picojson::object& out) {
  LoggerD("Enter");
  std::string sound_mode_type;
  PlatformResult status = manager_.GetSoundMode(&sound_mode_type);

  if (status.IsSuccess()) {
    ReportSuccess(picojson::value(sound_mode_type), out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void SoundInstance::SoundManagerSetVolume(const picojson::value& args,
                                          picojson::object& out) {
  LoggerD("Enter");
  PlatformResult status = manager_.SetVolume(args.get<picojson::object>());

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}


void SoundInstance::SoundManagerGetVolume(const picojson::value& args,
                                          picojson::object& out) {
  LoggerD("Enter");
  double volume;
  PlatformResult status =
      manager_.GetVolume(args.get<picojson::object>(), &volume);

  if (status.IsSuccess()) {
    ReportSuccess(picojson::value(volume), out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void SoundInstance::SoundManagerSetSoundModeChangeListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  PlatformResult status = manager_.SetSoundModeChangeListener(this);

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void SoundInstance::SoundManagerUnsetSoundModeChangeListener(const picojson::value& args, picojson::object& out) {
  PlatformResult status = manager_.UnsetSoundModeChangeListener();

  LoggerD("Enter");

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void SoundInstance::OnSoundModeChange(const std::string& newmode)
{
  LoggerD("Enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  picojson::value result = picojson::value(newmode);
  ReportSuccess(result, obj);
  obj["listenerId"] = picojson::value("SoundModeChangeListener");
  LoggerD("Posting: %s", event.serialize().c_str());
  Instance::PostMessage(this, event.serialize().c_str());
}


void SoundInstance::SoundManagerSetVolumeChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  PlatformResult status = manager_.SetVolumeChangeListener();

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void SoundInstance::SoundManagerUnsetVolumeChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  PlatformResult status = manager_.UnsetVolumeChangeListener();

  if (status.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(status, &out);
  }
}

void SoundInstance::SoundManagerGetConnectedDeviceList(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");
  manager_.GetDeviceList(SOUND_DEVICE_ALL_MASK, out);
}

void SoundInstance::SoundManagerGetActivatedDeviceList(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");
  manager_.GetDeviceList(SOUND_DEVICE_STATE_ACTIVATED_MASK, out);
}

void SoundInstance::SoundManagerAddDeviceStateChangeListener(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");
  PlatformResult result = manager_.AddDeviceStateChangeListener();

  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(result, &out);
  }
}

void SoundInstance::SoundManagerRemoveDeviceStateChangeListener(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");
  PlatformResult result = manager_.RemoveDeviceStateChangeListener();

  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Failed");
    ReportError(result, &out);
  }
}

#undef CHECK_EXIST

} // namespace sound
} // namespace extension
