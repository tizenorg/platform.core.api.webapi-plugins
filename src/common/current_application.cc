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
 
#include "common/current_application.h"

#include <app_manager.h>
#include <package_manager.h>
#include <unistd.h>

#include "common/logger.h"
#include "common/scope_exit.h"

namespace common {

CurrentApplication& CurrentApplication::GetInstance() {
  LoggerD("Enter");
  static CurrentApplication current_application;
  return current_application;
}

pid_t CurrentApplication::GetProcessId() const {
  LoggerD("Enter");
  return pid_;
}

std::string CurrentApplication::GetApplicationId() const {
  LoggerD("Enter");
  return app_id_;
}

std::string CurrentApplication::GetPackageId() const {
  LoggerD("Enter");
  return package_id_;
}

std::string CurrentApplication::GetRoot() const {
  LoggerD("Enter");
  return root_;
}

CurrentApplication::CurrentApplication() :
    pid_(getpid()),
    app_id_(FetchApplicationId()),
    package_id_(FetchPackageId()),
    root_(FetchRoot()) {
  LoggerD("Enter");
}

std::string CurrentApplication::FetchApplicationId() const {
  LoggerD("Enter");
  std::string app_id;
  char* tmp_str = nullptr;

  const int ret = app_manager_get_app_id(pid_, &tmp_str);

  if ((APP_MANAGER_ERROR_NONE == ret) && (nullptr != tmp_str)) {
    app_id = tmp_str;
  } else {
    LoggerE("Failed to get application ID: %d (%s)", ret, get_error_message(ret));
  }

  free(tmp_str);

  return app_id;
}

std::string CurrentApplication::FetchPackageId() const {
  LoggerD("Enter");
  std::string package_id;
  app_info_h app_info;
  int err = app_info_create(app_id_.c_str(), &app_info);
  if (APP_MANAGER_ERROR_NONE != err) {
    LoggerE("Can't create app info handle from appId %s: %d (%s)",
            app_id_.c_str(), err, get_error_message(err));
    return std::string();
  }
  SCOPE_EXIT {
    app_info_destroy(app_info);
  };

  char* package = nullptr;
  err = app_info_get_package(app_info, &package);
  if (APP_MANAGER_ERROR_NONE != err) {
    LoggerE("Can't get package name from app info: %d (%s)", err,
            get_error_message(err));
  } else {
    package_id = package;
  }

  free(package);

  return package_id;
}

std::string CurrentApplication::FetchRoot() const {
  LoggerD("Enter");

  if(package_id_.empty()) {
    LoggerE("Can't get package id, no root path can be obtained");
    return std::string();
  }

  package_info_h pkg_info;
  int err = package_info_create(package_id_.c_str(), &pkg_info);
  if (PACKAGE_MANAGER_ERROR_NONE != err) {
    LoggerE("Can't create package info handle from pkg (%s)", get_error_message(err));
    return std::string();
  }
  SCOPE_EXIT {
    package_info_destroy(pkg_info);
  };

  char* root = nullptr;
  err = package_info_get_root_path(pkg_info, &root);
  if (PACKAGE_MANAGER_ERROR_NONE != err || nullptr == root) {
    LoggerE("Can't get root_ path from package info (%s)", get_error_message(err));
    return std::string();
  }

  std::string ret(root);
  free(root);

  LoggerD("Exit");
  return ret;
}

}  // namespace common
