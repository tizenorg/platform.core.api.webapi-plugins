// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "exif/exif_extension.h"
#include "exif/exif_instance.h"

common::Extension* CreateExtension() { return new ExifExtension; }

// This will be generated from exif_api.js.
extern const char kSource_exif_api[];

ExifExtension::ExifExtension() {
  SetExtensionName("tizen.exif");
  SetJavaScriptAPI(kSource_exif_api);

  const char* entry_points[] = {"tizen.ExifInformation", NULL};
  SetExtraJSEntryPoints(entry_points);
}

ExifExtension::~ExifExtension() {}

common::Instance* ExifExtension::CreateInstance() { return new extension::exif::ExifInstance; }
