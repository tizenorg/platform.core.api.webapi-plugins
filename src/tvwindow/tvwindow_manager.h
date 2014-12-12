// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVWINDOW_TVWINDOW_MANAGER_H_
#define SRC_TVWINDOW_TVWINDOW_MANAGER_H_

#include "common/logger.h"

namespace extension {
namespace tvwindow {

class TVWindowManager {
 public:
    static TVWindowManager& getInstance();

 private:
    TVWindowManager();
    // Not copyable, assignable, movable
    TVWindowManager(TVWindowManager const&);
    void operator=(TVWindowManager const&);
    TVWindowManager(TVWindowManager &&);
};

}  // namespace tvwindow
}  // namespace extension

#endif  // SRC_TVWINDOW_TVWINDOW_MANAGER_H_
