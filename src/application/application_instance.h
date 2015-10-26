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
  ApplicationInstance();
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
  void BroadcastEvent(const picojson::value& args, picojson::object& out);
  void BroadcastTrustedEvent(const picojson::value& args, picojson::object& out);
  void AddEventListener(const picojson::value& args, picojson::object& out);
  void RemoveEventListener(const picojson::value& args, picojson::object& out);

  ApplicationManager manager_;
  Application current_application_;
  std::string app_id_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_INSTANCE_H_
