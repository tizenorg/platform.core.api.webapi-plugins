// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "common/platform_exception.h"
#include "common/logger.h"

#include "message_sms.h"

namespace extension {
namespace messaging {

MessageSMS::MessageSMS():
    Message()
{
    LoggerD("MessageSMS constructor.");
    this->m_type = MessageType(MessageType(SMS));
}

MessageSMS::~MessageSMS()
{
    LoggerD("MessageSMS destructor.");
}

} // messaging
} // extension
