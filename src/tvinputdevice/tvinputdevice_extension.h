// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVINPUTDEVICE_TVINPUTDEVICE_EXTENSION_H_
#define SRC_TVINPUTDEVICE_TVINPUTDEVICE_EXTENSION_H_

#include "common/extension.h"

namespace extension {
namespace tvinputdevice {

class TVInputDeviceExtension : public common::Extension {
 public:
    TVInputDeviceExtension();
    virtual ~TVInputDeviceExtension();

 private:
    virtual common::Instance* CreateInstance();
};

}  // namespace tvinputdevice
}  // namespace extension

#endif  // SRC_TVINPUTDEVICE_TVINPUTDEVICE_EXTENSION_H_

