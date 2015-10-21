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

#include "notification/notification_manager.h"

#include <fcntl.h>
#include <unistd.h>

#include <app_control_internal.h>
#include <device/led.h>
#include <notification_internal.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/scope_exit.h"

#include "notification/status_notification.h"

namespace extension {
namespace notification {

using namespace common;

NotificationManager::NotificationManager() {
  LoggerD("Enter");
}

NotificationManager::~NotificationManager() {
  LoggerD("Enter");
}

NotificationManager* NotificationManager::GetInstance() {
  LoggerD("Enter");
  static NotificationManager instance;
  return &instance;
}

PlatformResult NotificationManager::Post(const picojson::object& args,
                                         picojson::object& out) {
  LoggerD("Enter");
  return StatusNotification::FromJson(args, false, &out);
}

PlatformResult NotificationManager::Update(const picojson::object& args) {
  LoggerD("Enter");
  return StatusNotification::FromJson(args, true, NULL);
}

PlatformResult NotificationManager::Remove(const picojson::object& args) {
  LoggerD("Enter");
  int id = std::stoi(FromJson<std::string>(args, "id"));

  int ret = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NONE, id);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Cannot remove notification error: %d", ret);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Cannot remove notification error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::RemoveAll() {
  LoggerD("Enter");
  int ret = notification_delete_all(NOTIFICATION_TYPE_NOTI);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Notification remove all failed: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Notification noti remove all failed");
  }

  ret = notification_delete_all(NOTIFICATION_TYPE_ONGOING);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerE("Notification remove all failed: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Notification ongoing remove all failed");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::Get(const picojson::object& args,
                                        picojson::object& out) {
  LoggerD("Enter");
  int id = std::stoi(FromJson<std::string>(args, "id"));

  app_control_h app_control = nullptr;
  notification_h noti_handle = nullptr;

  SCOPE_EXIT {
    if (app_control) {
      app_control_destroy(app_control);
    }
    free(noti_handle);
  };

  PlatformResult status = StatusNotification::GetNotiHandle(id, &noti_handle);
  if (status.IsError())
  {
    LoggerE("Failed: GetNotiHandle");
    return status;
  }

  status = StatusNotification::GetAppControl(noti_handle, &app_control);
  if (status.IsError())
  {
    LoggerE("Failed: GetAppControl");
    return status;
  }
  status = StatusNotification::ToJson(id, noti_handle, app_control, &out);
  if (status.IsError())
  {
    LoggerE("Failed: ToJson");
    return status;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::GetAll(picojson::array& out) {
  LoggerD("Enter");
  notification_h noti = nullptr;
  notification_list_h noti_list = nullptr;
  notification_list_h noti_list_iter = nullptr;
  char* package = nullptr;

  if (APP_ERROR_NONE == app_get_id(&package)) {
    LoggerD("Package id: %s", package);
  } else {
    LoggerD("Could not get package id");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Could not get package id");
  }
  const std::string package_str = package;
  free(package);

  int ret = notification_get_detail_list(package_str.c_str(), NOTIFICATION_GROUP_ID_NONE,
                                         NOTIFICATION_PRIV_ID_NONE, -1, &noti_list);
  if (NOTIFICATION_ERROR_NONE != ret) {
    LoggerD("Get notification list error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification list error");
  }

  SCOPE_EXIT { notification_free_list(noti_list); };

  noti_list_iter = notification_list_get_head(noti_list);

  while (nullptr != noti_list_iter) {
    noti = notification_list_get_data(noti_list_iter);
    if (nullptr != noti) {
      int noti_priv = -1;
      ret = notification_get_id(noti, NULL, &noti_priv);
      if (NOTIFICATION_ERROR_NONE != ret) {
        LoggerE("Cannot get notification id, error: %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Cannot get notification id error");
      }

      app_control_h app_control = nullptr;
      PlatformResult status =
          StatusNotification::GetAppControl(noti, &app_control);

      SCOPE_EXIT {
          if (app_control) {
              app_control_destroy(app_control);
          }
      };

      if (status.IsError())
        return status;

      picojson::object noti_item = picojson::object();

      status =
          StatusNotification::ToJson(noti_priv, noti, app_control, &noti_item);
      if (status.IsError())
        return status;

      out.push_back(picojson::value(noti_item));
    }

    noti_list_iter = notification_list_get_next(noti_list_iter);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::PlayLEDCustomEffect(
    const picojson::object& args) {
  LoggerD("Enter");

  int timeOn = FromJson<double>(args, "timeOn");
  int timeOff = FromJson<double>(args, "timeOff");
  unsigned int color = FromJson<double>(args, "color");

  auto& flags = FromJson<picojson::array>(args, "flags");
  unsigned int platformFlags = 0;
  for (auto flag : flags) {
    std::string flagStr = JsonCast<std::string>(flag);
    if (flagStr == "LED_CUSTOM_DEFAULT")
      platformFlags |= LED_CUSTOM_DEFAULT;
    else if (flagStr == "LED_CUSTOM_DUTY_ON")
      platformFlags |= LED_CUSTOM_DUTY_ON;
  }

  int ret;
  ret = device_led_play_custom(timeOn, timeOff, color, platformFlags);
  if (ret != DEVICE_ERROR_NONE) {
    LOGGER(ERROR) << "Cannot play LED custom effect: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot play LED custom effect");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::StopLEDCustomEffect() {
  LoggerD("Enter");

  int ret = device_led_stop_custom();
  if (ret != DEVICE_ERROR_NONE) {
    LOGGER(ERROR) << "Cannot stop LED custom effect: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot stop LED custom effect");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace notification
}  // namespace extension
