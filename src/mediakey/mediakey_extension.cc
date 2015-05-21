// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediakey/mediakey_extension.h"
#include "mediakey/mediakey_instance.h"
#include "mediakey/mediakey_manager.h"

// This will be generated from mediakey_api.js.
extern const char kSource_mediakey_api[];

namespace extension {
namespace mediakey {

MediaKeyExtension::MediaKeyExtension() {
  SetExtensionName("tizen.mediakey");
  SetJavaScriptAPI(kSource_mediakey_api);
}

MediaKeyExtension::~MediaKeyExtension() {
}

common::Instance* MediaKeyExtension::CreateInstance() {
  return new MediaKeyInstance();
}

}  // namespace mediakey
}  // namespace extension

// entry point
common::Extension* CreateExtension() {
  return new extension::mediakey::MediaKeyExtension();
}

