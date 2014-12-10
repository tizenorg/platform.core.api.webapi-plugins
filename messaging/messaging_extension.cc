// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messaging_extension.h"

#include "messaging_instance.h"

// This will be generated from time_api.js.
extern const char kSource_messaging_api[];

common::Extension* CreateExtension() {
  return new MessagingExtension;
}

MessagingExtension::MessagingExtension() {
  SetExtensionName("tizen.messaging");
  SetJavaScriptAPI(kSource_messaging_api);
}

MessagingExtension::~MessagingExtension() {}

common::Instance* MessagingExtension::CreateInstance() {
  return new extension::messaging::MessagingInstance;
}
