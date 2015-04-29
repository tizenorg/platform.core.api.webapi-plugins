// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "websetting/websetting_extension.h"

extern const char kSource_websetting_api[];

common::Extension* CreateExtension() {
  return new WebSettingExtension();
}

WebSettingExtension::WebSettingExtension() {
  SetExtensionName("tizen.websetting");
  SetJavaScriptAPI(kSource_websetting_api);
}

WebSettingExtension::~WebSettingExtension() {}

common::Instance* WebSettingExtension::CreateInstance() {
  return new common::ParsedInstance();
}
