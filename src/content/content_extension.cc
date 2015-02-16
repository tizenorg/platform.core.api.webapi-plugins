// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/content_extension.h"

#include "content/content_instance.h"

// This will be generated from content_api.js
extern const char kSource_content_api[];

common::Extension* CreateExtension() {
  return new ContentExtension;
}

ContentExtension::ContentExtension() {
  SetExtensionName("tizen.content");
  SetJavaScriptAPI(kSource_content_api);
}

ContentExtension::~ContentExtension() {}

common::Instance* ContentExtension::CreateInstance() {
  return new extension::content::ContentInstance;
}