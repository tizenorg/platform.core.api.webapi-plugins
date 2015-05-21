// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
