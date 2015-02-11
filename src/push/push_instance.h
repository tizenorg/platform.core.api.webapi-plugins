// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PUSH_PUSH_INSTANCE_H_
#define SRC_PUSH_PUSH_INSTANCE_H_

#include "common/extension.h"

#include "push/push_manager.h"

namespace extension {
namespace push {

class PushInstance: public common::ParsedInstance {
 public:
    PushInstance();
    virtual ~PushInstance();

 private:
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_INSTANCE_H_
