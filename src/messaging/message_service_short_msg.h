/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
