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

#ifndef __TIZEN_CONVERSATIONS_CHANGE_CALLBACK_H__
#define __TIZEN_CONVERSATIONS_CHANGE_CALLBACK_H__

#include <JavaScriptCore/JavaScript.h>

#include <MultiCallbackUserData.h>

#include <AbstractFilter.h>

#include "MessageConversation.h"

namespace DeviceAPI {
namespace Messaging {

extern const char* CONVERSATIONSADDED;
extern const char* CONVERSATIONSUPDATED;
extern const char* CONVERSATIONSREMOVED;

class ConversationsChangeCallback {
public:
    ConversationsChangeCallback(JSContextRef globalCtx,
            JSObjectRef on_added_obj,
            JSObjectRef on_updated_obj,
            JSObjectRef on_removed_obj,
            int service_id,
            MessageType service_type);
    virtual ~ConversationsChangeCallback();

    void added(const ConversationPtrVector& conversations);
    void updated(const ConversationPtrVector& conversations);
    void removed(const ConversationPtrVector& conversations);

    void setFilter(DeviceAPI::Tizen::AbstractFilterPtr filter);
    DeviceAPI::Tizen::AbstractFilterPtr getFilter() const;

    int getServiceId() const;
    MessageType getServiceType() const;

    void setActive(bool act);
    bool isActive();

    void setItems(ConversationPtrVector& items);
    ConversationPtrVector getItems();
    JSContextRef getContext() const;
private:
    static ConversationPtrVector filterConversations(
            DeviceAPI::Tizen::AbstractFilterPtr a_filter,
            const ConversationPtrVector& a_sourceConversations);

    Common::MultiCallbackUserData m_callback_data;
    DeviceAPI::Tizen::AbstractFilterPtr m_filter;
    int m_id;
    MessageType m_msg_type;
    bool m_is_act;
    ConversationPtrVector m_items;

};

} // Messaging
} // DeviceAPI




#endif // __TIZEN_CONVERSATIONS_CHANGE_CALLBACK_H__
