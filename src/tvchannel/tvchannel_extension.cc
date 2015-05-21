// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_extension.h"
#include "tvchannel/tvchannel_instance.h"

// This will be generated from datasync_api.js.
extern const char kSource_tvchannel_api[];

TVChannelExtension::TVChannelExtension() {
    SetExtensionName("tizen.tvchannel");
    SetJavaScriptAPI(kSource_tvchannel_api);
}

TVChannelExtension::~TVChannelExtension() {
}

common::Instance* TVChannelExtension::CreateInstance() {
    return new extension::tvchannel::TVChannelInstance();
}

// entry point
common::Extension* CreateExtension() {
    return new TVChannelExtension();
}

