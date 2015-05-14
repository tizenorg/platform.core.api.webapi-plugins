// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_INPUTDEVICE_INPUTDEVICE_EXTENSION_H_
#define SRC_INPUTDEVICE_INPUTDEVICE_EXTENSION_H_

#include "common/extension.h"

namespace extension {
namespace inputdevice {

class InputDeviceExtension : public common::Extension {
 public:
    InputDeviceExtension();
    virtual ~InputDeviceExtension();

 private:
    virtual common::Instance* CreateInstance();
};

}  // namespace inputdevice
}  // namespace extension

#endif  // SRC_INPUTDEVICE_INPUTDEVICE_EXTENSION_H_

