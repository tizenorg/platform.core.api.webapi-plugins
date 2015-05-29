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
 
#ifndef MESSAGING_MESSAGE_SERVICE_EMAIL_H_
#define MESSAGING_MESSAGE_SERVICE_EMAIL_H_

#include "message_service.h"

#include <unordered_set>

namespace extension {
namespace messaging {

class MessageServiceEmail : public MessageService {
public:
    MessageServiceEmail(int id, std::string name);
    virtual ~MessageServiceEmail();

    virtual common::PlatformResult sendMessage(MessageRecipientsCallbackData *callback);
    virtual common::PlatformResult loadMessageBody(MessageBodyCallbackData* callback);
    virtual common::PlatformResult loadMessageAttachment(MessageAttachmentCallbackData* callback);
    virtual common::PlatformResult sync(SyncCallbackData *callback, long* operation_id);
    virtual common::PlatformResult syncFolder(SyncFolderCallbackData *callback, long* operation_id);
    virtual common::PlatformResult stopSync(long op_id);

private:
    std::unordered_set<long> registered_callbacks_;
};

} // messaging
} // extension

#endif // MESSAGING_MESSAGE_SERVICE_EMAIL_H_
