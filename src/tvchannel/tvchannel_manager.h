// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_
#define SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_

#include <string>
#include <memory>
#include <TVServiceAPI.h>
#include "tvchannel/types.h"

namespace extension {
namespace tvchannel {

class ChannelInfo;
class ProgramInfo;
//  TVServiceAPI static methods returns 0 on success
static const int TV_SERVICE_API_SUCCESS = 0;
//  TVServiceAPI object methods return 1 on success
static const int TV_SERVICE_API_METHOD_SUCCESS = 1;
//  TVServiceAPI object methods return -1 on failure
static const int TV_SERVICE_API_METHOD_FAILURE = -1;
//  You need to check error return in function/method docs and use correct
//  constant

class EventListener {
 public:
    virtual void onChannelChange() = 0;
};

class TVChannelManager {
 public:
    static TVChannelManager* getInstance();
    std::unique_ptr<ChannelInfo> getCurrentChannel(std::string const& _windowType);
    ProgramInfo* getCurrentProgram(std::string const& _windowType);
    static void ucs2utf8(char *out, size_t out_len, char *in, size_t in_len);
    void registerListener(EventListener* listener);
 private:
    EventListener* m_listener;
    //  Not copyable, assignable, movable
    TVChannelManager():
        m_listener(NULL) {
    }
    TVChannelManager(TVChannelManager const&) = delete;
    void operator=(TVChannelManager const&) = delete;
    TVChannelManager(TVChannelManager &&) = delete;

    EProfile getProfile(WindowType windowType);
    TCServiceId getCurrentChannelId(std::string const& _windowType);
    static int signalListener(ESignalType type, TSSignalData data, void*);
};

}  // namespace tvchannel
}  // namespace extension

#endif  // SRC_TVCHANNEL_TVCHANNEL_MANAGER_H_
