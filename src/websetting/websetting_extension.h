// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBSETTING_WEBSETTING_EXTENSION_H_
#define WEBSETTING_WEBSETTING_EXTENSION_H_

#include "common/extension.h"

class WebSettingExtension : public common::Extension {
 public:
  WebSettingExtension();
  virtual ~WebSettingExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // WEBSETTING_WEBSETTING_EXTENSION_H_
