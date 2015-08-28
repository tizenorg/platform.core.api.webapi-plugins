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

#include <functional>
#include <map>
#include <string>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

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
    LoggerD("Enter");
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
    REGISTER_SYNC("AudioControlManager_setVolumeChangeListener", setVolumeChangeListener);
    REGISTER_SYNC("AudioControlManager_unsetVolumeChangeListener", unsetVolumeChangeListener);
    REGISTER_SYNC("AudioControlManager_playSound", playSound);
    #undef REGISTER_SYNC
}

TVAudioInstance::~TVAudioInstance() {
    LoggerD("Enter");
}

void TVAudioInstance::setMute(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    bool mute = args.get("mute").get<bool>();
    common::PlatformResult result =
            AudioControlManager::getInstance().setMute(mute);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

void TVAudioInstance::isMute(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    bool mute;
    common::PlatformResult result =
            AudioControlManager::getInstance().isMute(mute);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else
        ReportSuccess(picojson::value(mute), out);
}

void TVAudioInstance::setVolume(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    double volume = args.get("volume").get<double>();
    common::PlatformResult result =
            AudioControlManager::getInstance().setVolume(volume);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

void TVAudioInstance::setVolumeUp(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    common::PlatformResult result =
            AudioControlManager::getInstance().setVolumeUp();
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

void TVAudioInstance::setVolumeDown(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    common::PlatformResult result =
            AudioControlManager::getInstance().setVolumeDown();
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

void TVAudioInstance::getVolume(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    u_int16_t volume;
    common::PlatformResult result =
            AudioControlManager::getInstance().getVolume(volume);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result = picojson::value(static_cast<double>(volume));
        ReportSuccess(result, out);
    }
}

void TVAudioInstance::getOutputMode(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    AudioOutputMode mode;
    common::PlatformResult result =
            AudioControlManager::getInstance().getOutputMode(mode);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else if (AudioOutputModeMap.find(mode) == AudioOutputModeMap.end()) {
        LoggerE("Unknown mode type: %d", mode);
        ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR,
            "Uknown audio output mode"), &out);
    } else {
        ReportSuccess(picojson::value(AudioOutputModeMap.at(mode)), out);
    }
}

void TVAudioInstance::setVolumeChangeListener(const picojson::value& args,
        picojson::object& out) {
    common::PlatformResult result =
            AudioControlManager::getInstance().registerVolumeChangeListener(this);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

void TVAudioInstance::unsetVolumeChangeListener(const picojson::value& args,
        picojson::object& out) {
    AudioControlManager::getInstance().unregisterVolumeChangeListener();
    ReportSuccess(out);
}

void TVAudioInstance::onVolumeChangeCallback(u_int16_t volume) {
  LoggerD("Enter");
    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    obj["listenerId"] = picojson::value("VolumeChangeCallback");
    obj["volume"] = picojson::value(static_cast<double>(volume));
    Instance::PostMessage(this, event.serialize().c_str());
}

void TVAudioInstance::playSound(const picojson::value& args,
        picojson::object& out) {
    const std::string& type = args.get("type").to_str();
    common::PlatformResult result =
            AudioControlManager::getInstance().playSound(type);
    if (result.IsError()) {
        LoggerD("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

}  // namespace tvaudio
}  // namespace extension
