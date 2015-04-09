// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_extension.h"

#include "mediacontroller/mediacontroller_instance.h"

// This will be generated from mediacontroller_api.js
extern const char kSource_mediacontroller_api[];

common::Extension* CreateExtension() {
  return new MediacontrollerExtension;
}

MediacontrollerExtension::MediacontrollerExtension() {
  SetExtensionName("tizen.mediaController");
  SetJavaScriptAPI(kSource_mediacontroller_api);

  const char* entry_points[] = {
      NULL
    };
  SetExtraJSEntryPoints(entry_points);
}

MediacontrollerExtension::~MediacontrollerExtension() {}

common::Instance* MediacontrollerExtension::CreateInstance() {
  return new extension::mediacontroller::MediacontrollerInstance;
}