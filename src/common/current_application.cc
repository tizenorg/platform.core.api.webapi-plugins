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

CurrentApplication::CurrentApplication() :
    pid_(getpid()),
    app_id_(FetchApplicationId()),
    package_id_(FetchPackageId()) {
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
    LoggerE("Failed to get application ID.");
  }

  free(tmp_str);

  return app_id;
}

std::string CurrentApplication::FetchPackageId() const {
  LoggerD("Enter");
  std::string package_id;
  char* tmp_str = nullptr;

  const int ret = package_manager_get_package_id_by_app_id(app_id_.c_str(),
                                                           &tmp_str);

  if ((PACKAGE_MANAGER_ERROR_NONE == ret) && (nullptr != tmp_str)) {
    package_id = tmp_str;
  } else {
    LoggerE("Failed to get package ID.");
  }

  free(tmp_str);

  return package_id;
}

}  // namespace common
