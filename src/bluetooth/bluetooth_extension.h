// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLUETOOTH_BLUETOOTH_EXTENSION_H_
#define BLUETOOTH_BLUETOOTH_EXTENSION_H_

#include "common/extension.h"

class BluetoothExtension : public common::Extension {
 public:
  BluetoothExtension();
  virtual ~BluetoothExtension();

 private:
  virtual common::Instance* CreateInstance();
};

#endif // BLUETOOTH_BLUETOOTH_EXTENSION_H_

