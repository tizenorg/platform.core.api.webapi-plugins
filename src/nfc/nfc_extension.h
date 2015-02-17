// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_EXTENSION_H_
#define NFC_NFC_EXTENSION_H_

#include "common/extension.h"

class NFCExtension : public common::Extension {
 public:
  NFCExtension();
  virtual ~NFCExtension();
 private:
  // common::Extension implementation.
  virtual common::Instance* CreateInstance();
};

#endif  // NFC_NFC_EXTENSION_H_
