// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "exif/exif_extension.h"
#include "exif/exif_instance.h"

namespace {
const char kExtensionName[] = "tizen.exif";
const char kExifInformationEntryPoint[] = "tizen.ExifInformation";
}

common::Extension* CreateExtension() {
  return new ExifExtension;
}

// This will be generated from exif_api.js.
extern const char kSource_exif_api[];

ExifExtension::ExifExtension() {
  SetExtensionName(kExtensionName);
  SetJavaScriptAPI(kSource_exif_api);

  const char* entry_points[] = {
    kExifInformationEntryPoint,
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

ExifExtension::~ExifExtension() {}

common::Instance* ExifExtension::CreateInstance() {
  return new extension::exif::ExifInstance;
}
