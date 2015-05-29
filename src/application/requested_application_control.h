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
 

#ifndef SRC_APPLICATION_REQUESTED_APPLICATION_CONTROL_H_
#define SRC_APPLICATION_REQUESTED_APPLICATION_CONTROL_H_

#include <string>
#include <memory>
#include <type_traits>

#include <app_control.h>

#include "common/picojson.h"
#include "common/extension.h"
#include "common/platform_result.h"
#include "tizen/tizen.h"

namespace extension {
namespace application {

class RequestedApplicationControl {
 public:
  common::PlatformResult set_bundle(const std::string& encoded_bundle);
  void ToJson(picojson::object* out);
  void ReplyResult(const picojson::value& args, picojson::object* out);
  void ReplyFailure(picojson::object* out);

  static std::string GetEncodedBundle();

 private:
  void set_app_control(app_control_h app_control);
  common::PlatformResult VerifyCallerPresence();

  std::string bundle_;
  std::shared_ptr<std::remove_pointer<app_control_h>::type> app_control_;
  std::string caller_app_id_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_REQUESTED_APPLICATION_CONTROL_H_
