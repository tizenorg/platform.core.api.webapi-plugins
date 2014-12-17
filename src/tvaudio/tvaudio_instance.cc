// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <functional>
#include <map>
#include <string>

#include "common/picojson.h"

#include "tvaudio/tvaudio_instance.h"
#include "tvaudio/tvaudio_manager.h"

namespace extension {
namespace tvaudio {

namespace {
const std::map<AudioOutputMode, std::string> AudioOutputModeMap = {
    {PCM, "PCM"},
    {DOLBY, "DOLBY"},
    {DTS, "DTS"},
    {AAC, "AAC"}
};
}  // namespace

TVAudioInstance::TVAudioInstance() {
    using std::placeholders::_1;
    using std::placeholders::_2;
    #define REGISTER_SYNC(c, x) \
      RegisterSyncHandler(c, std::bind(&TVAudioInstance::x, this, _1, _2));
    REGISTER_SYNC("AudioControlManager_setMute", setMute);
    REGISTER_SYNC("AudioControlManager_isMute", isMute);
    REGISTER_SYNC("AudioControlManager_setVolume", setVolume);
    REGISTER_SYNC("AudioControlManager_setVolumeUp", setVolumeUp);
    REGISTER_SYNC("AudioControlManager_setVolumeDown", setVolumeDown);
    REGISTER_SYNC("AudioControlManager_getVolume", getVolume);
    REGISTER_SYNC("AudioControlManager_getOutputMode", getOutputMode);
    #undef REGISTER_SYNC
}

TVAudioInstance::~TVAudioInstance() {}

void TVAudioInstance::setMute(const picojson::value& args,
        picojson::object& out) {
    bool mute = args.get("mute").get<bool>();
    AudioControlManager::getInstance().setMute(mute);
    ReportSuccess(out);
}

void TVAudioInstance::isMute(const picojson::value& args,
        picojson::object& out) {
    bool mute = AudioControlManager::getInstance().isMute();
    ReportSuccess(picojson::value(mute), out);
}

void TVAudioInstance::setVolume(const picojson::value& args,
        picojson::object& out) {
    double volume = args.get("volume").get<double>();
    AudioControlManager::getInstance().setVolume(volume);
    ReportSuccess(out);
}

void TVAudioInstance::setVolumeUp(const picojson::value& args,
        picojson::object& out) {
    AudioControlManager::getInstance().setVolumeUp();
    ReportSuccess(out);
}

void TVAudioInstance::setVolumeDown(const picojson::value& args,
        picojson::object& out) {
    AudioControlManager::getInstance().setVolumeDown();
    ReportSuccess(out);
}

void TVAudioInstance::getVolume(const picojson::value& args,
        picojson::object& out) {
    u_int16_t volume = AudioControlManager::getInstance().getVolume();
    ReportSuccess(picojson::value(static_cast<double>(volume)), out);
}

void TVAudioInstance::getOutputMode(const picojson::value& args,
        picojson::object& out) {
    AudioOutputMode mode = AudioControlManager::getInstance().getOutputMode();
    ReportSuccess(picojson::value(AudioOutputModeMap.at(mode)), out);
}

}  // namespace tvaudio
}  // namespace extension
