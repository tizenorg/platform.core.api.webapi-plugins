// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "systemsetting/systemsetting_instance.h"

#include <memory>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include <system_settings.h>


namespace extension {
namespace systemsetting {

namespace {
const std::string SETTING_HOME_SCREEN = "HOME_SCREEN";
const std::string SETTING_LOCK_SCREEN = "LOCK_SCREEN";
const std::string SETTING_INCOMING_CALL = "INCOMING_CALL";
const std::string SETTING_NOTIFICATION_EMAIL = "NOTIFICATION_EMAIL";
}

using namespace common;
using namespace extension::systemsetting;

SystemSettingInstance::SystemSettingInstance()
{
    using namespace std::placeholders;

#define REGISTER(c,x) \
RegisterHandler(c, std::bind(&SystemSettingInstance::x, this, _1, _2));

    REGISTER("SystemSettingManager_getProperty", getProperty);
    REGISTER("SystemSettingManager_setProperty", setProperty);

#undef REGISTER
}

SystemSettingInstance::~SystemSettingInstance()
{
}

void SystemSettingInstance::getProperty(const picojson::value& args, picojson::object& out)
{
    LoggerD("");
    const double callback_id = args.get("callbackId").get<double>();

    const std::string& type = args.get("type").get<std::string>();
    LoggerD("Getting property type: %s ", type.c_str());

    auto get = [this, type](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting platform value");
        try {
            picojson::value result;
            getPlatformPropertyValue(type, &result);
            ReportSuccess(result, response->get<picojson::object>());
        } catch (const PlatformException& e) {
            ReportError(e, response->get<picojson::object>());
        }
    };

    auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        PostMessage(response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>
        (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SystemSettingInstance::getPlatformPropertyValue(
    const std::string& settingType, picojson::value* out)
{
    picojson::object& result_obj = out->get<picojson::object>();

    int ret;
    char *value = NULL;
    if (settingType == SETTING_HOME_SCREEN) {
        ret = system_settings_get_value_string(
            SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, &value);
    }
    else if (settingType == SETTING_LOCK_SCREEN) {
        ret = system_settings_get_value_string(
            SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, &value);
    }
    else if (settingType == SETTING_INCOMING_CALL) {
        ret = system_settings_get_value_string(
            SYSTEM_SETTINGS_KEY_INCOMING_CALL_RINGTONE, &value);
    }
    else if (settingType == SETTING_NOTIFICATION_EMAIL) {
        ret = system_settings_get_value_string(
            SYSTEM_SETTINGS_KEY_EMAIL_ALERT_RINGTONE, &value);
    }
    // other values (not specified in the documentation) are handled in JS

    switch (ret) {
        case SYSTEM_SETTINGS_ERROR_NONE:
            LoggerD("ret == SYSTEM_SETTINGS_ERROR_NONE");
            result_obj.insert(std::make_pair("value", value));
            free(value);
            break;
        case SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API:
            LoggerD("ret == SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API");
            throw NotSupportedException("This property is not supported.");
        default:
            LoggerD("Other error");
            throw UnknownException("Unknown error");
  }
}

void SystemSettingInstance::setProperty(const picojson::value& args, picojson::object& out)
{
    LoggerD("");
    const double callback_id = args.get("callbackId").get<double>();

    const std::string& type = args.get("type").get<std::string>();
    LoggerD("Type to set: %s ", type.c_str());

    const std::string& value = args.get("value").get<std::string>();
    LoggerD("Value to set: %s ", value.c_str());

    auto get = [this, type, value](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Setting platform value");
        try {
            picojson::value result;
            setPlatformPropertyValue(type, value, &result);
            ReportSuccess(result, response->get<picojson::object>());
        } catch (const PlatformException& e) {
            ReportError(e, response->get<picojson::object>());
        }
    };

    auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", callback_id));
        PostMessage(response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>
        (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SystemSettingInstance::setPlatformPropertyValue(
    const std::string& settingType, const std::string& settingValue, picojson::value* out)
{
    picojson::object& result_obj = out->get<picojson::object>();

    int ret;
    if (settingType == SETTING_HOME_SCREEN) {
        ret = system_settings_set_value_string(
            SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, settingValue.c_str());
    }
    else if (settingType == SETTING_LOCK_SCREEN) {
        ret = system_settings_set_value_string(
            SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, settingValue.c_str());
    }
    else if (settingType == SETTING_INCOMING_CALL) {
        ret = system_settings_set_value_string(
            SYSTEM_SETTINGS_KEY_INCOMING_CALL_RINGTONE, settingValue.c_str());
    }
    else if (settingType == SETTING_NOTIFICATION_EMAIL) {
        ret = system_settings_set_value_string(
            SYSTEM_SETTINGS_KEY_EMAIL_ALERT_RINGTONE, settingValue.c_str());
    }
    // other values (not specified in the documentation) are handled in JS

    switch (ret) {
        case SYSTEM_SETTINGS_ERROR_NONE:
            LoggerD("ret == SYSTEM_SETTINGS_ERROR_NONE");
            return;
        case SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API:
            LoggerD("ret == SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API");
            throw NotSupportedException("This property is not supported.");
        default:
            LoggerD("Other error");
            throw UnknownException("Unknown error");
    }
}

} // namespace systemsetting
} // namespace extension

