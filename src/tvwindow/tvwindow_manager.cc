// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvwindow/tvwindow_manager.h"

namespace extension {
namespace tvwindow {

TVWindowManager::TVWindowManager() {
	LOGD("Enter");
}

TVWindowManager& TVWindowManager::getInstance() {
    static TVWindowManager manager;
    return manager;
}
}  // namespace tvwindow
}  // namespace extension
