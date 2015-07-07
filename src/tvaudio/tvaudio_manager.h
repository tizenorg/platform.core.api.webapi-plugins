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

#include <glib.h>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifndef SRC_TVAUDIO_TVAUDIO_MANAGER_H_
#define SRC_TVAUDIO_TVAUDIO_MANAGER_H_


#include <sys/types.h>
#include "common/platform_result.h"

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
    common::PlatformResult setMute(bool mute);
    common::PlatformResult isMute(bool &isMute);
    common::PlatformResult setVolume(u_int16_t volume);
    common::PlatformResult setVolumeUp();
    common::PlatformResult setVolumeDown();
    common::PlatformResult playSound(const std::string &type);
    common::PlatformResult getVolume(u_int16_t &volume);
    common::PlatformResult getOutputMode(AudioOutputMode &mode);
    common::PlatformResult registerVolumeChangeListener(VolumeChangeListener* listener);
    common::PlatformResult unregisterVolumeChangeListener();
    static void volumeChangeCallback(unsigned int volume, void* user_data);
    static gboolean onVolumeChange(gpointer user_data);

    // Non-copyable, -assignable, -movable
    AudioControlManager(const AudioControlManager &) = delete;
    AudioControlManager(AudioControlManager &&) = delete;
    AudioControlManager& operator=(const AudioControlManager &) & = delete;
    AudioControlManager& operator=(AudioControlManager &&) & = delete;

    static AudioControlManager& getInstance();

    virtual ~AudioControlManager();

 private:
    u_int16_t m_volume_step;
    VolumeChangeListener* m_volume_change_listener;
    AudioControlManager();

    static const int CHUNK = 768;
    static const int SAMPLING_FREQ = 44100;

    static void* play(void* _args);

    bool m_playThreadIdInit;
    pthread_t m_playThreadId;

    static std::unique_ptr<std::vector<char>>
        loadFile(const std::string& filename);

    //  key : sound file path
    const std::map<const std::string, const std::string> SoundMap = {
        {"MOVE",          "/opt/usr/share/settings/Ringtones/move.pcm"},
        {"UP",            "/opt/usr/share/settings/Ringtones/move.pcm"},
        {"DOWN",          "/opt/usr/share/settings/Ringtones/move.pcm"},
        {"LEFT",          "/opt/usr/share/settings/Ringtones/move.pcm"},
        {"RIGHT",         "/opt/usr/share/settings/Ringtones/move.pcm"},
        {"PAGE_LEFT",     "/opt/usr/share/settings/Ringtones/move.pcm"},
        {"PAGE_RIGHT",    "/opt/usr/share/settings/Ringtones/move.pcm"},

        {"BACK",          "/opt/usr/share/settings/Ringtones/back.pcm"},
        {"SELECT",        "/opt/usr/share/settings/Ringtones/select.pcm"},
        {"CANCEL",        "/opt/usr/share/settings/Ringtones/cancel.pcm"},
        {"WARNING",       "/opt/usr/share/settings/Ringtones/enter.pcm"},
        {"KEYPAD",        "/opt/usr/share/settings/Ringtones/keypad.pcm"},
        {"KEYPAD_ENTER",  "/opt/usr/share/settings/Ringtones/enter.pcm"},
        {"KEYPAD_DEL",    "/opt/usr/share/settings/Ringtones/del.pcm"},
        {"PREPARING",     "/opt/usr/share/settings/Ringtones/preparing.pcm"},
    };
    struct PlayData {
        std::atomic<bool> stopSound;
        std::string beep_type;
        std::string filename;
    };
    PlayData m_playData;
};

}  // namespace tvaudio
}  // namespace extension

#endif  // SRC_TVAUDIO_TVAUDIO_MANAGER_H_

