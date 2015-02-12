// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "push/push_instance.h"
#include <string>
#include <vector>
#include "common/logger.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

PushInstance::PushInstance():
        m_ignoreNotificationEvents(true) {
    LoggerD("Enter");
    using std::placeholders::_1;
    using std::placeholders::_2;
    RegisterHandler("Push_registerService",
            std::bind(&PushInstance::registerService, this, _1, _2));
    RegisterHandler("Push_unregisterService",
            std::bind(&PushInstance::unregisterService, this, _1, _2));
    RegisterSyncHandler("Push_connectService",
            std::bind(&PushInstance::connectService, this, _1, _2));
    RegisterSyncHandler("Push_disconnectService",
            std::bind(&PushInstance::disconnectService, this, _1, _2));
    RegisterSyncHandler("Push_getRegistrationId",
            std::bind(&PushInstance::getRegistrationId, this, _1, _2));
    RegisterSyncHandler("Push_getUnreadNotifications",
            std::bind(&PushInstance::getUnreadNotifications, this, _1, _2));
    PushManager::getInstance().setListener(this);
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
    picojson::value result;
    ReportSuccess(result, out);
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
    picojson::value result;
    ReportSuccess(result, out);
}

void PushInstance::getUnreadNotifications(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    picojson::value result;
    ReportSuccess(result, out);
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
    PostMessage(res.serialize().c_str());
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
    PostMessage(resultListener.serialize().c_str());
}

PushInstance::~PushInstance() {
    LoggerD("Enter");
}

}  // namespace push
}  // namespace extension
