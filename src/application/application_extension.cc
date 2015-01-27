// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application/application_extension.h"

#include <iostream>
#include <sstream>

#include "application/application_instance.h"
#include "common/logger.h"

// This will be generated from application_api.js
extern const char kSource_application_api[];

common::Extension* CreateExtension() {
  std::string app_id = common::Extension::GetRuntimeVariable("app_id", 64);
  LoggerD("app_id: %s", app_id.c_str());

  if (app_id.empty()) {
    LoggerD("Application extension will not be created.");
    return NULL;
  }

  return new ApplicationExtension(app_id);
}

ApplicationExtension::ApplicationExtension(const std::string& app_id) {
  app_id_ = app_id;

  SetExtensionName("tizen.application");
  SetJavaScriptAPI(kSource_application_api);

  const char* entry_points[] = {
      "tizen.ApplicationControlData", "tizen.ApplicationControl", NULL};
  SetExtraJSEntryPoints(entry_points);
}

ApplicationExtension::~ApplicationExtension() {}

common::Instance* ApplicationExtension::CreateInstance() {
  return new extension::application::ApplicationInstance(app_id_);
}
