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

#ifndef SRC_TVAUDIO_TVAUDIO_INSTANCE_H_
#define SRC_TVAUDIO_TVAUDIO_INSTANCE_H_

#include "common/extension.h"

#include "tvaudio/tvaudio_manager.h"

namespace extension {
namespace tvaudio {

class TVAudioInstance :
        public common::ParsedInstance,
        public VolumeChangeListener {
 public:
    TVAudioInstance();
    virtual ~TVAudioInstance();

 private:
    void setMute(const picojson::value& args, picojson::object& out);
    void isMute(const picojson::value& args, picojson::object& out);
    void setVolume(const picojson::value& args, picojson::object& out);
    void setVolumeUp(const picojson::value& args, picojson::object& out);
    void setVolumeDown(const picojson::value& args, picojson::object& out);
    void getVolume(const picojson::value& args, picojson::object& out);
    void getOutputMode(const picojson::value& args, picojson::object& out);
    void setVolumeChangeListener(const picojson::value& args, picojson::object& out);
    void unsetVolumeChangeListener(const picojson::value& args, picojson::object& out);
    virtual void onVolumeChangeCallback(u_int16_t volume);
    void playSound(const picojson::value& args, picojson::object& out);
};

}  // namespace tvaudio
}  // namespace extension

#endif  // SRC_TVAUDIO_TVAUDIO_INSTANCE_H_
