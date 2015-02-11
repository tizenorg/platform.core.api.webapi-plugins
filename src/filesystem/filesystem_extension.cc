// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem/filesystem_extension.h"

#include "filesystem/filesystem_instance.h"

// This will be generated from filesystem_api.js
extern const char kSource_filesystem_api[];

common::Extension* CreateExtension() {
  return new FilesystemExtension;
}

FilesystemExtension::FilesystemExtension() {
  SetExtensionName("tizen.filesystem");
  SetJavaScriptAPI(kSource_filesystem_api);
}

FilesystemExtension::~FilesystemExtension() {}

common::Instance* FilesystemExtension::CreateInstance() {
  return new extension::filesystem::FilesystemInstance;
}
