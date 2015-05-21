// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "networkbearerselection/networkbearerselection_extension.h"

#include "networkbearerselection/networkbearerselection_instance.h"

// This will be generated from networkbearerselection_api.js
extern const char kSource_networkbearerselection_api[];

common::Extension* CreateExtension() {
  return new NetworkBearerSelectionExtension;
}

NetworkBearerSelectionExtension::NetworkBearerSelectionExtension() {
  SetExtensionName("tizen.networkbearerselection");
  SetJavaScriptAPI(kSource_networkbearerselection_api);
}

NetworkBearerSelectionExtension::~NetworkBearerSelectionExtension() {}

common::Instance* NetworkBearerSelectionExtension::CreateInstance() {
  return new extension::networkbearerselection::NetworkBearerSelectionInstance;
}
