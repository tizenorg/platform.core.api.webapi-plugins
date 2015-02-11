// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PUSH_PUSH_MANAGER_H_
#define SRC_PUSH_PUSH_MANAGER_H_

namespace extension {
namespace push {


class PushManager {
 public:
    static PushManager& getInstance();
    virtual ~PushManager();

 private:
    PushManager();
};

}  // namespace push
}  // namespace extension

#endif  // SRC_PUSH_PUSH_MANAGER_H_

