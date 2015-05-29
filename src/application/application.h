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
