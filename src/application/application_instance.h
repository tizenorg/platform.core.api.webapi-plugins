// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPLICATION_APPLICATION_INSTANCE_H_
#define APPLICATION_APPLICATION_INSTANCE_H_

#include "common/extension.h"

#include <string>
#include <list>

#include <app_manager.h>
#include <package-manager.h>
#include <package_manager.h>

#include "application.h"
#include "application_context.h"

namespace extension {
namespace application {

class ApplicationInstance
 : public common::ParsedInstance {
 public:
  ApplicationInstance(const std::string& app_id);
  virtual ~ApplicationInstance();

 private:
  
  void ApplicationManagerGetcurrentapplication(const picojson::value& args, picojson::object& out);
  void ApplicationManagerKill(const picojson::value& args, picojson::object& out);
  void ApplicationManagerLaunch(const picojson::value& args, picojson::object& out);
  void ApplicationManagerLaunchappcontrol(const picojson::value& args, picojson::object& out);
  void ApplicationManagerFindappcontrol(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappscontext(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappcontext(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappsinfo(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappinfo(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappcerts(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappshareduri(const picojson::value& args, picojson::object& out);
  void ApplicationManagerGetappmetadata(const picojson::value& args, picojson::object& out);
  void ApplicationManagerAddappinfoeventlistener(const picojson::value& args, picojson::object& out);
  void ApplicationManagerRemoveappinfoeventlistener(const picojson::value& args, picojson::object& out);
  void ApplicationExit(const picojson::value& args, picojson::object& out);
  void ApplicationHide(const picojson::value& args, picojson::object& out);
  void ApplicationGetrequestedappcontrol(const picojson::value& args, picojson::object& out);
  void RequestedApplicationControlReplyresult(const picojson::value& args, picojson::object& out);
  void RequestedApplicationControlReplyfailure(const picojson::value& args, picojson::object& out);

  ApplicationPtr GetCurrentApplication(const std::string app_id);
  ApplicationInformationPtr GetAppInfo(const std::string app_id);
  void Kill(const std::string contextId, int callbackId);
  void Launch(const std::string appId, int callbackId);
  ApplicationContextPtr GetAppContext(const std::string contextId);
  std::string GetAppSharedURI(std::string app_id);
  int AddAppInfoEventListener(const int& callback_id);
  static bool app_callback(package_info_app_component_type_e comp_type, const char *app_id, void *user_data);
  static int app_list_changed_cb(int id, const char *type, const char *package, const char *key, const char *val, const void *msg, void *data);
  void ReplyAppListChangedCallback(app_info_event_e event_type, const char *pkg_id, void *user_data);
  void RemoveAppInfoEventListener(long watch_id);
  int GetAppInstalledSize(const std::string& app_id);

  int get_installed_size();
  void set_installed_size(const int &installed_size);
  int get_watch_id_and_increase();
    
  std::string app_id_;
  int installed_size_;
  pkgmgr_client* manager_handle_;
  std::vector<int> callback_id_list_;
  int watch_id_;
  std::vector<std::string> app_list_;
};

} // namespace application
} // namespace extension

#endif // APPLICATION_APPLICATION_INSTANCE_H_
