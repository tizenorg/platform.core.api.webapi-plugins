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

#ifndef SRC_APPLICATION_APPLICATION_MANAGER_H__
#define SRC_APPLICATION_APPLICATION_MANAGER_H__

#include <string>
#include <memory>

#include <package-manager.h>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace application {

class ApplicationInstance;

class ApplicationManager {
 public:
  explicit ApplicationManager(ApplicationInstance& instance);
  ~ApplicationManager();

  void GetCurrentApplication(const std::string& app_id, picojson::object* out);
  void Kill(const picojson::value& args);
  void Launch(const picojson::value& args);
  void LaunchAppControl(const picojson::value& args);
  void FindAppControl(const picojson::value& args);
  void GetAppsContext(const picojson::value& args);
  void GetAppContext(const picojson::value& args, picojson::object* out);
  void GetAppsInfo(const picojson::value& args);
  void GetAppInfo(const std::string& app_id, picojson::object* out);
  void GetAppCerts(const std::string& app_id, picojson::object* out);
  void GetAppSharedUri(const std::string& app_id, picojson::object* out);
  void GetAppMetaData(const std::string& app_id, picojson::object* out);
  void StartAppInfoEventListener(picojson::object* out);
  void StopAppInfoEventListener();
  void GetApplicationInformationSize(const picojson::value& args, picojson::object* out);
  void AsyncResponse(common::PlatformResult& result, std::shared_ptr<picojson::value>* response);

 private:
  char* GetPackageId(const std::string& app_id);

  pkgmgr_client* pkgmgr_client_handle_;
  ApplicationInstance& instance_;
};

} // namespace application
} // namespace extension

#endif // SRC_APPLICATION_APPLICATION_MANAGER_H__