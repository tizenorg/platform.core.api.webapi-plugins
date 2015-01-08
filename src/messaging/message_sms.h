// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __MESSAGING_MESSAGE_SMS_H__
#define __MESSAGING_MESSAGE_SMS_H__

// Header with core msg-service declarations
#include <msg.h>

#include "message.h"

namespace extension {
namespace messaging {

class MessageSMS: public Message {
public:
// constructor
    MessageSMS();
    ~MessageSMS();

};

} // messaging
} // extension

#endif // __MESSAGING_MESSAGE_SMS_H__
