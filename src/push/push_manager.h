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

#include <push-service.h>
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
    virtual void onPushNotify(push_service_notification_h noti) = 0;
    virtual void onDeregister(double callbackId,
            common::PlatformResult result) = 0;
    virtual ~EventListener() {}
};

class PushManager {
 public:
    static PushManager& getInstance();
    virtual ~PushManager();

    void setListener(EventListener* listener);

    common::PlatformResult registerApplication(double callbackId);
    common::PlatformResult unregisterApplication(double callbackId);
    common::PlatformResult getRegistrationId(std::string &id);
    common::PlatformResult getUnreadNotifications();
    common::PlatformResult getPushMessage(picojson::value* out);
    void notificationToJson(push_service_notification_h noti, picojson::object* obj);

 private:
    PushManager();
    void initAppId();
    void InitAppControl();

    static void onPushState(push_service_state_e state, const char *err,
        void *user_data);
    static void onPushNotify(push_service_notification_h noti, void *user_data);
    static void onApplicationRegister(push_service_result_e result, const char *msg,
        void *user_data);
    static gboolean onFakeDeregister(gpointer user_data);
    static void onDeregister(push_service_result_e result, const char *msg,
        void *user_data);

    static std::string GetEncodedBundle();

    push_service_connection_h m_handle;
    EventListener* m_listener;
    push_service_state_e m_state;
    std::string m_appId;
    std::string m_pkgId;

    app_control_h app_control_;
    char* operation_;
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_MANAGER_H_

