// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVAUDIO_TVAUDIO_EXTENSION_H_
#define SRC_TVAUDIO_TVAUDIO_EXTENSION_H_

#include "common/extension.h"
#include "tvaudio/tvaudio_manager.h"

namespace extension {
namespace tvaudio {

class TVAudioExtension : public common::Extension {
 public:
    TVAudioExtension();
    virtual ~TVAudioExtension();

    AudioControlManager& manager();

 private:
    virtual common::Instance* CreateInstance();
};

}  // namespace tvaudio
}  // namespace extension

#endif  // SRC_TVAUDIO_TVAUDIO_EXTENSION_H_

