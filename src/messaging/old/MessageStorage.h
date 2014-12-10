//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef __TIZEN_MESSAGE_STORAGE_H
#define __TIZEN_MESSAGE_STORAGE_H

#include "MessageCallbackUserData.h"
#include "MessagesCallbackUserData.h"
#include "FindMsgCallbackUserData.h"
#include "ConversationCallbackData.h"
#include "FoldersCallbackData.h"
#include "MessagingUtil.h"
#include <memory>
// headers for ChangeListeners support
#include "MessagesChangeCallback.h"
#include "FoldersChangeCallback.h"
#include "ConversationsChangeCallback.h"
#include <Security.h>

namespace DeviceAPI {
namespace Messaging {

class MessagesChangeCallback;

class MessageStorage : public Common::SecurityAccessor
{
public:
    MessageStorage(int id, MessageType msgType);
    virtual ~MessageStorage();

    virtual int getMsgServiceId() const;
    virtual MessageType getMsgServiceType() const;

    virtual void addDraftMessage(MessageCallbackUserData* callback) = 0;
    virtual void removeMessages(MessagesCallbackUserData* callback) = 0;
    virtual void updateMessages(MessagesCallbackUserData* callback) = 0;
    virtual void findMessages(FindMsgCallbackUserData* callback) = 0;
    virtual void findConversations(ConversationCallbackData* callback) = 0;
    virtual void removeConversations(ConversationCallbackData* callback) = 0;
    virtual void findFolders(FoldersCallbackData* callback) = 0;

    // Listeners registration/removal is common for all types of storage
    // and does not have to be overwritten in derived classes.
    long addMessagesChangeListener(
            std::shared_ptr<MessagesChangeCallback> callback);
    long addConversationsChangeListener(
            std::shared_ptr<ConversationsChangeCallback> callback);
    long addFoldersChangeListener(
            std::shared_ptr<FoldersChangeCallback> callback);
    void removeChangeListener(JSContextRef context, long watchId);

protected:
    int m_id;
    MessageType m_msg_type;
};

struct MessageStorageHolder {
    std::shared_ptr<MessageStorage> ptr;
};

}//Messaging
}//DeviceAPI

#endif /* __TIZEN_MESSAGE_STORAGE_H */

