// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "websetting/websetting_extension.h"

#include <string>

#include "websetting/websetting.h"
#include "websetting/websetting_instance.h"

extern const char kSource_websetting_api[];

common::Extension* CreateExtension() {
  std::string env_app_id = common::Extension::GetRuntimeVariable("app_id", 64);
  std::string app_id = env_app_id.substr(1, env_app_id.rfind('"') - 1);
  if (app_id.empty()) {
    std::cerr << "Got invalid application ID." << std::endl;
    return nullptr;
  }
  return new WebSettingExtension(app_id);
}

WebSettingExtension::WebSettingExtension(const std::string& app_id) {
  current_app_.reset(new WebSetting(app_id));
  SetExtensionName("tizen.websetting");
  SetJavaScriptAPI(kSource_websetting_api);
}

WebSettingExtension::~WebSettingExtension() {}

common::Instance* WebSettingExtension::CreateInstance() {
  return new extension::websetting::WebSettingInstance(this);
}
