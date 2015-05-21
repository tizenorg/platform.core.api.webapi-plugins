// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvwindow/tvwindow_extension.h"
#include "tvwindow/tvwindow_instance.h"

// This will be generated from datasync_api.js.
extern const char kSource_tvwindow_api[];

namespace extension {
namespace tvwindow {

TVWindowExtension::TVWindowExtension() {
  SetExtensionName("tizen.tvwindow");
  SetJavaScriptAPI(kSource_tvwindow_api);
}

TVWindowExtension::~TVWindowExtension() {}

TVWindowManager& TVWindowExtension::manager() {
  // Initialize API on first request
  return TVWindowManager::getInstance();
}

common::Instance* TVWindowExtension::CreateInstance() {
  return new TVWindowInstance(*this);
}

}  // namespace tvwindow
}  // namespace extension

// entry point
common::Extension* CreateExtension() {
  return new extension::tvwindow::TVWindowExtension;
}
