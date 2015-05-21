// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TIZEN_MESSAGES_CHANGE_CALLBACK_H__
#define __TIZEN_MESSAGES_CHANGE_CALLBACK_H__

//#include <JavaScriptCore/JavaScript.h>

//#include <MultiCallbackUserData.h>

#include "MsgCommon/AbstractFilter.h"

#include "message.h"
#include "messaging_util.h"
#include "messages_callback_user_data.h"

namespace extension {
namespace messaging {

extern const char* MESSAGESADDED;
extern const char* MESSAGESUPDATED;
extern const char* MESSAGESREMOVED;

class MessagesChangeCallback {
public:
    MessagesChangeCallback(
            long cid,
            int service_id,
            MessageType service_type,
            PostQueue& queue);
    virtual ~MessagesChangeCallback();

    void added(const MessagePtrVector& messages);
    void updated(const MessagePtrVector& messages);
    void removed(const MessagePtrVector& messages);

    void setFilter(tizen::AbstractFilterPtr filter);
    tizen::AbstractFilterPtr getFilter() const;

    int getServiceId() const;
    MessageType getServiceType() const;
    static MessagePtrVector filterMessages(
            tizen::AbstractFilterPtr a_filter,
            const MessagePtrVector& a_sourceMessages,
            const int service_id);

    void setActive(bool act);
    bool isActive();

    void setItems(MessagePtrVector& items);
    MessagePtrVector getItems();
private:
    MessagesCallbackUserData m_callback_data;
    tizen::AbstractFilterPtr m_filter;
    int m_service_id;
    MessageType m_msg_type;
    bool m_is_act;
    MessagePtrVector m_items;
};

} //namespace messaging
} //namespace extension


#endif // __TIZEN_MESSAGES_CHANGE_CALLBACK_H__
