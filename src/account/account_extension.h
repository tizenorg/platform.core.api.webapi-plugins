// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCOUNT_ACCOUNT_EXTENSION_H_
#define ACCOUNT_ACCOUNT_EXTENSION_H_

#include "common/extension.h"

class AccountExtension : public common::Extension {
 public:
  AccountExtension();
  virtual ~AccountExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // ACCOUNT_ACCOUNT_EXTENSION_H_
