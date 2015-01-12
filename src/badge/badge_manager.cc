// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge_manager.h"

#include "common/logger.h"
#include "common/platform_exception.h"

#include <badge.h>
#include <badge_internal.h>
#include <package_manager.h>
#include <aul.h>
#include <glib.h>

#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace common;

namespace extension {
namespace badge {

BadgeManager::BadgeManager() {}

BadgeManager::~BadgeManager() {}

BadgeManager *BadgeManager::GetInstance() {
  static BadgeManager instance;
  return &instance;
}

void BadgeManager::setBadgeCount(std::string appId, unsigned int count) {
  int ret = BADGE_ERROR_SERVICE_NOT_READY;
  bool badgeExist = false;
  const char *app_id = appId.c_str();

  ret = badge_is_existing(app_id, &badgeExist);
  if (ret != BADGE_ERROR_NONE) {
    LoggerE("Unknown error : %d", ret);
    throw UnknownException("Unknown error");
  }

  if (badgeExist == false) {
    if (!(checkPermisionForCreatingBadge(app_id))) {
      throw SecurityException("The author signature is not match");
    }

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

void BadgeManager::CheckErrorCode(int err) {
  switch (err) {
    case BADGE_ERROR_NONE:
      return;
    case BADGE_ERROR_INVALID_PARAMETER:
      throw InvalidValuesException("Invalid parameter");
    case BADGE_ERROR_PERMISSION_DENIED:
      throw SecurityException(
          "The application does not have the privilege to call this method");
    case BADGE_ERROR_IO_ERROR:
      throw UnknownException("Error from I/O");
    case BADGE_ERROR_SERVICE_NOT_READY:
      throw UnknownException("Service is not ready");
    case BADGE_ERROR_OUT_OF_MEMORY:
      throw UnknownException("Out of memory");
    case BADGE_ERROR_FROM_DB:
      throw UnknownException("Error from DB");
    case BADGE_ERROR_ALREADY_EXIST:
      throw UnknownException("Already exist");
    case BADGE_ERROR_FROM_DBUS:
      throw UnknownException("Error from DBus");
    case BADGE_ERROR_NOT_EXIST:
      throw UnknownException("Not exist");
    default:
      throw UnknownException("Unknown error");
  }
}

bool BadgeManager::checkPermisionForCreatingBadge(const char *appId) {
  if (!appId) {
    LoggerE("InvalidValues error : appId");
    throw InvalidValuesException("InvalidValues error : appId");
  }

  char *caller_appid = NULL;
  caller_appid = _badge_get_pkgname_by_pid();

  if (!caller_appid) {
    LoggerE("fail to get caller pkgId");
    throw UnknownException("Platform error while getting caller pkgId.");
  }

  char *caller_pkgname = NULL;
  caller_pkgname = _badge_get_pkgname_by_appid(caller_appid);
  if (!caller_pkgname) {
    if (caller_appid) {
      free(caller_appid);
    }
    LoggerE("fail to get caller pkgId");
    throw UnknownException("Platform error while getting caller pkgId.");
  }

  char *pkgname = NULL;
  pkgname = _badge_get_pkgname_by_appid(appId);
  if (!pkgname) {
    if (caller_appid) {
      free(caller_appid);
    }
    if (caller_pkgname) {
      free(caller_pkgname);
    }
    LoggerE("fail to get pkgId");
    throw InvalidValuesException("InvalidValues error : appId");
  }

  bool flag = false;
  if (_badge_is_same_certinfo(caller_pkgname, pkgname) == 1) {
    flag = true;
  } else {
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

  return flag;
}

char *BadgeManager::_badge_get_pkgname_by_appid(const char *appId) {
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

char *BadgeManager::_badge_get_pkgname_by_pid() {
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

int BadgeManager::_badge_is_same_certinfo(const char *caller,
                                          const char *pkgname) {
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
