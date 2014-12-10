// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_INSTANCE_H_
#define MESSAGING_MESSAGING_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace messaging {

class MessagingInstance : public common::ParsedInstance {
    public:
        MessagingInstance();
        virtual ~MessagingInstance();
};

} // namespace messaging
} // namespace extension

#endif // MESSAGING_MESSAGING_INSTANCE_H_
