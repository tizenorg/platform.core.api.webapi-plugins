// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "websetting/websetting_extension.h"

extern const char kSource_websetting_api[];

common::Extension* CreateExtension() {
  return new WebSettingExtension();
}

WebSettingExtension::WebSettingExtension() {
  LoggerD("Entered");
  SetExtensionName("tizen.websetting");
  SetJavaScriptAPI(kSource_websetting_api);
}

WebSettingExtension::~WebSettingExtension() {
  LoggerD("Entered");
}

common::Instance* WebSettingExtension::CreateInstance() {
  LoggerD("Entered");
  return new common::ParsedInstance();
}
