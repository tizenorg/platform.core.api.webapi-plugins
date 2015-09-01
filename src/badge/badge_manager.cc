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

#include "badge/badge_manager.h"

#include <aul.h>
#include <badge.h>
#include <badge_internal.h>
#include <fcntl.h>
#include <pkgmgr-info.h>
#include <unistd.h>

#include "badge/badge_instance.h"
#include "common/assert.h"
#include "common/logger.h"
#include "common/converter.h"
#include "common/platform_exception.h"

using namespace common;

namespace extension {
namespace badge {

BadgeManager::BadgeManager(BadgeInstance& instance)
    : instance_(instance),
      is_cb_registered_(false) {
}

BadgeManager::~BadgeManager() {
  LoggerD("Enter");
  if (is_cb_registered_) {
    if (!watched_applications_.empty()) watched_applications_.clear();
    int ret = badge_unregister_changed_cb(badge_changed_cb);
    if (ret != BADGE_ERROR_NONE) {
      LoggerE("Unknown error : %d", ret);
    }
    is_cb_registered_ = false;
  }
}

PlatformResult BadgeManager::SetBadgeCount(const std::string& app_id,
                                           unsigned int count) {
  LoggerD("Enter");
  SLoggerD("app_id : %s ", app_id.c_str());

  if (!IsAppInstalled(app_id)) {
    LoggerE("Application is not installed");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : app_id");
  }

  bool badge_exist = false;
  const char *app_id_str = app_id.c_str();
  int ret = badge_is_existing(app_id_str, &badge_exist);

  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }

  LoggerD("badge exist : %d", badge_exist);

  if (!badge_exist) {
    ret = badge_create(app_id_str, app_id_str);
    LoggerD("badge_create() ret : %d", ret);

    if (ret == BADGE_ERROR_PERMISSION_DENIED) {
      LoggerE("Security error");
      return PlatformResult(ErrorCode::SECURITY_ERR, "Security error");
#ifdef PROFILE_WEARABLE
    } else if (ret == BADGE_ERROR_INVALID_DATA) {
#else
    } else if (ret == BADGE_ERROR_INVALID_PARAMETER) {
#endif
      LoggerE("Invalid values error");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Invalid values error");
    } else if (ret != BADGE_ERROR_NONE && ret != BADGE_ERROR_ALREADY_EXIST) {
      LoggerE("Unknown error");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Unknown error");
    }
  }

  ret = badge_set_count(app_id_str, count);
  LoggerE("badge_set_count() ret : %d count : %d ", ret, count);

  if (ret == BADGE_ERROR_PERMISSION_DENIED) {
    LoggerE("Security error");
    return PlatformResult(ErrorCode::SECURITY_ERR, "Security error");
  } else if (ret == BADGE_ERROR_NOT_EXIST) {
    LoggerE("Application is not installed");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Application is not installed");
#ifdef PROFILE_WEARABLE
  } else if (ret == BADGE_ERROR_INVALID_DATA) {
#else
  } else if (ret == BADGE_ERROR_INVALID_PARAMETER) {
#endif
    LoggerE("Invalid values error");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid values error");
  } else if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult BadgeManager::GetBadgeCount(const std::string& app_id,
                                           unsigned int *count) {
  LoggerD("Enter");
  SLoggerD("app_id : %s ", app_id.c_str());

  Assert(count);

  if (!IsAppInstalled(app_id)) {
    LoggerE("Application is not installed");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : app_id");
  }

  bool badge_exist = false;
  int ret = badge_is_existing(app_id.c_str(), &badge_exist);

  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while checking badge.");
  }

  LoggerD("badge exist : %d", badge_exist);

  if (!badge_exist) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "badge not exist. app_id: " + app_id);
  }

  *count = 0;
  ret = badge_get_count(app_id.c_str(), count);

  LoggerD("badge_get_count() ret : %d count : %d", ret, *count);

  switch (ret) {
    case BADGE_ERROR_NONE:
      return PlatformResult(ErrorCode::NO_ERROR);
    case BADGE_ERROR_PERMISSION_DENIED:
      LoggerE("Security error");
      return PlatformResult(ErrorCode::SECURITY_ERR, "Security error.");
    case BADGE_ERROR_NOT_EXIST:
      LoggerE("Application is not installed");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Application is not installed");
#ifdef PROFILE_WEARABLE
    case BADGE_ERROR_INVALID_DATA:
#else
    case BADGE_ERROR_INVALID_PARAMETER:
#endif
      LoggerE("Invalid values error");
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "InvalidValues error : app_id");
    default:
      LoggerE("Unknown error : %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }
}

PlatformResult BadgeManager::AddChangeListener(const JsonObject &obj) {
  LoggerD("Enter");
  auto &items = FromJson<picojson::array>(obj, "appIdList");
  for (auto item : items) {
    watched_applications_.insert(common::JsonCast<std::string>(item));
  }
  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  if (!is_cb_registered_) {
    ret = badge_register_changed_cb(badge_changed_cb, this);
    if (ret != BADGE_ERROR_NONE) {
      LoggerE("Unknown error %d:", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Platform error while adding listener.");
    }
    is_cb_registered_ = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult BadgeManager::RemoveChangeListener(const JsonObject &obj) {
  LoggerD("Enter");
  auto &items = FromJson<picojson::array>(obj, "appIdList");
  for (auto item : items) {
    watched_applications_.erase(common::JsonCast<std::string>(item));
  }
  if (watched_applications_.empty() && is_cb_registered_) {
    int ret = badge_unregister_changed_cb(badge_changed_cb);
    if (ret != BADGE_ERROR_NONE) {
      LoggerE("Unknown error : %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Platform error while removing listener.");
    }
    is_cb_registered_ = false;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void BadgeManager::badge_changed_cb(unsigned int action, const char *pkgname,
                                    unsigned int count, void *user_data) {
  LoggerD("Enter");
  BadgeManager* that = static_cast<BadgeManager*>(user_data);
  if (action != BADGE_ACTION_SERVICE_READY &&
      that->watched_applications_.find(pkgname) != that->watched_applications_.end()) {
    picojson::value response = picojson::value(picojson::object());
    picojson::object &response_obj = response.get<picojson::object>();
    response_obj.insert(
        std::make_pair("listenerId", picojson::value(std::string("BadgeChangeListener"))));
    response_obj.insert(std::make_pair("appId", picojson::value(pkgname)));
    response_obj.insert(std::make_pair("count", picojson::value(std::to_string(count))));
    Instance::PostMessage(&that->instance_, response.serialize().c_str());
  }
}

bool BadgeManager::IsAppInstalled(const std::string &app_id) {
  LoggerD("Enter");

  if (app_id.empty()) {
    return false;
  }

  pkgmgrinfo_appinfo_h pkgmgrinfo_appinfo = nullptr;
  int ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &pkgmgrinfo_appinfo);

  return (ret == PMINFO_R_OK);
}

}  // namespace badge
}  // namespace extension
