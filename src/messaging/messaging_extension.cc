// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "messaging_extension.h"
#include "messaging_instance.h"
#include "email_manager.h"

namespace {
  const char* kMessaging = "tizen.messaging";
  const char* kMessage = "tizen.Message";
  const char* kMessageAttachment = "tizen.MessageAttachment";
}
// This will be generated from messaging_api.js.
extern const char kSource_messaging_api[];

using namespace common;

common::Extension* CreateExtension() {
  return new MessagingExtension;
}

MessagingExtension::MessagingExtension() {
  LoggerD("Entered");
  SetExtensionName(kMessaging);
  SetJavaScriptAPI(kSource_messaging_api);
  const char* entry_points[] = {
    kMessage,
    kMessageAttachment,
    NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

MessagingExtension::~MessagingExtension() {
  LoggerD("Entered");
}

common::Instance* MessagingExtension::CreateInstance() {
  LoggerD("Entered");
  PlatformResult ret = extension::messaging::EmailManager::InitializeEmailService();
  if (ret.IsError()) {
      LoggerE("Initializing the email service failed (%s)", ret.message().c_str());
      return nullptr;
  }
  return new extension::messaging::MessagingInstance();
}
