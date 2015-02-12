// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PUSH_PUSH_MANAGER_H_
#define SRC_PUSH_PUSH_MANAGER_H_

#include <push.h>
#include <string>
#include <vector>
#include <map>
#include "common/platform_result.h"

namespace extension {
namespace push {

class EventListener {
 public:
    virtual void onPushRegister(double callbackId,
            common::PlatformResult result, const std::string& id) = 0;
    virtual ~EventListener() {}
};

class PushManager {
 public:
    static PushManager& getInstance();
    virtual ~PushManager();

    void setListener(EventListener* listener);
    struct ApplicationControl {
        std::string operation;
        std::string uri;
        std::string mime;
        std::string category;
        std::map<std::string, std::vector<std::string> > data;
    };
    common::PlatformResult registerService(const ApplicationControl &appControl,
        double callbackId);

 private:
    PushManager();
    void initAppId();
    static void onPushState(push_state_e state, const char *err,
        void *user_data);
    static void onPushNotify(push_notification_h noti, void *user_data);
    static void onPushRegister(push_result_e result, const char *msg,
        void *user_data);

    push_connection_h m_handle;
    EventListener* m_listener;
    std::string m_appId;
    std::string m_pkgId;
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_MANAGER_H_

