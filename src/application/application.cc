// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application.h"

#include "common/extension.h"
#include "common/logger.h"
#include "common/platform_result.h"

using namespace common;
using namespace tools;

namespace extension {
namespace application {

RequestedApplicationControl& Application::app_control() {
    return app_control_;
}

void Application::GetRequestedAppControl(const picojson::value& args, picojson::object* out) {
  LoggerD("Entered");

  const std::string& encoded_bundle =
      GetCurrentExtension()->GetRuntimeVariable("encoded_bundle", 1024);

  picojson::value result = picojson::value(picojson::object());

  if (!encoded_bundle.empty()) {
    PlatformResult ret = app_control_.set_bundle(encoded_bundle);
    if (ret.IsError()) {
      ReportError(ret, out);
      return;
    }

    app_control_.ToJson(&result.get<picojson::object>());
  } else {
    LoggerD("bundle string is empty.");
    result = picojson::value();
  }

  ReportSuccess(result, *out);
}

}  // namespace application
}  // namespace extension
