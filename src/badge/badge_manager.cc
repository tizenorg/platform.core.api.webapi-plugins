// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge_manager.h"

#include <badge.h>
#include <badge_internal.h>
#include <package_manager.h>
#include <pkgmgr-info.h>

#include "badge_instance.h"
#include "common/logger.h"
#include "common/converter.h"
#include "common/platform_exception.h"

using namespace common;

namespace extension {
namespace badge {

std::set<std::string> BadgeManager::watched_applications_;
bool BadgeManager::is_cb_registered_ = false;

BadgeManager::BadgeManager() {}

BadgeManager::~BadgeManager() {
  if (is_cb_registered_) {
    if (!watched_applications_.empty()) watched_applications_.clear();
    int ret = badge_unregister_changed_cb(badge_changed_cb);
    if (ret != BADGE_ERROR_NONE) {
      LoggerE("Unknown error : %d", ret);
    }
    is_cb_registered_ = false;
  }
}

BadgeManager* BadgeManager::GetInstance() {
  static BadgeManager instance;
  return &instance;
}

void BadgeManager::setBadgeCount(std::string appId, unsigned int count) {
  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badgeExist = false;
  const char* app_id = appId.c_str();

  if (!isAppInstalled(appId)) {
    LoggerE("fail to get pkgId");
    throw InvalidValuesException("InvalidValues error : appId");
  }

  ret = badge_is_existing(app_id, &badgeExist);
  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    throw UnknownException("Unknown error");
  }

  if (!badgeExist) {
    ret = badge_create(app_id, app_id);
    LoggerD("badge create : %", ret);

    if (ret == BADGE_ERROR_PERMISSION_DENIED) {
      LoggerE("Security error");
      throw SecurityException("Security error");
#ifdef PROFILE_WEARABLE
    } else if (ret == BADGE_ERROR_INVALID_DATA) {
#else
    } else if (ret == BADGE_ERROR_INVALID_PARAMETER) {
#endif
      LoggerE("Invalid values error");
      throw InvalidValuesException("Invalid values error");
    } else if (ret != BADGE_ERROR_NONE && ret != BADGE_ERROR_ALREADY_EXIST) {
      LoggerE("Unknown error");
      throw InvalidValuesException("Unknown error");
    }
  }

  ret = badge_set_count(app_id, count);
  LoggerE("set ret : %d count :%d ", ret, count);

  if (ret == BADGE_ERROR_PERMISSION_DENIED) {
    LoggerE("Security error");
    throw SecurityException("Security error");
#ifdef PROFILE_WEARABLE
  } else if (ret == BADGE_ERROR_INVALID_DATA) {
#else
  } else if (ret == BADGE_ERROR_INVALID_PARAMETER) {
#endif
    LoggerE("Invalid values error");
    throw InvalidValuesException("Invalid values error");
  } else if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    throw InvalidValuesException("Unknown error");
  }
}

unsigned int BadgeManager::getBadgeCount(std::string appId) {
  LoggerD("Enter");

  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badgeExist = false;
  if (!isAppInstalled(appId)) {
    LoggerE("fail to get pkgId");
    throw InvalidValuesException("InvalidValues error : appId");
  }
  ret = badge_is_existing(appId.c_str(), &badgeExist);
  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    throw UnknownException("Platform error while checking badge.");
  }
  LoggerD("badge exist : %d", badgeExist);
  unsigned int count = 0;

  if (!badgeExist) {
    throw UnknownException("badge not exist. appId: " + appId);
  }

  ret = badge_get_count(appId.c_str(), &count);

  if (ret == BADGE_ERROR_NONE) {
    LoggerD("success get ret : %d count : ", count);
    return count;
  } else if (ret == BADGE_ERROR_PERMISSION_DENIED) {
    LoggerE("Security error");
    throw SecurityException("Security error.");
#ifdef PROFILE_WEARABLE
  } else if (ret == BADGE_ERROR_INVALID_DATA) {
#else
  } else if (ret == BADGE_ERROR_INVALID_PARAMETER) {
#endif
    LoggerE("Invalid values error");
    throw InvalidValuesException("InvalidValues error : appId");
  } else {
    LoggerE("Unknown error : %d", ret);
    throw UnknownException("Unknown error");
  }
}

void BadgeManager::addChangeListener(const JsonObject& obj) {
  LoggerD("entered here");
  auto& items = FromJson<picojson::array>(obj, "appIdList");
  for (auto item : items) {
    watched_applications_.insert(common::JsonCast<std::string>(item));
  }
  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  if (!is_cb_registered_) {
    ret = badge_register_changed_cb(badge_changed_cb, this);
    if (ret != BADGE_ERROR_NONE) {
      LoggerE("Unknown error %d:", ret);
      throw UnknownException("Platform error while adding listener.");
    }
    is_cb_registered_ = true;
  }
}

void BadgeManager::removeChangeListener(const JsonObject& obj) {
  auto& items = FromJson<picojson::array>(obj, "appIdList");
  for (auto item : items) {
    watched_applications_.erase(common::JsonCast<std::string>(item));
  }
  if (watched_applications_.empty() && is_cb_registered_) {
    int ret = badge_unregister_changed_cb(badge_changed_cb);
    if (ret != BADGE_ERROR_NONE) {
      LoggerE("Unknown error : %d", ret);
    }
    is_cb_registered_ = false;
  }
}

void BadgeManager::badge_changed_cb(unsigned int action, const char* pkgname,
                                    unsigned int count, void* user_data) {
  if (action != BADGE_ACTION_SERVICE_READY &&
      watched_applications_.find(pkgname) != watched_applications_.end()) {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj.insert(
        std::make_pair("listenerId", std::string("BadgeChangeListener")));
    response_obj.insert(std::make_pair("appId", pkgname));
    response_obj.insert(std::make_pair("count", std::to_string(count)));
    BadgeInstance::GetInstance().PostMessage(response.serialize().c_str());
  }
}

bool BadgeManager::isAppInstalled(const std::string& appId) {
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  pkgmgrinfo_appinfo_h pkgmgrinfo_appinfo;
  if (appId.empty()) {
    return false;
  }
  ret = pkgmgrinfo_appinfo_get_appinfo(appId.c_str(), &pkgmgrinfo_appinfo);
  if (ret != PMINFO_R_OK) {
    return false;
  }
  return true;
}

}  // namespace badge
}  // namespace extension
