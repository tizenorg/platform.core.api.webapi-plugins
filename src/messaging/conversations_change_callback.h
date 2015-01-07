// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
            MessageType service_type);
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
