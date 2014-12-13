// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory/callhistory_extension.h"
#include "callhistory/callhistory_instance.h"

// This will be generated from power_api.js
extern const char kSource_callhistory_api[];

common::Extension* CreateExtension() {
    return new CallHistoryExtension;
}

CallHistoryExtension::CallHistoryExtension() {
    SetExtensionName("tizen.callhistory");
    SetJavaScriptAPI(kSource_callhistory_api);
}

CallHistoryExtension::~CallHistoryExtension() {}

common::Instance* CallHistoryExtension::CreateInstance() {
    return new extension::callhistory::CallHistoryInstance;
}
