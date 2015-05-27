// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_STORAGE_H_
#define MESSAGING_MESSAGE_STORAGE_H_

#include <memory>
#include <unordered_set>

#include "common/platform_result.h"
#include "common/logger.h"

#include "messaging_util.h"
#include "message_callback_user_data.h"
#include "messages_callback_user_data.h"
#include "conversation_callback_data.h"
#include "find_msg_callback_user_data.h"
#include "folders_callback_data.h"

namespace extension {
namespace messaging {

class MessagesChangeCallback;
class ConversationsChangeCallback;
class FoldersChangeCallback;

class MessageStorage;
typedef std::shared_ptr<MessageStorage> MessageStoragePtr;

class MessageStorage {
public:
    MessageStorage(int id, MessageType msgType);
    virtual ~MessageStorage();

    virtual int getMsgServiceId() const;
    virtual MessageType getMsgServiceType() const;
    std::string getMsgServiceTypeString() const;

    virtual void addDraftMessage(MessageCallbackUserData* callback) = 0;
    virtual void removeMessages(MessagesCallbackUserData* callback) = 0;
    virtual void updateMessages(MessagesCallbackUserData* callback) = 0;
    virtual void findMessages(FindMsgCallbackUserData* callback) = 0;
    virtual void findConversations(ConversationCallbackData* callback) = 0;
    virtual void removeConversations(ConversationCallbackData* callback) = 0;
    virtual void findFolders(FoldersCallbackData* callback) = 0;

    // Listeners registration/removal is common for all types of storage
    // and does not have to be overwritten in derived classes.
    long addMessagesChangeListener(std::shared_ptr<MessagesChangeCallback> callback);
    long addConversationsChangeListener(std::shared_ptr<ConversationsChangeCallback> callback);
    long addFoldersChangeListener(std::shared_ptr<FoldersChangeCallback> callback);
    void removeChangeListener(long watchId);

protected:
    int m_id;
    MessageType m_msg_type;
    std::unordered_set<long> registered_listeners_;
};

}    //messaging
}    //extension

#endif /* MESSAGING_MESSAGE_STORAGE_H_ */

