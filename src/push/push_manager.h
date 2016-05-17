/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef SRC_PUSH_PUSH_MANAGER_H_
#define SRC_PUSH_PUSH_MANAGER_H_

#include <push.h>
#include <glib.h>
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
    virtual void onPushNotify(const std::string& appData,
            const std::string& alertMessage, double date) = 0;
    virtual void onDeregister(double callbackId,
            common::PlatformResult result) = 0;
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
    common::PlatformResult unregisterService(double callbackId);
    common::PlatformResult getRegistrationId(std::string &id);
    common::PlatformResult getUnreadNotifications();

 private:
    PushManager();
    void initAppId();
    static void onPushState(push_state_e state, const char *err,
        void *user_data);
    static void onPushNotify(push_notification_h noti, void *user_data);
    static void onPushRegister(push_result_e result, const char *msg,
        void *user_data);
    static gboolean onFakeDeregister(gpointer user_data);
    static void onDeregister(push_result_e result, const char *msg,
        void *user_data);

    push_connection_h m_handle;
    EventListener* m_listener;
    push_state_e m_state;
    std::string m_appId;
    std::string m_pkgId;
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_MANAGER_H_

