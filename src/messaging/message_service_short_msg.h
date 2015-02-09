// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __MESSAGING_MESSAGE_SERVICE_SHORT_MSG_H__
#define __MESSAGING_MESSAGE_SERVICE_SHORT_MSG_H__

#include "message_service.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

class MessageServiceShortMsg : public MessageService {
public:
    virtual ~MessageServiceShortMsg();

    common::PlatformResult sendMessage(MessageRecipientsCallbackData *callback);

    virtual common::PlatformResult loadMessageBody(MessageBodyCallbackData *callback);

    static MessageServiceShortMsg* GetMmsMessageService();
    static MessageServiceShortMsg* GetSmsMessageService();

protected:
    MessageServiceShortMsg(int id, MessageType msgType);
};

} // namespace messaging
} // namespace extension
#endif // __MESSAGING_MESSAGE_SERVICE_SHORT_MSG_H__
