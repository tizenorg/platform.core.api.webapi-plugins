// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATACONTROL_DATACONTROL_EXTENSION_H_
#define DATACONTROL_DATACONTROL_EXTENSION_H_

#include "common/extension.h"

class DatacontrolExtension : public common::Extension {
 public:
  DatacontrolExtension();
  virtual ~DatacontrolExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif  // DATACONTROL_DATACONTROL_EXTENSION_H_
