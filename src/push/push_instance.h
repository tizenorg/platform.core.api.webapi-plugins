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

#ifndef SRC_PUSH_PUSH_INSTANCE_H_
#define SRC_PUSH_PUSH_INSTANCE_H_

#include <string>
#include "common/extension.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

class PushInstance: public common::ParsedInstance, public EventListener {
 public:
    PushInstance();
    virtual ~PushInstance();
    virtual void onPushRegister(double callbackId,
            common::PlatformResult result, const std::string& id);
    virtual void onPushNotify(const std::string& appData,
            const std::string& alertMessage, double date);
    virtual void onDeregister(double callbackId, common::PlatformResult result);

 private:
     void registerService(const picojson::value& args, picojson::object& out);
     void unregisterService(const picojson::value& args, picojson::object& out);
     void connectService(const picojson::value& args, picojson::object& out);
     void disconnectService(const picojson::value& args, picojson::object& out);
     void getRegistrationId(const picojson::value& args, picojson::object& out);
     void getUnreadNotifications(const picojson::value& args,
            picojson::object& out);

     bool m_ignoreNotificationEvents;
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_INSTANCE_H_
