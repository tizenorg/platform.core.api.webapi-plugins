// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVAUDIO_TVAUDIO_MANAGER_H_
#define SRC_TVAUDIO_TVAUDIO_MANAGER_H_

#include <sys/types.h>

namespace extension {
namespace tvaudio {

enum AudioOutputMode {
  PCM = 0,
  DOLBY,
  DTS,
  AAC
};

class VolumeChangeListener {
 public:
    virtual void onVolumeChangeCallback(u_int16_t volume) = 0;
    virtual ~VolumeChangeListener();
};

class AudioControlManager {
 public:
    void setMute(bool mute);
    bool isMute();
    void setVolume(u_int16_t volume);
    void setVolumeUp();
    void setVolumeDown();
    u_int16_t getVolume();
    AudioOutputMode getOutputMode();
    void registerVolumeChangeListener(VolumeChangeListener* listener);
    void unregisterVolumeChangeListener();
    static void volumeChangeCallback(unsigned int volume, void* user_data);

    // Not copyable, assignable, movable
    AudioControlManager(AudioControlManager const&) = delete;
    void operator=(AudioControlManager const&) = delete;
    AudioControlManager(AudioControlManager &&) = delete;

    static AudioControlManager& getInstance();

 private:
    u_int16_t m_volume_step;
    VolumeChangeListener* m_volume_change_listener;

    AudioControlManager();
    virtual ~AudioControlManager();
};

}  // namespace tvaudio
}  // namespace extension

#endif  // SRC_TVAUDIO_TVAUDIO_MANAGER_H_

