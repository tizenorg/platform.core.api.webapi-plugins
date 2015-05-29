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

  const std::string& encoded_bundle = RequestedApplicationControl::GetEncodedBundle();

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
