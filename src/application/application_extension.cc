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

#include "application/application_extension.h"

#include <iostream>
#include <sstream>

#include "application/application_instance.h"
#include "common/logger.h"

namespace {
const char* kApplication = "tizen.application";
const char* kApplicationControl = "tizen.ApplicationControl";
const char* kApplicationControlData = "tizen.ApplicationControlData";
}

// This will be generated from application_api.js
extern const char kSource_application_api[];

common::Extension* CreateExtension() {
  ApplicationExtension* e = new ApplicationExtension();

  if (e->app_id().empty()) {
    LoggerD("Application extension will not be created.");
    delete e;
    return nullptr;
  }

  return e;
}

ApplicationExtension::ApplicationExtension() {
  app_id_ = GetRuntimeVariable("app_id", 64);

  LoggerD("app_id: %s", app_id_.c_str());

  SetExtensionName(kApplication);
  SetJavaScriptAPI(kSource_application_api);

  const char* entry_points[] = {
      kApplicationControl,
      kApplicationControlData,
      NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

ApplicationExtension::~ApplicationExtension() {}

common::Instance* ApplicationExtension::CreateInstance() {
  return new extension::application::ApplicationInstance(app_id_);
}
