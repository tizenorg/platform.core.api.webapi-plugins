// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_extension.h"
#include "tvchannel/tvchannel_instance.h"

// This will be generated from datasync_api.js.
extern const char kSource_tvchannel_api[];

namespace tvchannel {

TVChannelExtension::TVChannelExtension() {
  SetExtensionName("tizen.tvchannel");
  SetJavaScriptAPI(kSource_tvchannel_api);
}

TVChannelExtension::~TVChannelExtension() {}

TVChannelManager& TVChannelExtension::manager() {
  // Initialize API on first request
  return TVChannelManager::getInstance();
}

common::Instance* TVChannelExtension::CreateInstance() {
  return new TVChannelInstance(*this);
}

}  // namespace tvchannel

// entry point
common::Extension* CreateExtension() {
  return new tvchannel::TVChannelExtension;
}
