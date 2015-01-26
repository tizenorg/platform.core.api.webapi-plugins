// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_INSTANCE_H_
#define SRC_APPLICATION_APPLICATION_INSTANCE_H_

// to handle app-control
#include <bundle.h>
#include <app.h>
#include <app_control.h>

// to get package name bye appid
#include <app_manager.h>

// to get cert info from package
#include <package-manager.h>
#include <package_manager.h>

#include <string>
#include <list>
#include <vector>
#include <map>

#include "common/extension.h"
#include "application/application.h"
#include "application/application_context.h"
#include "application/application_metadata.h"
#include "application/application_certificate.h"
#include "application/application_control.h"
#include "application/requested_application_control.h"

namespace extension {
namespace application {

class ApplicationInstance
  : public common::ParsedInstance {
 public:
  explicit ApplicationInstance(const std::string& app_id);
  virtual ~ApplicationInstance();

 private:
  void AppMgrGetCurrentApplication(const picojson::value& args,
    picojson::object& out);
  void AppMgrKill(const picojson::value& args, picojson::object& out);
  void AppMgrLaunch(const picojson::value& args, picojson::object& out);
  void AppMgrLaunchAppControl(const picojson::value& args,
    picojson::object& out);
  void AppMgrFindAppControl(const picojson::value& args,
    picojson::object& out);
  void AppMgrGetAppsContext(const picojson::value& args,
    picojson::object& out);
  void AppMgrGetAppContext(const picojson::value& args, picojson::object& out);
  void AppMgrGetAppsInfo(const picojson::value& args, picojson::object& out);
  void AppMgrGetAppInfo(const picojson::value& args, picojson::object& out);
  void AppMgrGetAppCerts(const picojson::value& args, picojson::object& out);
  void AppMgrGetAppSharedURI(const picojson::value& args,
    picojson::object& out);
  void AppMgrGetAppMetaData(const picojson::value& args,
    picojson::object& out);
  void AppMgrAddAppInfoEventListener(const picojson::value& args,
    picojson::object& out);
  void AppMgrRemoveAppInfoEventListener(const picojson::value& args,
    picojson::object& out);
  void AppExit(const picojson::value& args, picojson::object& out);
  void AppHide(const picojson::value& args, picojson::object& out);
  void AppGetRequestedAppControl(const picojson::value& args,
    picojson::object& out);
  void RequestedAppControlReplyResult(const picojson::value& args,
    picojson::object& out);
  void RequestedAppControlReplyFailure(const picojson::value& args,
    picojson::object& out);

  RequestedApplicationControlPtr GetRequestedAppControl();
  void ReplyResult(const std::string& caller_app_id,
    ApplicationControlDataArrayPtr app_ctr_data_array_ptr);
  void ReplyFailure(const std::string& caller_app_id);
  ApplicationPtr GetCurrentApplication(const std::string app_id);
  ApplicationInformationPtr GetAppInfo(const std::string app_id);
  void GetAppsInfo(const int& callback_id);
  void Kill(const std::string context_id, int callback_id);
  void Launch(const std::string app_id, int callback_id);
  void LaunchAppControl(const ApplicationControlPtr& app_ctr_ptr,
    const std::string& app_id, const int& callback_id);
  void FindAppControl(const ApplicationControlPtr& app_ctr_ptr,
    const int& callback_id);
  void GetAppsContext(const int& callback_id);
  ApplicationContextPtr GetAppContext(const std::string context_id);
  ApplicationCertificateArrayPtr GetAppCertificate(const std::string app_id);
  ApplicationMetaDataArrayPtr GetAppMetaData(const std::string app_id);
  std::string GetAppSharedURI(std::string app_id);
  int AddAppInfoEventListener(const int& callback_id);
  static bool app_callback(package_info_app_component_type_e comp_type,
    const char *app_id, void *user_data);
  static int app_list_changed_cb(int id, const char *type, const char *package,
    const char *key, const char *val, const void *msg, void *data);
  void ReplyAppListChangedCallback(app_info_event_e event_type,
    const char *pkg_id, void *user_data);
  void RemoveAppInfoEventListener(int watch_id);

  int get_watch_id_and_increase();

  std::string app_id_;
  pkgmgr_client* manager_handle_;
  std::vector<int> callback_id_list_;
  int watch_id_;
  std::vector<std::string> app_list_;
  std::map<std::string, app_control_h> reply_map_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_INSTANCE_H_
