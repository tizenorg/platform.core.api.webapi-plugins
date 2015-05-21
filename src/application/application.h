// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_APPLICATION_APPLICATION_H_
#define SRC_APPLICATION_APPLICATION_H_

#include <string>
#include <memory>

#include "common/picojson.h"
#include "application/requested_application_control.h"

namespace extension {
namespace application {

class Application {
 public:
  RequestedApplicationControl& app_control();
  void GetRequestedAppControl(const picojson::value& args, picojson::object* out);

 private:
  RequestedApplicationControl app_control_;
};

}  // namespace application
}  // namespace extension

#endif  // SRC_APPLICATION_APPLICATION_H_
