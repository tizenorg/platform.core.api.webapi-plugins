// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RADIO_RADIO_EXTENSION_H_
#define RADIO_RADIO_EXTENSION_H_

#include "common/extension.h"

class RadioExtension : public common::Extension {
 public:
  RadioExtension();
  virtual ~RadioExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif //RADIO_RADIO_EXTENSION_H_

