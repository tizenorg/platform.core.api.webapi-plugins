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

}  // namespace tvaudio
}  // namespace extension

