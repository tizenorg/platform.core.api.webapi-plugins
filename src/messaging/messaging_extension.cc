// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messaging_extension.h"
#include "messaging_instance.h"

namespace {
  const char* kMessaging = "tizen.messaging";
  const char* kMessage = "tizen.Message";
  const char* kMessageAttachment = "tizen.MessageAttachment";
}
// This will be generated from messaging_api.js.
extern const char kSource_messaging_api[];

common::Extension* CreateExtension() {
  return new MessagingExtension;
}

MessagingExtension::MessagingExtension() {
  SetExtensionName(kMessaging);
  SetJavaScriptAPI(kSource_messaging_api);
  const char* entry_points[] = {
    kMessage,
    kMessageAttachment,
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

MessagingExtension::~MessagingExtension() {}

common::Instance* MessagingExtension::CreateInstance() {
  return new extension::messaging::MessagingInstance;
}
