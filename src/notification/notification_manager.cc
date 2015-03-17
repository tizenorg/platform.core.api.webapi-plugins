// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "notification/notification_manager.h"

#include <notification_internal.h>
#include <app_control_internal.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/scope_exit.h"

#include "notification/status_notification.h"

namespace extension {
namespace notification {

using namespace common;

NotificationManager::NotificationManager() {}

NotificationManager::~NotificationManager() {}

NotificationManager* NotificationManager::GetInstance() {
  static NotificationManager instance;
  return &instance;
}

PlatformResult NotificationManager::Post(const picojson::object& args,
                                         int* id) {
  return StatusNotification::FromJson(args, id);
}

PlatformResult NotificationManager::Update(const picojson::object& args) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NotificationManager::Remove(const picojson::object& args) {
  int id = std::stoi(FromJson<std::string>(args, "id"));

  int ret = notification_delete_by_priv_id(NULL, NOTIFICATION_TYPE_NONE, id);
  if (ret != NOTIFICATION_ERROR_NONE) {
      LoggerE("Cannot remove notification error: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot remove notification error");
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
  int id = common::stol(FromJson<std::string>(args, "id"));

  notification_h noti = notification_load(NULL, id);
  if (NULL == noti) {
    LoggerE("Not found or removed notification id");
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Not found or removed notification id");
  }

  app_control_h app_control;
  PlatformResult status = StatusNotification::GetAppControl(noti, &app_control);
  if (status.IsError()) return status;

  status = StatusNotification::ToJson(id, noti, app_control, &out);
  if (status.IsError()) return status;

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

  SCOPE_EXIT {
      notification_free_list(noti_list);
  };

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
      if (status.IsError()) return status;

      picojson::object noti_item = picojson::object();

      status =
          StatusNotification::ToJson(noti_priv, noti, app_control, &noti_item);
      if (status.IsError()) return status;

      out.push_back(picojson::value(noti_item));
    }

    noti_list_iter = notification_list_get_next(noti_list_iter);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace notification
}  // namespace extension
