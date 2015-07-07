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
#include <package_manager.h>
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
  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badge_exist = false;
  const char *app_id_str = app_id.c_str();

  if (!IsAppInstalled(app_id)) {
    LoggerE("fail to get pkgId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : app_id");
  }

  ret = badge_is_existing(app_id_str, &badge_exist);
  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown error");
  }

  if (!badge_exist) {
    PlatformResult status = CheckPermisionForCreatingBadge(app_id_str);
    if (status.IsError()) return status;

    ret = badge_create(app_id_str, app_id_str);
    LoggerD("badge create : %d", ret);

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

PlatformResult BadgeManager::GetBadgeCount(const std::string& app_id,
                                           unsigned int *count) {
  LoggerD("Enter");
  Assert(count);

  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badge_exist = false;
  if (!IsAppInstalled(app_id)) {
    LoggerE("fail to get pkgId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : app_id");
  }
  ret = badge_is_existing(app_id.c_str(), &badge_exist);
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

  switch (ret) {
    case BADGE_ERROR_NONE:
      LoggerD("success get ret : %d count : ", *count);
      return PlatformResult(ErrorCode::NO_ERROR);
    case BADGE_ERROR_PERMISSION_DENIED:
      LoggerE("Security error");
      return PlatformResult(ErrorCode::SECURITY_ERR, "Security error.");
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
    that->instance_.PostMessage(response.serialize().c_str());
  }
}

bool BadgeManager::IsAppInstalled(const std::string &app_id) {
  LoggerD("Enter");
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  pkgmgrinfo_appinfo_h pkgmgrinfo_appinfo;
  if (app_id.empty()) {
    return false;
  }
  ret = pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &pkgmgrinfo_appinfo);
  if (ret != PMINFO_R_OK) {
    return false;
  }
  return true;
}

PlatformResult BadgeManager::CheckPermisionForCreatingBadge(
    const char *app_id) {
  LoggerD("Enter");
  if (!app_id) {
    LoggerE("InvalidValues error : app_id");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : app_id");
  }

  char *caller_appid = NULL;
  caller_appid = GetPkgnameByPid();

  if (!caller_appid) {
    LoggerE("fail to get caller pkgId");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting caller pkgId.");
  }

  char *caller_pkgname = NULL;
  caller_pkgname = GetPkgnameByAppid(caller_appid);
  if (!caller_pkgname) {
    if (caller_appid) {
      free(caller_appid);
    }
    LoggerE("fail to get caller pkgId");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform error while getting caller pkgId.");
  }

  char *pkgname = NULL;
  pkgname = GetPkgnameByAppid(app_id);
  if (!pkgname) {
    if (caller_appid) {
      free(caller_appid);
    }
    if (caller_pkgname) {
      free(caller_pkgname);
    }
    LoggerE("fail to get pkgId");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "InvalidValues error : app_id");
  }

  bool flag = true;
  if (IsSameCertInfo(caller_pkgname, pkgname) != 1) {
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

char *BadgeManager::GetPkgnameByAppid(const char *app_id) {
  LoggerD("Enter");
  char *pkg_id = NULL;
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  if (!app_id) {
    LoggerE("app_id is null");
    return NULL;
  }

  ret = package_manager_get_package_id_by_app_id(app_id, &pkg_id);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("fail to get caller pkg_id : ", ret);
    return NULL;
  }
  if (!pkg_id) {
    LoggerE("fail to get caller pkg_id");
    return NULL;
  }

  return pkg_id;
}

char *BadgeManager::GetPkgnameByPid() {
  LoggerD("Enter");
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

int BadgeManager::IsSameCertInfo(const char *caller, const char *pkgname) {
  LoggerD("Enter");
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  package_manager_compare_result_type_e compare_result =
      PACKAGE_MANAGER_COMPARE_MISMATCH;

  if (!caller) {
    return 0;
  }
  if (!pkgname) {
    return 0;
  }

  LoggerE("pkgname : %d caller : ", pkgname, caller);

  ret = package_manager_compare_package_cert_info(pkgname, caller,
                                                  &compare_result);

  LoggerE("result : %d %d", ret, compare_result);
  if (ret == PACKAGE_MANAGER_ERROR_NONE &&
      compare_result == PACKAGE_MANAGER_COMPARE_MATCH) {
    return 1;
  }

  return 0;
}

}  // namespace badge
}  // namespace extension
