// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGEPORT_MESSAGEPORT_EXTENSION_H_
#define MESSAGEPORT_MESSAGEPORT_EXTENSION_H_

#include "common/extension.h"

class MessageportExtension : public common::Extension {
 public:
  MessageportExtension();
  virtual ~MessageportExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // MESSAGEPORT_MESSAGEPORT_EXTENSION_H_
