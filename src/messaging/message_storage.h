// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_STORAGE_H_
#define MESSAGING_MESSAGE_STORAGE_H_

#include <memory>

#include "common/logger.h"

#include "messaging_util.h"
#include "message_callback_user_data.h"
#include "messages_callback_user_data.h"

namespace extension {
namespace messaging {

class MessageStorage;
typedef std::shared_ptr<MessageStorage> MessageStoragePtr;

class MessageStorage {
public:
    MessageStorage(int id, MessageType msgType);
    virtual ~MessageStorage();

    virtual int getMsgServiceId() const;
    virtual MessageType getMsgServiceType() const;

    virtual void addDraftMessage(MessageCallbackUserData* callback) = 0;
    virtual void removeMessages(MessagesCallbackUserData* callback) = 0;
    virtual void updateMessages() = 0;
    virtual void findMessages() = 0;
    virtual void findConversations() = 0;
    virtual void removeConversations() = 0;
    virtual void findFolders() = 0;

    // Listeners registration/removal is common for all types of storage
    // and does not have to be overwritten in derived classes.
    long addMessagesChangeListener();
    long addConversationsChangeListener();
    long addFoldersChangeListener();
    void removeChangeListener();

protected:
    int m_id;
    MessageType m_msg_type;
};

}    //messaging
}    //extension

#endif /* MESSAGING_MESSAGE_STORAGE_H_ */

