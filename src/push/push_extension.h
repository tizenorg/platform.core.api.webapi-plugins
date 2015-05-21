// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PUSH_PUSH_EXTENSION_H_
#define SRC_PUSH_PUSH_EXTENSION_H_

#include "common/extension.h"
#include "push/push_manager.h"

namespace extension {
namespace push {

class PushExtension : public common::Extension {
 public:
    PushExtension();
    virtual ~PushExtension();

    PushManager& manager();

 private:
    virtual common::Instance* CreateInstance();
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_EXTENSION_H_

