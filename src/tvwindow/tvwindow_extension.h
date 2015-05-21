// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVWINDOW_TVWINDOW_EXTENSION_H_
#define SRC_TVWINDOW_TVWINDOW_EXTENSION_H_

#include "common/extension.h"
#include "tvwindow/tvwindow_manager.h"

namespace extension {
namespace tvwindow {

class TVWindowExtension : public common::Extension {
 public:
    TVWindowExtension();
    virtual ~TVWindowExtension();

    TVWindowManager& manager();

 private:
    // common::Extension implementation.
    virtual common::Instance* CreateInstance();
};

}  // namespace tvwindow
}  // namespace extension

#endif  // SRC_TVWINDOW_TVWINDOW_EXTENSION_H_
