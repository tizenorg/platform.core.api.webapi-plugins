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

#include "tvaudio/tvaudio_manager.h"

#include <avoc.h>
#include <avoc_defs.h>
#include <audio_io.h>
#include <sound_manager.h>
#include <sound_manager_product.h>

#include <fstream> // NOLINT (readability/streams)
// this flag is no longer enforced in the newest cpplint.py

#include <string>
#include <vector>

#include "common/logger.h"
#include "common/platform_exception.h"


namespace extension {
namespace tvaudio {

using common::UnknownException;
using common::InvalidValuesException;
using common::ErrorCode;

namespace {
const int AVOC_SUCCESS = 0;
const u_int16_t VOLUME_STEP = 1;
}

VolumeChangeListener::~VolumeChangeListener() {
    LoggerD("Enter");
}

AudioControlManager::AudioControlManager() :
    m_volume_step(VOLUME_STEP),
    m_volume_change_listener(NULL),
    m_playThreadIdInit(false) {
    LoggerD("Enter");
    m_playData.stopSound = false;
}

AudioControlManager::~AudioControlManager() {
    LoggerD("Enter");
}

AudioControlManager& AudioControlManager::getInstance() {
  static AudioControlManager instance;
  return instance;
}

common::PlatformResult AudioControlManager::setMute(bool mute) {
    LoggerD("Enter");
    int ret = sound_manager_set_master_mute(mute);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LoggerE("Failed to change mute state: %d", ret);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error. Failed to change mute state");
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::isMute(bool &isMute) {
    LoggerD("Enter");
    int ret = sound_manager_get_master_mute(&isMute);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LoggerE("Failed to get mute state: %d", ret);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error. Failed to get mute state");
    }
    LoggerD("Mute state: %d", isMute);
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::setVolume(u_int16_t volume) {
    LoggerD("Enter. Volume: %d", volume);
    if (volume > 100) {
        LoggerE("Invalid volume number");
        return common::PlatformResult(ErrorCode::INVALID_VALUES_ERR,
            "Invalid volume number");
    }
    int ret = sound_manager_set_master_volume(volume);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LoggerE("Failed to set volume: %d", ret);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error. Failed to set volume");
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::setVolumeUp() {
    LoggerD("Enter");
    common::PlatformResult ret(ErrorCode::NO_ERROR);
    u_int16_t currentVolume;
    ret = getVolume(currentVolume);
    if (ret.IsError())
        return ret;
    if (currentVolume < 100) {
        ret = setVolume(currentVolume + m_volume_step <= 100 ?
                currentVolume + m_volume_step : 100);
        if (ret.IsError())
            return ret;
    }
    bool muteState;
    ret = isMute(muteState);
    if (ret.IsError())
        return ret;
    if (muteState) {
        return setMute(false);
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::setVolumeDown() {
    LoggerD("Enter");
    common::PlatformResult ret(ErrorCode::NO_ERROR);
    u_int16_t currentVolume;
    ret = getVolume(currentVolume);
    if (ret.IsError())
        return ret;
    if (currentVolume > 0) {
        ret = setVolume(currentVolume >= m_volume_step ?
                currentVolume - m_volume_step : 0);
        if (ret.IsError())
            return ret;
    }
    bool muteState;
    ret = isMute(muteState);
    if (ret.IsError())
        return ret;
    if (muteState) {
        return setMute(false);
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::getVolume(u_int16_t &volume) {
    LoggerD("Enter");
    int tempVolume;
    int ret = sound_manager_get_master_volume(&tempVolume);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LoggerE("Failed to get volume: %d", ret);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error. Failed to get volume");
    }
    LoggerD("Volume: %d", volume);
    volume = tempVolume;
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::getOutputMode(AudioOutputMode &mode) {
    LoggerD("Enter");
    avoc_audio_format_e type;
    int ret = avoc_get_audio_format(&type);
    if (AVOC_SUCCESS != ret) {
        LoggerE("Failed to get audio output type: %d", ret);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error. Failed to get audio output type");
    }
    switch (type) {
        case AVOC_AUDIO_FORMAT_PCM:
            mode = AudioOutputMode::PCM;
            break;
        case AVOC_AUDIO_FORMAT_ES_DOLBY_DIGITAL:
            mode = AudioOutputMode::DOLBY;
            break;
        case AVOC_AUDIO_FORMAT_ES_DTS:
        case AVOC_AUDIO_FORMAT_NEO_ES_DTS:
            mode = AudioOutputMode::DTS;
            break;
        case AVOC_AUDIO_FORMAT_ES_AAC:
            mode = AudioOutputMode::AAC;
            break;
        default:
            LoggerE("Unexpected audio output type: %d", type);
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Unexecpted audio output type");
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::registerVolumeChangeListener(
            VolumeChangeListener* listener) {
    LoggerD("Enter");
    unregisterVolumeChangeListener();
    int r = sound_manager_set_master_volume_changed_cb(
            volumeChangeCallback, NULL);
    if (SOUND_MANAGER_ERROR_NONE != r) {
        LoggerE("Failed to add listener: %d", r);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to add listener");
    }
    m_volume_change_listener = listener;
    LoggerD("Added listener");
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult AudioControlManager::unregisterVolumeChangeListener() {
    LoggerD("Enter");
    int r = sound_manager_unset_master_volume_changed_cb();
    if (SOUND_MANAGER_ERROR_NONE != r) {
        LoggerW("Failed to remove listener: %d", r);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to remove listener");
    }
    m_volume_change_listener = NULL;
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

void AudioControlManager::volumeChangeCallback(
        unsigned int /*volume*/,
        void* /*user_data*/) {
    LoggerD("Enter");
    if (!g_idle_add(onVolumeChange, NULL)) {
        LoggerW("Failed to add to g_idle");
    }
}

gboolean AudioControlManager::onVolumeChange(gpointer /*user_data*/) {
    LoggerD("Enter");
    if (!getInstance().m_volume_change_listener) {
        LoggerD("Listener is null. Ignoring");
        return G_SOURCE_REMOVE;
    }
    u_int16_t val;
    common::PlatformResult ret = getInstance().getVolume(val);
    if (ret.IsError()) {
        LoggerE("Failed to retrieve volume level");
    }
    getInstance().m_volume_change_listener->onVolumeChangeCallback(val);
    return G_SOURCE_REMOVE;
}

/**
 * Play one of predefined sounds
 *
 * If sound is already played it is replaced by the new sound
 */
common::PlatformResult AudioControlManager::playSound(const std::string &type) {
    LoggerD("Enter");
    const auto beep = SoundMap.find(type);
    if (beep == SoundMap.end()) {
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Unknown error. Unknown beep type: " + type);
    }

    if (m_playThreadIdInit) {
        m_playData.stopSound = true;
        pthread_join(m_playThreadId, NULL);
        m_playData.stopSound = false;
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    m_playData.beep_type = beep->first;
    m_playData.filename = beep->second;

    if (0 == pthread_create(&m_playThreadId, &attr, play, &m_playData )) {
        m_playThreadIdInit = true;
    } else {
        LoggerE("Failed to create pthread");
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to create pthread to play sound");
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

void* AudioControlManager::play(void* play_data) {
    LoggerD("Enter");
    PlayData* pData = static_cast<PlayData*>(play_data);

    LoggerD("Beep type: %s", pData->beep_type.c_str());

    const std::string& filename = pData->filename;
    std::unique_ptr <std::vector <char>> pBuffer =
            AudioControlManager::loadFile(filename);

    audio_out_h handle;
    int error = audio_out_create(SAMPLING_FREQ,
                                AUDIO_CHANNEL_STEREO,
                                AUDIO_SAMPLE_TYPE_S16_LE,
                                SOUND_TYPE_NOTIFICATION,
                                &handle);
    if (AUDIO_IO_ERROR_NONE != error) {
        LoggerE("Failed to open audio output: %d", error);
        return NULL;
    }

    error = audio_out_prepare(handle);
    if (AUDIO_IO_ERROR_NONE != error) {
        LoggerE("Failed to open audio output: %d", error);
        audio_out_destroy(handle);
        return NULL;
    }

    int counter = 0;
    int dataLeftSize = pBuffer->size();
    while ((!pData->stopSound) && (dataLeftSize > 0)) {
        if ((dataLeftSize - AudioControlManager::CHUNK) < 0) {
            error = audio_out_write(handle,
                                    &(*pBuffer)[counter],
                                    dataLeftSize);
            if (dataLeftSize != error) {
                LoggerE("Failed to write to audio output: %d", error);
                audio_out_destroy(handle);
                return NULL;
            }
            break;
        } else {
            dataLeftSize = dataLeftSize - AudioControlManager::CHUNK;
            error = audio_out_write(handle,
                                    &(*pBuffer)[counter],
                                    AudioControlManager::CHUNK);
            if (AudioControlManager::CHUNK != error) {
                LoggerE("Failed to write to audio output: %d", error);
                audio_out_destroy(handle);
                return NULL;
            }
        }
        counter += AudioControlManager::CHUNK;
    }  // while
    audio_out_destroy(handle);
    return NULL;
}

std::unique_ptr <std::vector<char>>
AudioControlManager::loadFile(const std::string& filename) {
    LoggerD("Enter");
    std::unique_ptr<std::vector<char>> pBuffer(new std::vector<char>());
    std::ifstream file(filename.c_str(),
                    (std::ios::binary | std::ios::in | std::ios::ate));
    if (!file.is_open()) {
        LoggerE("Could not open file %s", filename.c_str());
        return std::unique_ptr< std::vector<char>>();
    }

    std::ifstream::pos_type size = file.tellg();
    if (size < 0) {
        file.close();
        LoggerE("Failed to open file %s - incorrect size", filename.c_str());
        return std::unique_ptr<std::vector<char>>();
    }

    LoggerD("resizing");
    pBuffer->resize(size);
    LoggerD("resized");
    file.seekg(0, std::ios::beg);
    if (!file.read(&(*pBuffer)[0], size)) {
        file.close();
        LoggerE("Failed to read audio file %s", filename.c_str());
        return std::unique_ptr <std::vector <char>>();
    }
    file.close();

    LoggerD("Got buffer");
    return pBuffer;
}

}  // namespace tvaudio
}  // namespace extension

