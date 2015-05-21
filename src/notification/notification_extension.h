// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NOTIFICATION_NOTIFICATION_EXTENSION_H_
#define NOTIFICATION_NOTIFICATION_EXTENSION_H_

#include "common/extension.h"

class NotificationExtension : public common::Extension {
 public:
  NotificationExtension();
  virtual ~NotificationExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // NOTIFICATION_NOTIFICATION_EXTENSION_H_
