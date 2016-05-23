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
 
#ifndef __MESSAGING_MESSAGE_STORAGE_SMS_H
#define __MESSAGING_MESSAGE_STORAGE_SMS_H

#include "message_storage.h"


namespace extension {
namespace messaging {

class MessageStorageShortMsg: public MessageStorage {
public:
    MessageStorageShortMsg(int id, MessageType msgType);
    virtual ~MessageStorageShortMsg();

    virtual void addDraftMessage(MessageCallbackUserData* callback);
    virtual void removeMessages(MessagesCallbackUserData* callback);
    virtual void updateMessages(MessagesCallbackUserData* callback);
    virtual void findMessages(FindMsgCallbackUserData* callback);
    virtual void findConversations(ConversationCallbackData* callback);
    virtual void removeConversations(ConversationCallbackData* callback);
    virtual void findFolders(FoldersCallbackData* callback);
};

} // messaging
} // extension

#endif /* __MESSAGING_MESSAGE_STORAGE_SMS_H */
