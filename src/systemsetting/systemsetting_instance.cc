// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "systemsetting/systemsetting_instance.h"

#include <memory>

#include "common/logger.h"
#include "common/picojson.h"
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
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER(c,x) \
    RegisterSyncHandler(c, std::bind(&SystemSettingInstance::x, this, _1, _2));

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
    picojson::value result = picojson::value(picojson::object());
    PlatformResult status = getPlatformPropertyValue(type, &result);
    if(status.IsSuccess())
      ReportSuccess(result, response->get<picojson::object>());
    else
      ReportError(status, &response->get<picojson::object>());
  };

  auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>
  (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

PlatformResult SystemSettingInstance::getPlatformPropertyValue(
    const std::string& settingType,
    picojson::value* out) {
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
      result_obj.insert(std::make_pair("value", picojson::value(value)));
      free(value);
      return PlatformResult(ErrorCode::NO_ERROR);
    // TODO(p.kaczmarek3) temporarily removed - not supported by platform
    //case SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API:
    //  LoggerD("ret == SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API");
    //  return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR,
    //                        "This property is not supported.");
    default:
      LoggerD("Other error");
      return PlatformResult(ErrorCode::UNKNOWN_ERR);
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
    PlatformResult status = setPlatformPropertyValue(type, value);
    if (status.IsSuccess())
      ReportSuccess(response->get<picojson::object>());
    else
      ReportError(status, &response->get<picojson::object>());
  };

  auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
    LoggerD("Getting response");
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<picojson::value>
  (get, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

PlatformResult SystemSettingInstance::setPlatformPropertyValue(
    const std::string& settingType,
    const std::string& settingValue) {
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
      return PlatformResult(ErrorCode::NO_ERROR);
    // TODO(p.kaczmarek3) temporarily removed - not supported by platform
    //case SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API:
    //  LoggerD("ret == SYSTEM_SETTINGS_ERROR_CALL_UNSUPPORTED_API");
    //  return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR,
    //                        "This property is not supported.");
    default:
      LoggerD("Other error");
      return PlatformResult(ErrorCode::UNKNOWN_ERR);
  }
}

} // namespace systemsetting
} // namespace extension

