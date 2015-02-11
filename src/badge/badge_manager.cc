// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge/badge_manager.h"

#include <aul.h>
#include <badge.h>
#include <badge_internal.h>
#include <fcntl.h>
#include <package_manager.h>
#include <pkgmgr-info.h>

#include "badge/badge_instance.h"
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

BadgeManager *BadgeManager::GetInstance() {
  static BadgeManager instance;
  return &instance;
}

PlatformResult BadgeManager::setBadgeCount(std::string appId,
                                           unsigned int count) {
  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badgeExist = false;
  const char *app_id = appId.c_str();

  if (!isAppInstalled(appId)) {
    LoggerE("fail to get pkgId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : appId");
  }

  ret = badge_is_existing(app_id, &badgeExist);
  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }

  if (!badgeExist) {
    PlatformResult status = checkPermisionForCreatingBadge(app_id);
    if (status.IsError()) return status;

    ret = badge_create(app_id, app_id);
    LoggerD("badge create : %", ret);

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

  ret = badge_set_count(app_id, count);
  LoggerE("set ret : %d count :%d ", ret, count);

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
  } else if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult BadgeManager::getBadgeCount(std::string appId,
                                           unsigned int *count) {
  LoggerD("Enter");

  assert(count);

  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badgeExist = false;
  if (!isAppInstalled(appId)) {
    LoggerE("fail to get pkgId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : appId");
  }
  ret = badge_is_existing(appId.c_str(), &badgeExist);
  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while checking badge.");
  }
  LoggerD("badge exist : %d", badgeExist);

  if (!badgeExist) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "badge not exist. appId: " + appId);
  }

  *count = 0;
  ret = badge_get_count(appId.c_str(), count);

  if (ret == BADGE_ERROR_NONE) {
    LoggerD("success get ret : %d count : ", *count);
    return PlatformResult(ErrorCode::NO_ERROR);
  } else if (ret == BADGE_ERROR_PERMISSION_DENIED) {
    LoggerE("Security error");
    return PlatformResult(ErrorCode::SECURITY_ERR, "Security error.");
#ifdef PROFILE_WEARABLE
  } else if (ret == BADGE_ERROR_INVALID_DATA) {
#else
  } else if (ret == BADGE_ERROR_INVALID_PARAMETER) {
#endif
    LoggerE("Invalid values error");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : appId");
  } else {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }
}

PlatformResult BadgeManager::addChangeListener(const JsonObject &obj) {
  LoggerD("entered here");
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

void BadgeManager::removeChangeListener(const JsonObject &obj) {
  auto &items = FromJson<picojson::array>(obj, "appIdList");
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

void BadgeManager::badge_changed_cb(unsigned int action, const char *pkgname, unsigned int count,
                                    void *user_data) {
  if (action != BADGE_ACTION_SERVICE_READY &&
      watched_applications_.find(pkgname) != watched_applications_.end()) {
    picojson::value response = picojson::value(picojson::object());
    picojson::object &response_obj = response.get<picojson::object>();
    response_obj.insert(std::make_pair("listenerId", std::string("BadgeChangeListener")));
    response_obj.insert(std::make_pair("appId", pkgname));
    response_obj.insert(std::make_pair("count", std::to_string(count)));
    BadgeInstance::GetInstance().PostMessage(response.serialize().c_str());
  }
}

bool BadgeManager::isAppInstalled(const std::string &appId) {
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

PlatformResult BadgeManager::checkPermisionForCreatingBadge(const char *appId) {
  if (!appId) {
    LoggerE("InvalidValues error : appId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : appId");
  }

  char *caller_appid = NULL;
  caller_appid = getPkgnameByPid();

  if (!caller_appid) {
    LoggerE("fail to get caller pkgId");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting caller pkgId.");
  }

  char *caller_pkgname = NULL;
  caller_pkgname = getPkgnameByAppid(caller_appid);
  if (!caller_pkgname) {
    if (caller_appid) {
      free(caller_appid);
    }
    LoggerE("fail to get caller pkgId");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting caller pkgId.");
  }

  char *pkgname = NULL;
  pkgname = getPkgnameByAppid(appId);
  if (!pkgname) {
    if (caller_appid) {
      free(caller_appid);
    }
    if (caller_pkgname) {
      free(caller_pkgname);
    }
    LoggerE("fail to get pkgId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : appId");
  }

  bool flag = true;
  if (isSameCertInfo(caller_pkgname, pkgname) != 1) {
    LoggerE("The author signature is not match");
    flag = false;
  }

  if (caller_appid) {
    free(caller_appid);
  }
  if (caller_pkgname) {
    free(caller_pkgname);
  }
  if (pkgname) {
    free(pkgname);
  }

  if (!flag) {
    LoggerE("Security error - cannot create badge");
    return PlatformResult(ErrorCode::SECURITY_ERR,
                          "Security error - cannot create badge");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

char *BadgeManager::getPkgnameByAppid(const char *appId) {
  char *pkgId = NULL;
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  if (!appId) {
    LoggerE("appId is null");
    return NULL;
  }

  ret = package_manager_get_package_id_by_app_id(appId, &pkgId);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("fail to get caller pkgId : ", ret);
    return NULL;
  }
  if (!pkgId) {
    LoggerE("fail to get caller pkgId");
    return NULL;
  }

  return pkgId;
}

char *BadgeManager::getPkgnameByPid() {
  char *pkgname = NULL;
  int pid = 0;
  int ret = AUL_R_OK;
  int fd = 0;
  long max = 4096;

  pid = getpid();
  pkgname = static_cast<char *>(malloc(max));
  if (!pkgname) {
    LoggerE("fail to alloc memory");
    return NULL;
  }
  memset(pkgname, 0x00, max);

  ret = aul_app_get_pkgname_bypid(pid, pkgname, max);
  if (ret != AUL_R_OK) {
    fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0) {
      free(pkgname);
      return NULL;
    }

    ret = read(fd, pkgname, max - 1);
    if (ret <= 0) {
      close(fd);
      free(pkgname);
      return NULL;
    }

    close(fd);
  }

  if (pkgname[0] == '\0') {
    free(pkgname);
    return NULL;
  } else
    return pkgname;
}

int BadgeManager::isSameCertInfo(const char *caller, const char *pkgname) {
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  package_manager_compare_result_type_e compare_result = PACKAGE_MANAGER_COMPARE_MISMATCH;

  if (!caller) {
    return 0;
  }
  if (!pkgname) {
    return 0;
  }

  LoggerE("pkgname : %d caller : ", pkgname, caller);

  ret = package_manager_compare_package_cert_info(pkgname, caller, &compare_result);

  LoggerE("result : %d %d", ret, compare_result);
  if (ret == PACKAGE_MANAGER_ERROR_NONE && compare_result == PACKAGE_MANAGER_COMPARE_MATCH) {
    return 1;
  }

  return 0;
}

}  // namespace badge
}  // namespace extension
