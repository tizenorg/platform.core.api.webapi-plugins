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

#ifndef __TIZEN_MESSAGES_CHANGE_CALLBACK_H__
#define __TIZEN_MESSAGES_CHANGE_CALLBACK_H__

#include <JavaScriptCore/JavaScript.h>

#include <MultiCallbackUserData.h>

#include <AbstractFilter.h>

#include "Message.h"
#include "MessagingUtil.h"

namespace DeviceAPI {
namespace Messaging {

extern const char* MESSAGESADDED;
extern const char* MESSAGESUPDATED;
extern const char* MESSAGESREMOVED;

class MessagesChangeCallback {
public:
    MessagesChangeCallback(JSContextRef globalCtx,
            JSObjectRef on_added_obj,
            JSObjectRef on_updated_obj,
            JSObjectRef on_removed_obj,
            int service_id,
            MessageType service_type);
    virtual ~MessagesChangeCallback();

    void added(const MessagePtrVector& messages);
    void updated(const MessagePtrVector& messages);
    void removed(const MessagePtrVector& messages);

    void setFilter(DeviceAPI::Tizen::AbstractFilterPtr filter);
    DeviceAPI::Tizen::AbstractFilterPtr getFilter() const;

    int getServiceId() const;
    MessageType getServiceType() const;
    static MessagePtrVector filterMessages(
            DeviceAPI::Tizen::AbstractFilterPtr a_filter,
            const MessagePtrVector& a_sourceMessages,
            const int service_id);

    void setActive(bool act);
    bool isActive();

    void setItems(MessagePtrVector& items);
    MessagePtrVector getItems();
    JSContextRef getContext() const;
private:
    Common::MultiCallbackUserData m_callback_data;
    DeviceAPI::Tizen::AbstractFilterPtr m_filter;
    int m_service_id;
    MessageType m_msg_type;
    bool m_is_act;
    MessagePtrVector m_items;
};

} // Messaging
} // DeviceAPI


#endif // __TIZEN_MESSAGES_CHANGE_CALLBACK_H__
