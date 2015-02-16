// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <glib.h>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

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
    bool playSound(const std::string &type);
    u_int16_t getVolume();
    AudioOutputMode getOutputMode();
    void registerVolumeChangeListener(VolumeChangeListener* listener);
    void unregisterVolumeChangeListener();
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

