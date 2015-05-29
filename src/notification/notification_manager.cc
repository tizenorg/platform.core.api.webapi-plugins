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
}

NotificationManager::~NotificationManager() {
}

NotificationManager* NotificationManager::GetInstance() {
  static NotificationManager instance;
  return &instance;
}

PlatformResult NotificationManager::Post(const picojson::object& args,
                                         picojson::object& out) {
  return StatusNotification::FromJson(args, false, &out);
}

PlatformResult NotificationManager::Update(const picojson::object& args) {
  return StatusNotification::FromJson(args, true, NULL);
}

PlatformResult NotificationManager::Remove(const picojson::object& args) {
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
  int id = std::stoi(FromJson<std::string>(args, "id"));

  notification_h noti_handle;
  PlatformResult status = StatusNotification::GetNotiHandle(id, &noti_handle);
  if (status.IsError())
    return status;

  app_control_h app_control;
  status = StatusNotification::GetAppControl(noti_handle, &app_control);
  if (status.IsError())
    return status;

  status = StatusNotification::ToJson(id, noti_handle, app_control, &out);
  if (status.IsError())
    return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::GetAll(picojson::array& out) {
  notification_h noti = nullptr;
  notification_list_h noti_list = nullptr;
  notification_list_h noti_list_iter = nullptr;

  int ret =
      notification_get_grouping_list(NOTIFICATION_TYPE_NONE, -1, &noti_list);
  if (ret != NOTIFICATION_ERROR_NONE) {
    LoggerD("Get notification list error: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Get notification list error");
  }

  SCOPE_EXIT { notification_free_list(noti_list); };

  noti_list_iter = notification_list_get_head(noti_list);

  while (noti_list_iter != nullptr) {
    noti = notification_list_get_data(noti_list_iter);
    if (noti != nullptr) {
      int noti_priv = -1;
      ret = notification_get_id(noti, NULL, &noti_priv);
      if (ret != NOTIFICATION_ERROR_NONE) {
        LoggerE("Cannot get notification id, error: %d", ret);
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Cannot get notification id error");
      }

      app_control_h app_control;
      PlatformResult status =
          StatusNotification::GetAppControl(noti, &app_control);
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
  LOGGER(DEBUG) << "entered";

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
  LOGGER(DEBUG) << "entered";

  int ret = DEVICE_ERROR_NONE;
  ret = device_led_stop_custom();
  if (ret != DEVICE_ERROR_NONE) {
    LOGGER(ERROR) << "Cannot stop LED custom effect: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot stop LED custom effect");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace notification
}  // namespace extension
