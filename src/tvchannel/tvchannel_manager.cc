// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_manager.h"

namespace tvchannel {

TVChannelManager& TVChannelManager::getInstance() {
    static TVChannelManager manager;
    return manager;
}

}
