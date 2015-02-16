// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

namespace {
const int AVOC_SUCCESS = 0;
const u_int16_t VOLUME_STEP = 1;
}

VolumeChangeListener::~VolumeChangeListener() {
    LOGD("Enter");
}

AudioControlManager::AudioControlManager() :
    m_volume_step(VOLUME_STEP),
    m_volume_change_listener(NULL),
    m_playThreadIdInit(false) {
    LOGD("Enter");
    m_playData.stopSound = false;
}

AudioControlManager::~AudioControlManager() {
    LOGD("Enter");
}

AudioControlManager& AudioControlManager::getInstance() {
  static AudioControlManager instance;
  return instance;
}

void AudioControlManager::setMute(bool mute) {
    LOGD("Enter. Mute: %d", mute);
    int ret = sound_manager_set_master_mute(mute);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LOGE("Failed to change mute state: %d", ret);
        throw UnknownException("Failed to change mute state");
    }
}

bool AudioControlManager::isMute() {
    LOGD("Enter");
    bool muteState;
    int ret = sound_manager_get_master_mute(&muteState);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LOGE("Failed to get mute state: %d", ret);
        throw UnknownException("Failed to get mute state");
    }
    LOGD("Mute state: %d", muteState);
    return muteState;
}

void AudioControlManager::setVolume(u_int16_t volume) {
    LOGD("Enter. Volume: %d", volume);
    if (volume > 100) {
        LOGE("Invalid volume number");
        throw InvalidValuesException("Invalid volume number");
    }
    int ret = sound_manager_set_master_volume(volume);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LOGE("Failed to set volume: %d", ret);
        throw UnknownException("Failed to set volume");
    }
}

void AudioControlManager::setVolumeUp() {
    LOGD("Enter");
    u_int16_t currentVolume = getVolume();
    if (currentVolume < 100) {
        setVolume(currentVolume + m_volume_step <= 100 ?
                currentVolume + m_volume_step : 100);
    }
    if (isMute()) {
        setMute(false);
    }
}

void AudioControlManager::setVolumeDown() {
    LOGD("Enter");
    u_int16_t currentVolume = getVolume();
    if (currentVolume > 0) {
        setVolume(currentVolume >= m_volume_step ?
                currentVolume - m_volume_step : 0);
    }
    if (isMute()) {
        setMute(false);
    }
}

u_int16_t AudioControlManager::getVolume() {
    LOGD("Enter");
    int volume;
    int ret = sound_manager_get_master_volume(&volume);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
        LOGE("Failed to get volume: %d", ret);
        throw UnknownException("Failed to get volume");
    }
    LOGD("Volume: %d", volume);
    return volume;
}

AudioOutputMode AudioControlManager::getOutputMode() {
    LOGD("Enter");
    avoc_audio_format_e type;
    int ret = avoc_get_audio_format(&type);
    if (AVOC_SUCCESS != ret) {
        LOGE("Failed to get audio output type: %d", ret);
        throw UnknownException("Failed to get audio output type");
    }
    switch (type) {
        case AVOC_AUDIO_FORMAT_PCM:
            return AudioOutputMode::PCM;
        case AVOC_AUDIO_FORMAT_ES_DOLBY_DIGITAL:
            return AudioOutputMode::DOLBY;
        case AVOC_AUDIO_FORMAT_ES_DTS:
        case AVOC_AUDIO_FORMAT_NEO_ES_DTS:
            return AudioOutputMode::DTS;
        case AVOC_AUDIO_FORMAT_ES_AAC:
            return AudioOutputMode::AAC;
        default:
            LOGE("Unexpected audio output type: %d", type);
            throw UnknownException("Unexecpted audio output type");
    }
}

void AudioControlManager::registerVolumeChangeListener(
            VolumeChangeListener* listener) {
    LOGD("Enter");
    unregisterVolumeChangeListener();
    int r = sound_manager_set_master_volume_changed_cb(
            volumeChangeCallback, NULL);
    if (SOUND_MANAGER_ERROR_NONE != r) {
        LOGE("Failed to add listener: %d", r);
        throw UnknownException("Failed to add listener");
    }
    m_volume_change_listener = listener;
    LOGD("Added listener");
}

void AudioControlManager::unregisterVolumeChangeListener() {
    LOGD("Enter");
    int r = sound_manager_unset_master_volume_changed_cb();
    if (SOUND_MANAGER_ERROR_NONE != r) {
        LOGW("Failed to remove listener: %d", r);
    }
    m_volume_change_listener = NULL;
}

void AudioControlManager::volumeChangeCallback(
        unsigned int /*volume*/,
        void* /*user_data*/) {
    LOGD("Enter");
    if (!g_idle_add(onVolumeChange, NULL)) {
        LOGW("Failed to add to g_idle");
    }
}

gboolean AudioControlManager::onVolumeChange(gpointer /*user_data*/) {
    LOGD("Enter");
    try {
        if (!getInstance().m_volume_change_listener) {
            LOGD("Listener is null. Ignoring");
            return G_SOURCE_REMOVE;
        }
        u_int16_t val;
        try {
            val = getInstance().getVolume();
        } catch (...) {
            LOGE("Failed to retrieve volume level");
        }
        getInstance().m_volume_change_listener->onVolumeChangeCallback(val);
    } catch (...) {
        LOGE("Failed to call callback");
    }
    return G_SOURCE_REMOVE;
}

/**
 * Play one of predefined sounds
 *
 * If sound is already played it is replaced by the new sound
 *
 * @return {bool} true if successful, false otherwise
 */
bool AudioControlManager::playSound(const std::string &type) {
    LOGD("Enter");
    const auto beep = SoundMap.find(type);
    if (beep == SoundMap.end()) {
        throw UnknownException("Unknown beep type: " + type);
    }

    void *status;
    if (m_playThreadIdInit) {
        m_playData.stopSound = true;
        pthread_join(m_playThreadId, &status);
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
        LOGE("Failed to create pthread");
        throw UnknownException("Failed to create pthread to play sound");
    }
    return true;
}

void* AudioControlManager::play(void* play_data) {
    LOGD("Enter");
    PlayData* pData = static_cast<PlayData*>(play_data);

    LOGD("Beep type: %d", pData->beep_type.c_str());

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
        LOGE("Failed to open audio output: %d", error);
        return NULL;
    }

    error = audio_out_prepare(handle);
    if (AUDIO_IO_ERROR_NONE != error) {
        LOGE("Failed to open audio output: %d", error);
        audio_out_destroy(handle);
        return NULL;
    }

    int counter = 0;
    int dataLeftSize = pBuffer->size();
    while ((!pData->stopSound) && (dataLeftSize > 0)) {
        if ((dataLeftSize - AudioControlManager::CHUNK) < 0) {
            dataLeftSize = dataLeftSize;
            error = audio_out_write(handle,
                                    &(*pBuffer)[counter],
                                    dataLeftSize);
            if (dataLeftSize != error) {
                LOGE("Failed to write to audio output: %d", error);
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
                LOGE("Failed to write to audio output: %d", error);
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
    LOGD("Enter");
    std::unique_ptr<std::vector<char>> pBuffer(new std::vector<char>());
    std::ifstream file(filename.c_str(),
                    (std::ios::binary | std::ios::in | std::ios::ate));
    if (!file.is_open()) {
        LOGE("Could not open file %s", filename.c_str());
        return std::unique_ptr< std::vector<char>>();
    }

    std::ifstream::pos_type size = file.tellg();
    if (size < 0) {
        file.close();
        LOGE("Failed to open file %s - incorrect size", filename.c_str());
        return std::unique_ptr<std::vector<char>>();
    }

    LOGD("resizing");
    pBuffer->resize(size);
    LOGD("resized");
    file.seekg(0, std::ios::beg);
    if (!file.read(&(*pBuffer)[0], size)) {
        file.close();
        LOGE("Failed to read audio file %s", filename.c_str());
        return std::unique_ptr <std::vector <char>>();
    }
    file.close();

    LOGD("Got buffer");
    return pBuffer;
}

}  // namespace tvaudio
}  // namespace extension

