// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
};

}  // namespace tvaudio
}  // namespace extension

#endif  // SRC_TVAUDIO_TVAUDIO_INSTANCE_H_
