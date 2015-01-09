// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

