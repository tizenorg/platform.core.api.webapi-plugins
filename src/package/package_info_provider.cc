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

#include "package/package_info_provider.h"

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <app_manager.h>
#include <package_manager.h>
#include <package-manager.h>

#include <functional>

#include "common/logger.h"
#include "common/scope_exit.h"
#include "common/tools.h"

namespace extension {
namespace package {

using common::UnknownException;
using common::NotFoundException;

using common::ErrorCode;
using common::PlatformResult;
using common::tools::ReportError;
using common::tools::ReportSuccess;

#define REPORT_ERROR(out, exception) \
  out["status"] = picojson::value("error"); \
  out["error"] = exception.ToJSON();

static int PackageInfoGetListCb(
    const pkgmgrinfo_pkginfo_h info, void *user_data) {
  LoggerD("Enter");

  picojson::array* array_data = static_cast<picojson::array*>(user_data);
  if ( !array_data ) {
    LoggerE("user_data is NULL");
    return PMINFO_R_ERROR;
  }

  picojson::object object_info;
  if ( PackageInfoProvider::ConvertToPackageToObject(
      info, object_info) ) {
    array_data->push_back(picojson::value(object_info));
  }

  return PMINFO_R_OK;
}

void PackageInfoProvider::GetPackagesInfo(
    picojson::object& out) {
  LoggerD("Enter");

  clock_t start_time, end_time;
  start_time = clock();

  picojson::array array_data;
  if ( pkgmgrinfo_pkginfo_get_list(PackageInfoGetListCb, &array_data)
      != PMINFO_R_OK ) {
    LoggerE("Failed to get package information");
    REPORT_ERROR(out, UnknownException("Any other platform error occurs"));
    return;
  }

  end_time = clock();
  LoggerD(">>>>>>>>>>>>>>> GetPackagesInfo Time : %f s\n",
      (static_cast<double>(end_time-start_time)) / CLOCKS_PER_SEC);

  LoggerD("status: success");
  out["status"] = picojson::value("success");
  out["informationArray"] = picojson::value(array_data);
}

void PackageInfoProvider::GetPackageInfo(picojson::object& out) {
  LoggerD("Enter");

  char* package_id = NULL;
  if ( GetCurrentPackageId(&package_id) ) {
    GetPackageInfo(package_id, out);
    free(package_id);
  } else {
    LoggerE("Failed to get current package ID");
    REPORT_ERROR(out, NotFoundException("The package with the specified ID is not found"));
  }
}

void PackageInfoProvider::GetPackageInfo(
    const char* package_id, picojson::object& out) {
  LoggerD("Enter");

  if ( strlen(package_id) <= 0 ) {
    LoggerE("Wrong Package ID");
    REPORT_ERROR(out, NotFoundException("The package with the specified ID is not found"));
    return;
  }

  pkgmgrinfo_pkginfo_h info;
  if ( pkgmgrinfo_pkginfo_get_pkginfo(package_id, &info)
      != PMINFO_R_OK ) {
    LoggerE("Failed to get pkginfo");
    REPORT_ERROR(out, NotFoundException("The package with the specified ID is not found"));
    return;
  }

  picojson::object object_info;
  if ( !ConvertToPackageToObject(info, object_info) ) {
    LoggerE("Failed to convert pkginfo to object");
    REPORT_ERROR(out, UnknownException(
        "The package information cannot be retrieved " \
        "because of a platform error"));
    return;
  }

  pkgmgrinfo_pkginfo_destroy_pkginfo(info);
  out["status"] = picojson::value("success");
  out["result"] = picojson::value(object_info);
}

static bool PackageAppInfoCb(
    package_info_app_component_type_e comp_type,
    const char *app_id,
    void *user_data) {
  LoggerD("Enter");

  picojson::array* array_data =
      static_cast<picojson::array*>(user_data);
  if ( !array_data ) {
    LoggerE("user_data is NULL");
    return false;
  }

  array_data->push_back(picojson::value(app_id));
  return true;
}

bool PackageInfoProvider:: ConvertToPackageToObject(
    const pkgmgrinfo_pkginfo_h info, picojson::object& out) {
  int ret = 0;

  char* id = NULL;
  ret = pkgmgrinfo_pkginfo_get_pkgid(info, &id);
  if ( (ret != PMINFO_R_OK) || (id == NULL) ) {
    LoggerE("Failed to get package id");
    return false;
  }
  out["id"] = picojson::value(id);

  char* name = NULL;
  ret = pkgmgrinfo_pkginfo_get_label(info, &name);
  if ( (ret != PMINFO_R_OK) || (name == NULL) ) {
    LoggerE("[%s] Failed to get package name", id);
    return false;
  }
  out["name"] = picojson::value(name);

  char* iconPath = NULL;
  ret = pkgmgrinfo_pkginfo_get_icon(info, &iconPath);
  if ( (ret != PMINFO_R_OK) || (iconPath == NULL) ) {
    LoggerE("[%s] Failed to get package iconPath", id);
    return false;
  }
  out["iconPath"] = picojson::value(iconPath);

  char* version = NULL;
  ret = pkgmgrinfo_pkginfo_get_version(info, &version);
  if ( (ret != PMINFO_R_OK) || (version == NULL) ) {
    LoggerE("[%s] Failed to get package version", id);
    return false;
  }
  out["version"] = picojson::value(version);

  int lastModified = 0;
  ret = pkgmgrinfo_pkginfo_get_installed_time(info, &lastModified);
  if ( (ret != PMINFO_R_OK) ) {
    LoggerE("[%s] Failed to get package lastModified", id);
    return false;
  }
  // This value will be converted into JavaScript Date object
  double lastModified_double = lastModified * 1000.0;
  out["lastModified"] = picojson::value(lastModified_double);

  char* author = NULL;
  ret = pkgmgrinfo_pkginfo_get_author_name(info, &author);
  if ( (ret != PMINFO_R_OK) || (author == NULL) ) {
    LoggerE("[%s] Failed to get package author", id);
    return false;
  }
  out["author"] = picojson::value(author);

  char* description = NULL;
  ret = pkgmgrinfo_pkginfo_get_description(info, &description);
  if ( (ret != PMINFO_R_OK) || (description == NULL) ) {
    LoggerE("[%s] Failed to get package description", id);
    return false;
  }
  out["description"] = picojson::value(description);

  package_info_h package_info;
  ret = package_info_create(id, &package_info);
  if ( ret != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to create package info");
    return false;
  }

  SCOPE_EXIT {
    if (PACKAGE_MANAGER_ERROR_NONE != package_info_destroy(package_info)) {
      LoggerE("Failed to destroy package info");
    }
  };

  picojson::array array_data;
  ret = package_info_foreach_app_from_package(package_info,
      PACKAGE_INFO_ALLAPP, PackageAppInfoCb, &array_data);
  if ( ret != PACKAGE_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to get app info");
    return false;
  }
  out["appIds"] = picojson::value(array_data);

  return true;
}

namespace {

void GetSize(const std::string& id, int service_mode, picojson::object* out) {
  LoggerD("Enter");
  pkgmgr_client* pc = pkgmgr_client_new(PC_REQUEST);
  int size = pkgmgr_client_request_service(PM_REQUEST_GET_SIZE, service_mode,
                                           pc,
                                           NULL,
                                           id.c_str(), NULL, NULL, NULL);
  pkgmgr_client_free(pc);

  if (size < 0) {
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get size"),
                out);
  } else {
    ReportSuccess(picojson::value(static_cast<double>(size)), *out);
  }
}

}  // namespace

void PackageInfoProvider::GetTotalSize(const std::string& id,
                                       picojson::object* out) {
  LoggerD("Enter");
  GetSize(id, PM_GET_TOTAL_SIZE, out);
}

void PackageInfoProvider::GetDataSize(const std::string& id,
                                      picojson::object* out) {
  LoggerD("Enter");
  GetSize(id, PM_GET_DATA_SIZE, out);
}

bool PackageInfoProvider::GetCurrentPackageId(
    char** package_id) {
  LoggerD("Enter");

  int ret = 0;
  char *app_id = NULL;

  int pid = getpid();
  ret = app_manager_get_app_id(pid, &app_id);
  if ( ret != APP_MANAGER_ERROR_NONE ) {
    LoggerE("Failed to get app id");
    return false;
  }

  app_info_h handle;
  ret = app_info_create(app_id, &handle);
  free(app_id);
  if ( ret != APP_MANAGER_ERROR_NONE ) {
    LoggerE("Fail to get app info");
    return false;
  }

  ret = app_info_get_package(handle, package_id);
  app_info_destroy(handle);
  if ( (ret != APP_MANAGER_ERROR_NONE) || (*package_id == NULL) ) {
    LoggerE("Fail to get pkg id");
    return false;
  }

  return true;
}



#undef REPORT_ERROR

}  // namespace package
}  // namespace extension
