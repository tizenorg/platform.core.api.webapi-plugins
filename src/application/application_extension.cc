// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
