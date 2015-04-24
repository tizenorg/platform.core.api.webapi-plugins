// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
