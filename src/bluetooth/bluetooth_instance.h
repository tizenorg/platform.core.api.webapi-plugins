// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLUETOOTH_BLUETOOTH_INSTANCE_H_
#define BLUETOOTH_BLUETOOTH_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace bluetooth {

class BluetoothInstance: public common::ParsedInstance {
public:
    static BluetoothInstance& GetInstance();
private:
    BluetoothInstance();
    virtual ~BluetoothInstance();
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_INSTANCE_H_
