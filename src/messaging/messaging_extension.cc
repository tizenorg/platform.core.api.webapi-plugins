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
