// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KEYMANAGER_KEYMANAGER_EXTENSION_H_
#define KEYMANAGER_KEYMANAGER_EXTENSION_H_

#include "common/extension.h"

class KeyManagerExtension : public common::Extension {
 public:
  KeyManagerExtension();
  virtual ~KeyManagerExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // KEYMANAGER_KEYMANAGER_EXTENSION_H_
