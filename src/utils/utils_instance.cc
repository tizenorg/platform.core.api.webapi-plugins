// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include <utility>
#include <unistd.h>
#include <app_manager.h>
#include <pkgmgr-info.h>
#include "common/scope_exit.h"
#include "common/logger.h"
#include "utils/utils_instance.h"

using common::PlatformResult;
using common::ErrorCode;

namespace extension {
namespace utils {

UtilsInstance::UtilsInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  LoggerD("Entered");
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&UtilsInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&UtilsInstance::x, this, _1, _2));

  REGISTER_SYNC("Utils_getPkgApiVersion", GetPkgApiVersion);
  REGISTER_SYNC("Utils_checkPrivilegeAccess", CheckPrivilegeAccess);
  REGISTER_SYNC("Utils_checkBackwardCompabilityPrivilegeAccess", CheckBackwardCompabilityPrivilegeAccess);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

void UtilsInstance::GetPkgApiVersion(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  char* app_id = nullptr;
  char* pkgid = nullptr;
  app_info_h app_handle = nullptr;
  pkgmgrinfo_pkginfo_h pkginfo_handle = nullptr;
  char *api_version = nullptr;

  SCOPE_EXIT {
    if (app_id) {
      free(app_id);
    }
    if (pkgid) {
      free(pkgid);
    }
    if (app_handle) {
      app_info_destroy(app_handle);
    }
    if (pkginfo_handle) {
      pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_handle);
    }
  };

  pid_t pid = getpid();
  int ret = app_manager_get_app_id(pid, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Failed to get app id");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get app id"), &out);
    return;
  }

  ret = app_info_create(app_id, &app_handle);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LoggerE("Fail to get app info");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Fail to get app info"), &out);
    return;
  }

  ret = app_info_get_package(app_handle, &pkgid);
  if ((ret != APP_MANAGER_ERROR_NONE) || (pkgid == nullptr)) {
    LoggerE("Fail to get pkg id");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Fail to get pkg id"), &out);
    return;
  }

  ret = pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkginfo_handle);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get pkginfo_h");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Fail to get pkginfo_h"), &out);
    return;
  }

  ret = pkgmgrinfo_pkginfo_get_api_version(pkginfo_handle, &api_version);
  if (ret != PMINFO_R_OK) {
    LoggerE("Fail to get api version");
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Fail to get api version"), &out);
    return;
  }

  ReportSuccess(picojson::value(api_version), out);
}

void UtilsInstance::CheckPrivilegeAccess(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const auto& privilege = args.get("privilege").to_str();
  CHECK_PRIVILEGE_ACCESS(privilege, &out);
  ReportSuccess(out);
}

void UtilsInstance::CheckBackwardCompabilityPrivilegeAccess(const picojson::value& args,
                                                            picojson::object& out) {
  LoggerD("Entered");
  const auto& current_priv = args.get("current_privilege").to_str();
  const auto& prev_priv = args.get("previous_privilege").to_str();

  CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(current_priv, prev_priv, &out);
  ReportSuccess(out);
}

}  // namespace utils
}  // namespace extension
