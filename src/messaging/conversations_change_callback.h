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

#ifndef __MESSAGING_CONVERSATIONS_CHANGE_CALLBACK_H__
#define __MESSAGING_CONVERSATIONS_CHANGE_CALLBACK_H__

#include "MsgCommon/AbstractFilter.h"

#include "conversation_callback_data.h"
#include "message_conversation.h"

namespace extension {
namespace messaging {

extern const char* CONVERSATIONSADDED;
extern const char* CONVERSATIONSUPDATED;
extern const char* CONVERSATIONSREMOVED;

class ConversationsChangeCallback {
public:
    ConversationsChangeCallback(
            long cid,
            int service_id,
            MessageType service_type,
            PostQueue& queue);
    virtual ~ConversationsChangeCallback();

    void added(const ConversationPtrVector& conversations);
    void updated(const ConversationPtrVector& conversations);
    void removed(const ConversationPtrVector& conversations);

    void setFilter(tizen::AbstractFilterPtr filter);
    tizen::AbstractFilterPtr getFilter() const;

    int getServiceId() const;
    MessageType getServiceType() const;

    void setActive(bool act);
    bool isActive();

    void setItems(ConversationPtrVector& items);
    ConversationPtrVector getItems();
private:
    static ConversationPtrVector filterConversations(
            tizen::AbstractFilterPtr a_filter,
            const ConversationPtrVector& a_sourceConversations);

    ConversationCallbackData m_callback_data;
    tizen::AbstractFilterPtr m_filter;
    int m_id;
    MessageType m_msg_type;
    bool m_is_act;
    ConversationPtrVector m_items;

};

} //messaging
} //extension




#endif // __MESSAGING_CONVERSATIONS_CHANGE_CALLBACK_H__
