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

#include <JSWebAPIErrorFactory.h>
#include <PlatformException.h>
#include <JSUtil.h>
#include <Logger.h>

#include "MessagesChangeCallback.h"
#include "JSMessage.h"
#include "AbstractFilter.h"
#include "MessagingUtil.h"

using namespace DeviceAPI::Common;
using namespace DeviceAPI::Tizen;

namespace DeviceAPI {
namespace Messaging {

namespace {

std::string limitedString(const std::string& src,
                          const size_t max_len = 40)
{
    if(src.length() > max_len) {
        return src.substr(0,max_len);
    } else {
        return src;
    }
}

} //Anonymous namespace

const char* MESSAGESADDED = "messagesadded";
const char* MESSAGESUPDATED = "messagesupdated";
const char* MESSAGESREMOVED = "messagesremoved";

MessagesChangeCallback::MessagesChangeCallback(JSContextRef global_ctx,
        JSObjectRef on_added_obj,
        JSObjectRef on_updated_obj,
        JSObjectRef on_removed_obj,
        int service_id,
        MessageType service_type) :
        m_callback_data(global_ctx),
        m_service_id(service_id),
        m_msg_type(service_type),
        m_is_act(true)
{
    LOGD("Entered");

    m_callback_data.setCallback(MESSAGESADDED, on_added_obj);
    m_callback_data.setCallback(MESSAGESUPDATED, on_updated_obj);
    m_callback_data.setCallback(MESSAGESREMOVED, on_removed_obj);
}

MessagesChangeCallback::~MessagesChangeCallback()
{
    LOGD("Entered");
}

MessagePtrVector MessagesChangeCallback::filterMessages(
        AbstractFilterPtr filter,
        const MessagePtrVector& source_messages,
        const int service_id)
{
    LOGD("Entered sourceMessages.size():%d filter:%s", source_messages.size(),
            (filter ? "PRESENT" : "NULL"));

    if (filter) {
        MessagePtrVector filtered_messages;
        MessagePtrVector::const_iterator it = source_messages.begin();
        MessagePtrVector::const_iterator end_it = source_messages.end();

        for(int i = 0; it != end_it ; ++i, ++it) {
            const MessagePtr& message = *it;
            message->setServiceId(service_id);

            const bool matched = filter->isMatching(message.get());
            if(matched) {
                filtered_messages.push_back(message);
            }

            LOGD("[%d] is Message(%p) {", i, message.get());
            LOGD("[%d] messageId: %d", i, message->getId());
            LOGD("[%d] message subject: %s", i, message->getSubject().c_str());
            LOGD("[%d] from: %s", i, message->getFrom().c_str());

            if(message->getBody()) {
                const std::string& pBody = message->getBody()->getPlainBody();
                LOGD("[%d] message plainBody: %s", i, limitedString(pBody).c_str());
            }

            LOGD("[%d] matched filter: %s", i, matched ? "YES" : "NO");
            LOGD("[%d] }");
        }

        LOGD("returning matching %d of %d messages", filtered_messages.size(),
                source_messages.size());
        return filtered_messages;
    }
    else {
        return source_messages;
    }
}

void MessagesChangeCallback::added(const MessagePtrVector& msgs)
{
    LOGD("Entered num messages: %d", msgs.size());
    if (!m_is_act) {
        return;
    }
    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    MessagePtrVector filtered_msgs = filterMessages(m_filter, msgs, m_service_id);
    //cancel callback only if filter did remove all messages
    //if callback was called with empty msgs list, call it
    if (msgs.size() > 0 && filtered_msgs.size() == 0) {
        LOGD("All messages were filtered out, not calling callback");
        return;
    }
    JSObjectRef js_obj = JSMessage::messageVectorToJSObjectArray(
            ctx, filtered_msgs);

    LOGD("Calling:%s with:%d added messages", MESSAGESADDED,
        filtered_msgs.size());
    m_callback_data.invokeCallback(MESSAGESADDED, js_obj);
}

void MessagesChangeCallback::updated(const MessagePtrVector& msgs)
{
    LOGD("Entered num messages: %d", msgs.size());
    if (!m_is_act) {
        return;
    }
    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    MessagePtrVector filtered_msgs = filterMessages(m_filter, msgs, m_service_id);
    //cancel callback only if filter did remove all messages
    //if callback was called with empty msgs list, call it
    if (msgs.size() > 0 && filtered_msgs.size() == 0) {
        LOGD("All messages were filtered out, not calling callback");
        return;
    }
    JSObjectRef js_obj = JSMessage::messageVectorToJSObjectArray(
            ctx, filtered_msgs);

    LOGD("Calling:%s with:%d updated messages", MESSAGESUPDATED,
        filtered_msgs.size());
    m_callback_data.invokeCallback(MESSAGESUPDATED, js_obj);
}

void MessagesChangeCallback::removed(const MessagePtrVector& msgs)
{
    LOGD("Enter event: msgs.size() = %d", msgs.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    MessagePtrVector filtered_msgs = filterMessages(m_filter, msgs, m_service_id);
    //cancel callback only if filter did remove all messages
    //if callback was called with empty msgs list, call it
    if (msgs.size() > 0 && filtered_msgs.size() == 0) {
        LOGD("All messages were filtered out, not calling callback");
        return;
    }
    JSObjectRef js_obj = JSMessage::messageVectorToJSObjectArray(
            ctx, filtered_msgs);

    LOGD("Calling:%s with:%d removed messages", MESSAGESREMOVED,
        filtered_msgs.size());
    m_callback_data.invokeCallback(MESSAGESREMOVED, js_obj);
}

void MessagesChangeCallback::setFilter(AbstractFilterPtr filter)
{
    m_filter = filter;
}

AbstractFilterPtr MessagesChangeCallback::getFilter() const
{
    return m_filter;
}

int MessagesChangeCallback::getServiceId() const
{
    return m_service_id;
}

MessageType MessagesChangeCallback::getServiceType() const
{
    return m_msg_type;
}

void MessagesChangeCallback::setActive(bool act) {
    m_is_act = act;
}

bool MessagesChangeCallback::isActive() {
    return m_is_act;
}

void MessagesChangeCallback::setItems(MessagePtrVector& items)
{
    m_items = items;
}
MessagePtrVector MessagesChangeCallback::getItems()
{
    return m_items;
}

JSContextRef MessagesChangeCallback::getContext() const
{
    return m_callback_data.getContext();
}


} // Messaging
} // DeviceAPI
