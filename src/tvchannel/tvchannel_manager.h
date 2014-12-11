// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_
#define SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_

namespace tvchannel {

class TVChannelManager {
 public:
    static TVChannelManager& getInstance();

 private:
    // Not copyable, assignable, movable
    TVChannelManager(TVChannelManager const&);
    void operator=(TVChannelManager const&);
    TVChannelManager(TVChannelManager &&);
};

}  // namespace tvchannel

#endif  // SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_
