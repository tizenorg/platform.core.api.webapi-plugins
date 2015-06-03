/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
