// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "websetting/websetting_extension.h"

#include <string>

#include "websetting/websetting.h"
#include "websetting/websetting_instance.h"

#include "common/logger.h"

extern const char kSource_websetting_api[];

common::Extension* CreateExtension() {
  WebSettingExtension* e = new WebSettingExtension();

  if (e->current_app()->app_id().empty()) {
    LoggerE("Got invalid application ID.");
    delete e;
    return nullptr;
  }

  return e;
}

WebSettingExtension::WebSettingExtension() {
  std::string app_id = GetRuntimeVariable("app_id", 64);

  LoggerD("app_id: %s", app_id.c_str());

  current_app_.reset(new WebSetting(app_id));
  SetExtensionName("tizen.websetting");
  SetJavaScriptAPI(kSource_websetting_api);
}

WebSettingExtension::~WebSettingExtension() {}

common::Instance* WebSettingExtension::CreateInstance() {
  return new extension::websetting::WebSettingInstance(this);
}
