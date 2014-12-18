// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVAUDIO_TVAUDIO_MANAGER_H_
#define SRC_TVAUDIO_TVAUDIO_MANAGER_H_

namespace extension {
namespace tvaudio {

class AudioControlManager {
 public:
    // Not copyable, assignable, movable
    AudioControlManager(AudioControlManager const&) = delete;
    void operator=(AudioControlManager const&) = delete;
    AudioControlManager(AudioControlManager &&) = delete;

    static AudioControlManager& getInstance();
 private:
    AudioControlManager();
    virtual ~AudioControlManager();
};

}  // namespace tvaudio
}  // namespace extension

#endif  // SRC_TVAUDIO_TVAUDIO_MANAGER_H_

