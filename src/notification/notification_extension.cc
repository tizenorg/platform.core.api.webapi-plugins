// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "notification/notification_extension.h"
#include "notification/notification_instance.h"

// This will be generated from notification_api.js
extern const char kSource_notification_api[];

common::Extension* CreateExtension() { return new NotificationExtension; }

NotificationExtension::NotificationExtension() {
  SetExtensionName("tizen.notification");
  SetJavaScriptAPI(kSource_notification_api);

  const char* entry_points[] = {"tizen.StatusNotification",
                                "tizen.NotificationDetailInfo", NULL};
  SetExtraJSEntryPoints(entry_points);
}

NotificationExtension::~NotificationExtension() {}

common::Instance* NotificationExtension::CreateInstance() {
  return new extension::notification::NotificationInstance;
}
