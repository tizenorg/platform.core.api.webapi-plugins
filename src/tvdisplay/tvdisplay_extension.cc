// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvdisplay/tvdisplay_extension.h"
#include "tvdisplay/tvdisplay_instance.h"

// This will be generated from tvdisplay_api.js.
extern const char kSource_tvdisplay_api[];


namespace extension {
namespace tvdisplay {

TVDisplayExtension::TVDisplayExtension() {
    SetExtensionName("tizen.tvdisplay");
    SetJavaScriptAPI(kSource_tvdisplay_api);
}

TVDisplayExtension::~TVDisplayExtension() {}

common::Instance* TVDisplayExtension::CreateInstance() {
    return new extension::tvdisplay::TVDisplayInstance();
}

}  // namespace tvdisplay
}  // namespace extension

// entry point
common::Extension* CreateExtension() {
  return new extension::tvdisplay::TVDisplayExtension();
}

