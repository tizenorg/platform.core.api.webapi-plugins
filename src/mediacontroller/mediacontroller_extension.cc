// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_extension.h"

#include "mediacontroller/mediacontroller_instance.h"

// This will be generated from mediacontroller_api.js
extern const char kSource_mediacontroller_api[];

common::Extension* CreateExtension() {
  return new MediaControllerExtension;
}

MediaControllerExtension::MediaControllerExtension() {
  SetExtensionName("tizen.mediacontroller");
  SetJavaScriptAPI(kSource_mediacontroller_api);
}

MediaControllerExtension::~MediaControllerExtension() {}

common::Instance* MediaControllerExtension::CreateInstance() {
  return new extension::mediacontroller::MediaControllerInstance;
}
