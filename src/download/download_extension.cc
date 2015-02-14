// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "download/download_extension.h"

#include "download/download_instance.h"

// This will be generated from download_api.js
extern const char kSource_download_api[];

common::Extension* CreateExtension() {
  return new DownloadExtension;
}

DownloadExtension::DownloadExtension() {
  SetExtensionName("tizen.download");
  SetJavaScriptAPI(kSource_download_api);

  const char* entry_points[] = {
    "tizen.DownloadRequest",
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

DownloadExtension::~DownloadExtension() {}

common::Instance* DownloadExtension::CreateInstance() {
  return new extension::download::DownloadInstance;
}
