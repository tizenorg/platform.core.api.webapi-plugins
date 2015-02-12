// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "push/push_manager.h"
#include <unistd.h>
#include <pcrecpp.h>
#include <app_control.h>
#include <app_manager.h>
#include "common/logger.h"

namespace extension {
namespace push {

using common::PlatformResult;
using common::ErrorCode;

PushManager::PushManager() :
    m_handle(NULL),
    m_listener(NULL),
    m_state(PUSH_STATE_UNREGISTERED) {
    LoggerD("Enter");
    initAppId();

    int ret = push_connect(m_pkgId.c_str(), onPushState, onPushNotify, NULL,
        &m_handle);
    if (ret != PUSH_ERROR_NONE) {
        LoggerE("Failed to connect to push");
    }
}

PushManager::~PushManager() {
    LoggerD("Enter");
    push_disconnect(m_handle);
}

void PushManager::setListener(EventListener* listener) {
    m_listener = listener;
}

void PushManager::initAppId() {
    int pid = getpid();
    char *temp = NULL;
    int ret = app_manager_get_app_id(pid, &temp);
    if (ret != APP_MANAGER_ERROR_NONE || temp == NULL) {
        LoggerE("Failed to get appid");
        return;
    }

    m_appId = temp;
    free(temp);
    temp = NULL;

    app_info_h info;
    ret = app_manager_get_app_info(m_appId.c_str(), &info);
    if (ret != APP_MANAGER_ERROR_NONE) {
        LoggerE("Failed to get app info");
        return;
    }

    ret = app_info_get_package(info, &temp);
    if (ret == APP_MANAGER_ERROR_NONE && temp != NULL) {
        m_pkgId = temp;
        free(temp);
    } else {
        LoggerE("Failed to get pkg id");
    }

    app_info_destroy(info);
}

PushManager& PushManager::getInstance() {
  static PushManager instance;
  return instance;
}

PlatformResult PushManager::registerService(
        const ApplicationControl &appControl, double callbackId) {
    LoggerD("Enter");
    app_control_h service;
    int ret = app_control_create(&service);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LoggerE("Failed to create service: app_control_create failed");
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to create service");
    }

    if (appControl.operation.empty()) {
        LoggerE("Operation is empty");
        app_control_destroy(service);
        return common::PlatformResult(ErrorCode::INVALID_VALUES_ERR,
            "Operation is empty");
    }
    ret = app_control_set_operation(service, appControl.operation.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
        LoggerE("Failed to set operation: app_control_set_operation failed");
        app_control_destroy(service);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to set operation");
    }

    if (!appControl.uri.empty()) {
        ret = app_control_set_uri(service, appControl.uri.c_str());
        if (ret != APP_CONTROL_ERROR_NONE) {
            LoggerE("Failed to set uri: app_control_set_uri failed");
            app_control_destroy(service);
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Failed to set uri");
        }
    }

    if (!appControl.mime.empty()) {
        ret = app_control_set_mime(service, appControl.mime.c_str());
        if (ret != APP_CONTROL_ERROR_NONE) {
            LoggerE("Failed to set mime: app_control_set_mime failed");
            app_control_destroy(service);
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Failed to set mime");
        }
    }

    if (!appControl.category.empty()) {
        ret = app_control_set_category(service, appControl.category.c_str());
        if (ret != APP_CONTROL_ERROR_NONE) {
            LoggerE("Failed to set category: app_control_set_category failed");
            app_control_destroy(service);
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Failed to set category");
        }
    }

    ret = app_control_set_app_id(service, m_appId.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
        LoggerE("Failed to set app id: app_control_set_app_id failed");
        app_control_destroy(service);
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to set app id");
    }

    for (auto &item : appControl.data) {
        if (item.second.size() == 1) {
            ret = app_control_add_extra_data(service, item.first.c_str(),
                item.second.front().c_str());
        } else {
            const char *values[item.second.size()];
            for (size_t i = 0; i < item.second.size(); ++i) {
                values[i] = item.second.at(i).c_str();
            }
            ret = app_control_add_extra_data_array(service,
                item.first.c_str(), values, item.second.size());
        }
        if (ret != APP_CONTROL_ERROR_NONE) {
            LoggerE(
                "Failed to set extra data: app_control_add_extra_data failed");
            app_control_destroy(service);
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Failed to set extra data");
        }
    }

    double* pcallback = new double(callbackId);
    ret = push_register(m_handle, service, onPushRegister, pcallback);
    app_control_destroy(service);
    if (ret != PUSH_ERROR_NONE) {
        delete pcallback;
        LoggerE("Failed to register push: push_register failed");
        return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
            "Failed to register");
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult PushManager::unregisterService(double callbackId) {
    double* pcallbackId = new double(callbackId);
    if (m_state == PUSH_STATE_UNREGISTERED) {
        LoggerD("Already unregister, call unregister callback");
        if (!g_idle_add(onFakeDeregister, pcallbackId)) {
            delete pcallbackId;
            LoggerE("g_idle_add failed");
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Unknown error");
        }
    } else {
        int ret = push_deregister(m_handle, onDeregister, pcallbackId);
        if (ret != PUSH_ERROR_NONE) {
            delete pcallbackId;
            LoggerE("Failed to deregister: push_deregister failed");
            return common::PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Unknown error");
        }
    }
    return common::PlatformResult(ErrorCode::NO_ERROR);
}

void PushManager::onPushState(push_state_e state, const char* err,
        void* user_data) {
    LoggerD("Enter %d", state);
    getInstance().m_state = state;
}

void PushManager::onPushNotify(push_notification_h noti, void* user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return;
    }

    char* temp = NULL;
    int ret = push_get_notification_data(noti, &temp);
    if (ret != PUSH_ERROR_NONE) {
        LoggerE("Failed to get appData");
        return;
    }
    std::string appData = temp;
    free(temp);

    temp = NULL;
    ret = push_get_notification_message(noti, &temp);
    if (ret != PUSH_ERROR_NONE) {
        LoggerE("Failed to get message");
        return;
    }

    // parse query string and find value for alertMessage
    pcrecpp::StringPiece input(temp);
    pcrecpp::RE re("([^=]+)=([^&]*)&?");
    string key;
    string value;
    std::string alertMessage;
    while (re.Consume(&input, &key, &value)) {
        if (key == "alertMessage") {
            alertMessage = value;
            break;
        }
    }
    free(temp);

    int64_t date = -1;
    ret = push_get_notification_time(noti, &date);
    if (ret != PUSH_ERROR_NONE) {
        LoggerE("Failed to get date");
        return;
    }
    getInstance().m_listener->onPushNotify(appData, alertMessage, date);
}

void PushManager::onPushRegister(push_result_e result, const char* msg,
        void* user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return;
    }
    double* callbackId = static_cast<double*>(user_data);
    std::string id;
    PlatformResult res(ErrorCode::NO_ERROR);
    if (result == PUSH_RESULT_SUCCESS) {
        LoggerD("Success");
        char *temp = NULL;
        int ret = push_get_registration_id(getInstance().m_handle, &temp);
        if (ret == PUSH_ERROR_NONE) {
            LoggerD("Registration id retrieved");
            id = temp;
            free(temp);
        } else {
            res = PlatformResult(ErrorCode::UNKNOWN_ERR,
                "Failed to retrieve registration id");
        }
    } else {
        res = PlatformResult(ErrorCode::UNKNOWN_ERR,
                msg == NULL ? "Unknown error" : msg);
    }
    getInstance().m_listener->onPushRegister(*callbackId, res, id);
    delete callbackId;
}

gboolean PushManager::onFakeDeregister(gpointer user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return G_SOURCE_REMOVE;
    }
    double* callbackId = static_cast<double*>(user_data);
    getInstance().m_listener->onDeregister(*callbackId,
        PlatformResult(ErrorCode::NO_ERROR));
    delete callbackId;
    return G_SOURCE_REMOVE;
}

void PushManager::onDeregister(push_result_e result, const char* msg,
        void* user_data) {
    LoggerD("Enter");
    if (!getInstance().m_listener) {
        LoggerW("Listener not set, ignoring");
        return;
    }
    double* callbackId = static_cast<double*>(user_data);
    if (result == PUSH_RESULT_SUCCESS) {
        getInstance().m_listener->onDeregister(*callbackId,
            PlatformResult(ErrorCode::NO_ERROR));
    } else {
        getInstance().m_listener->onDeregister(*callbackId,
            PlatformResult(ErrorCode::UNKNOWN_ERR,
                msg == NULL ? "Unknown error" : msg));
    }
    delete callbackId;
}

}  // namespace push
}  // namespace extension

