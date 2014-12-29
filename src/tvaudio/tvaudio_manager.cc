// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <avoc_defs.h>
#include <avoc.h>
#include <sound_manager.h>
#include <sound_manager_product.h>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "tvaudio/tvaudio_manager.h"

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
        m_volume_change_listener(NULL) {
    LOGD("Enter");
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
    try {
        if (!getInstance().m_volume_change_listener) {
            LOGD("Listener is null. Ignoring");
            return;
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
}

}  // namespace tvaudio
}  // namespace extension

