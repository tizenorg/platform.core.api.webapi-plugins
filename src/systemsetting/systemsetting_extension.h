// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYSTEMSETTING_SYSTEMSETTING_EXTENSION_H_
#define SYSTEMSETTING_SYSTEMSETTING_EXTENSION_H_

#include "common/extension.h"

class SystemSettingExtension : public common::Extension {
 public:
  SystemSettingExtension();
  virtual ~SystemSettingExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // SYSTEMSETTING_SYSTEMSETTING_EXTENSION_H_

