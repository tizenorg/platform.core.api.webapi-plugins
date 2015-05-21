// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTACT_CONTACT_EXTENSION_H
#define CONTACT_CONTACT_EXTENSION_H

#include "common/extension.h"

class ContactExtension : public common::Extension {
 public:
  ContactExtension();
  virtual ~ContactExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // CONTACT_CONTACT_EXTENSION_H
