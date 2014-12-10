// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_EXTENSION_H_
#define MESSAGING_MESSAGING_EXTENSION_H_

#include "common/extension.h"

class MessagingExtension : public common::Extension {
 public:
  MessagingExtension();
  virtual ~MessagingExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // MESSAGING_MESSAGING_EXTENSION_H_

