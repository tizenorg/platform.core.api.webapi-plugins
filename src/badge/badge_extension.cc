// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "badge/badge_extension.h"
#include "badge/badge_instance.h"

// This will be generated from badge_api.js
extern const char kSource_badge_api[];

common::Extension* CreateExtension() { return new BadgeExtension; }

BadgeExtension::BadgeExtension() {
  SetExtensionName("tizen.badge");
  SetJavaScriptAPI(kSource_badge_api);
}

BadgeExtension::~BadgeExtension() {}

common::Instance* BadgeExtension::CreateInstance() {
  return &extension::badge::BadgeInstance::GetInstance();
}
