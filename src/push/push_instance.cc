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

#include "push/push_instance.h"
#include <string>
#include <vector>
#include "common/logger.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

PushInstance::PushInstance(): m_ignoreNotificationEvents(true) {
    LoggerD("Enter");
    using std::placeholders::_1;
    using std::placeholders::_2;

    #define REGISTER_ASYNC(c, func) \
        RegisterSyncHandler(c, func);
    #define REGISTER_SYNC(c, func) \
        RegisterSyncHandler(c, func);

    REGISTER_ASYNC("Push_registerService",
        std::bind(&PushInstance::registerService, this, _1, _2));
    REGISTER_ASYNC("Push_unregisterService",
        std::bind(&PushInstance::unregisterService, this, _1, _2));
    REGISTER_SYNC("Push_connectService",
        std::bind(&PushInstance::connectService, this, _1, _2));
    REGISTER_SYNC("Push_disconnectService",
        std::bind(&PushInstance::disconnectService, this, _1, _2));
    REGISTER_SYNC("Push_getRegistrationId",
        std::bind(&PushInstance::getRegistrationId, this, _1, _2));
    REGISTER_SYNC("Push_getUnreadNotifications",
        std::bind(&PushInstance::getUnreadNotifications, this, _1, _2));
    PushManager::getInstance().setListener(this);

    #undef REGISTER_ASYNC
    #undef REGISTER_SYNC
}

void PushInstance::registerService(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");

    PushManager::ApplicationControl appControl;
    appControl.operation = args.get("operation").get<std::string>();
    if (args.get("uri").is<std::string>()) {
        appControl.uri = args.get("uri").get<std::string>();
    }
    if (args.get("mime").is<std::string>()) {
        appControl.mime = args.get("mime").get<std::string>();
    }
    if (args.get("category").is<std::string>()) {
        appControl.category = args.get("category").get<std::string>();
    }
    if (args.get("data").is<picojson::null>() == false) {
        std::vector<picojson::value> dataArray =
                args.get("data").get<picojson::array>();
        for (auto &item : dataArray) {
            std::string key = item.get("key").get<std::string>();
            std::vector<picojson::value> values =
                    item.get("value").get<picojson::array>();
            for (auto &value : values) {
                appControl.data[key].push_back(value.to_str());
            }
        }
    }
    common::PlatformResult result = PushManager::getInstance().registerService(
            appControl,
            args.get("callbackId").get<double>());
    if (result.IsError()) {
        LoggerE("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value result;
        ReportSuccess(result, out);
    }
}

void PushInstance::unregisterService(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");

    common::PlatformResult result = PushManager::getInstance()
            .unregisterService(args.get("callbackId").get<double>());
    if (result.IsError()) {
        LoggerE("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value res;
        ReportSuccess(res, out);
    }
}

void PushInstance::connectService(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");

    m_ignoreNotificationEvents = false;
    picojson::value result;
    ReportSuccess(result, out);
}

void PushInstance::disconnectService(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");

    m_ignoreNotificationEvents = true;
    picojson::value result;
    ReportSuccess(result, out);
}

void PushInstance::getRegistrationId(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");

    std::string id;
    common::PlatformResult result = PushManager::getInstance()
            .getRegistrationId(id);
    if (result.IsError()) {
        // this method should fail silently and return null
        picojson::value res = picojson::value();
        ReportSuccess(res, out);
    } else {
        picojson::value res(id);
        ReportSuccess(res, out);
    }
}

void PushInstance::getUnreadNotifications(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");

    common::PlatformResult result = PushManager::getInstance()
            .getUnreadNotifications();
    if (result.IsError()) {
        LoggerE("Error occured");
        ReportError(result, &out);
    } else {
        picojson::value res;
        ReportSuccess(res, out);
    }
}

void PushInstance::onPushRegister(double callbackId,
        common::PlatformResult result, const std::string& id) {
    LoggerD("Enter");
    picojson::value::object dict;
    dict["callbackId"] = picojson::value(callbackId);
    if (result.IsError()) {
        dict["error"] = result.ToJSON();
    } else {
        dict["registrationId"] = picojson::value(id);
    }
    picojson::value res(dict);
    Instance::PostMessage(this, res.serialize().c_str());
}

void PushInstance::onPushNotify(const std::string& appData,
        const std::string& alertMessage, double date) {
    LoggerD("Enter");
    if (m_ignoreNotificationEvents) {
        LoggerD("Listener not set, ignoring event");
    }
    picojson::value::object dict;
    dict["listenerId"] = picojson::value("Push_Notification_Listener");
    picojson::value::object pushMessage;
    pushMessage["appData"] = picojson::value(appData);
    pushMessage["alertMessage"] = picojson::value(alertMessage);
    pushMessage["date"] = picojson::value(date);
    dict["pushMessage"] = picojson::value(pushMessage);
    picojson::value resultListener(dict);
    Instance::PostMessage(this, resultListener.serialize().c_str());
}

void PushInstance::onDeregister(double callbackId,
        common::PlatformResult result) {
    LoggerD("Enter");
    picojson::value::object dict;
    dict["callbackId"] = picojson::value(callbackId);
    if (result.IsError()) {
        dict["error"] = result.ToJSON();
    }
    picojson::value res(dict);
    Instance::PostMessage(this, res.serialize().c_str());
}

PushInstance::~PushInstance() {
    LoggerD("Enter");
}

}  // namespace push
}  // namespace extension
