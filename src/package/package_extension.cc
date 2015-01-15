// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "package/package_extension.h"

#include "package/package_instance.h"
#include "common/logger.h"

// This will be generated from package_api.js
extern const char kSource_package_api[];

common::Extension* CreateExtension() {
  LoggerD("Enter");
  return new PackageExtension;
}

PackageExtension::PackageExtension() {
  LoggerD("Enter");
  SetExtensionName("tizen.package");
  SetJavaScriptAPI(kSource_package_api);
}

PackageExtension::~PackageExtension() {
  LoggerD("Enter");
}

common::Instance* PackageExtension::CreateInstance() {
  LoggerD("Enter");
  return new extension::package::PackageInstance;
}