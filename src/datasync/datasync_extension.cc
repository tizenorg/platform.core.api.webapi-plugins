// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// File copied from Crosswalk

#include "datasync/datasync_extension.h"
#include "datasync/datasync_instance.h"

// This will be generated from datasync_api.js.
extern const char kSource_datasync_api[];

using namespace extension::datasync;

DatasyncExtension::DatasyncExtension() {
  SetExtensionName("tizen.datasync");
  SetJavaScriptAPI(kSource_datasync_api);

  const char* entry_points[] = {"tizen.SyncInfo", "tizen.SyncProfileInfo",
                                "tizen.SyncServiceInfo", NULL};
  SetExtraJSEntryPoints(entry_points);
}

DatasyncExtension::~DatasyncExtension() {}

DataSyncManager& DatasyncExtension::manager() {
  // Initialize API on first request
  return DataSyncManager::Instance();
}

common::Instance* DatasyncExtension::CreateInstance() {
  return new DatasyncInstance;
}

// entry point
common::Extension* CreateExtension() { return new DatasyncExtension; }
