// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"

#include "push/push_instance.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

PushInstance::PushInstance() {
    LoggerD("Enter");
    using std::placeholders::_1;
    using std::placeholders::_2;
    RegisterHandler("Push_registerService",
            std::bind(&PushInstance::registerService, this, _1, _2));
    RegisterHandler("Push_unregisterService",
            std::bind(&PushInstance::unregisterService, this, _1, _2));
    RegisterHandler("Push_connectService",
            std::bind(&PushInstance::connectService, this, _1, _2));
    RegisterSyncHandler("Push_disconnectService",
            std::bind(&PushInstance::disconnectService, this, _1, _2));
    RegisterSyncHandler("Push_getRegistrationId",
            std::bind(&PushInstance::getRegistrationId, this, _1, _2));
    RegisterSyncHandler("Push_getUnreadNotifications",
            std::bind(&PushInstance::getUnreadNotifications, this, _1, _2));
}

void PushInstance::registerService(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
    picojson::value result;
    ReportSuccess(result, out);
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
    picojson::value result;
    ReportSuccess(result, out);
}

void PushInstance::disconnectService(const picojson::value& args,
        picojson::object& out) {
    LoggerD("Enter");
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

PushInstance::~PushInstance() {
    LoggerD("Enter");
}

}  // namespace push
}  // namespace extension
