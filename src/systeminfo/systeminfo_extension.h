// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSTEMINFO_EXTENSION_H_
#define SYSTEMINFO_EXTENSION_H_

#include "common/extension.h"

class SysteminfoExtension : public common::Extension {
 public:
  SysteminfoExtension();
  virtual ~SysteminfoExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // SYSTEMINFO_EXTENSION_H_

