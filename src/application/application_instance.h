// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_INSTANCE_H_
#define SRC_APPLICATION_APPLICATION_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "application/application_manager.h"
#include "application/application.h"
#include "application/requested_application_control.h"

namespace extension {
namespace application {

class ApplicationInstance: public common::ParsedInstance {
 public:
  ApplicationInstance(const std::string& app_id);
  virtual ~ApplicationInstance();
 private:
  void GetCurrentApplication(const picojson::value& args, picojson::object& out);
  void GetAppContext(const picojson::value& args, picojson::object& out);
  void GetAppInfo(const picojson::value& args, picojson::object& out);
  void GetAppCerts(const picojson::value& args, picojson::object& out);
  void GetAppSharedURI(const picojson::value& args, picojson::object& out);
  void GetAppMetaData(const picojson::value& args, picojson::object& out);
  void AddAppInfoEventListener(const picojson::value& args, picojson::object& out);
  void RemoveAppInfoEventListener(const picojson::value& args, picojson::object& out);
  void GetRequestedAppControl(const picojson::value& args, picojson::object& out);
  void ReplyResult(const picojson::value& args, picojson::object& out);
  void ReplyFailure(const picojson::value& args, picojson::object& out);
  void GetSize(const picojson::value& args, picojson::object& out);
  void Kill(const picojson::value& args, picojson::object& out);
  void Launch(const picojson::value& args, picojson::object& out);
  void LaunchAppControl(const picojson::value& args, picojson::object& out);
  void FindAppControl(const picojson::value& args, picojson::object& out);
  void GetAppsContext(const picojson::value& args, picojson::object& out);
  void GetAppsInfo(const picojson::value& args, picojson::object& out);

  ApplicationManager manager_;
  Application current_application_;
  std::string app_id_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_INSTANCE_H_
