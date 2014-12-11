// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVDISPLAY_TVDISPLAY_EXTENSION_H_
#define SRC_TVDISPLAY_TVDISPLAY_EXTENSION_H_

#include "common/extension.h"

namespace extension {
namespace tvdisplay {

class TVDisplayExtension : public common::Extension {
 public:
    TVDisplayExtension();
    virtual ~TVDisplayExtension();

 private:
    virtual common::Instance* CreateInstance();
};

}  // namespace tvdisplay
}  // namespace extension

#endif  // SRC_TVDISPLAY_TVDISPLAY_EXTENSION_H_
