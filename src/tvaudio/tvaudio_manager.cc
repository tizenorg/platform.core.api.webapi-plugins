// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvaudio/tvaudio_manager.h"

namespace extension {
namespace tvaudio {

AudioControlManager::AudioControlManager() {
}

AudioControlManager::~AudioControlManager() {
}

AudioControlManager& AudioControlManager::getInstance() {
  static AudioControlManager instance;
  return instance;
}

void AudioControlManager::setMute(bool mute) {
}

bool AudioControlManager::isMute() {
    return true;
}

void AudioControlManager::setVolume(u_int16_t volume) {
}

void AudioControlManager::setVolumeUp() {
}

void AudioControlManager::setVolumeDown() {
}

u_int16_t AudioControlManager::getVolume() {
    return 10;
}

AudioOutputMode AudioControlManager::getOutputMode() {
    return PCM;
}

}  // namespace tvaudio
}  // namespace extension

