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
#include <GlobalContextManager.h>

#include "MessagingUtil.h"
#include "ConversationsChangeCallback.h"
#include "JSMessageConversation.h"
#include "MessagingUtil.h"
#include <Logger.h>

using namespace DeviceAPI::Common;
using namespace DeviceAPI::Tizen;

namespace DeviceAPI {
namespace Messaging {

const char* CONVERSATIONSADDED = "conversationsadded";
const char* CONVERSATIONSUPDATED = "conversationsupdated";
const char* CONVERSATIONSREMOVED = "conversationsremoved";

ConversationsChangeCallback::ConversationsChangeCallback(
        JSContextRef global_ctx,
        JSObjectRef on_added_obj,
        JSObjectRef on_updated_obj,
        JSObjectRef on_removed_obj,
        int service_id,
        MessageType service_type) :
        m_callback_data(global_ctx),
        m_id(service_id),
        m_msg_type(service_type),
        m_is_act(true)
{
    LOGD("Entered");

    m_callback_data.setCallback(CONVERSATIONSADDED, on_added_obj);
    m_callback_data.setCallback(CONVERSATIONSUPDATED, on_updated_obj);
    m_callback_data.setCallback(CONVERSATIONSREMOVED, on_removed_obj);
}

ConversationsChangeCallback::~ConversationsChangeCallback()
{
    LOGD("Entered");
}

ConversationPtrVector ConversationsChangeCallback::filterConversations(
        AbstractFilterPtr filter,
        const ConversationPtrVector& source_conversations)
{
    if (filter) {
        ConversationPtrVector filtered_conversations;
        ConversationPtrVector::const_iterator it = source_conversations.begin();
        ConversationPtrVector::const_iterator end_it = source_conversations.end();

        for(int i = 0; it != end_it; ++i, ++it) {
            const ConversationPtr& conversation = *it;
            const bool matched = filter->isMatching(conversation.get());
            if(matched) {
                filtered_conversations.push_back(conversation);
            }

            LOGD("[%d] conversation id:%d", i, conversation->getConversationId());
            LOGD("[%d] conversation subject :%s", i, conversation->getSubject().c_str());
            LOGD("[%d] matched filter: %s", i, matched ? "YES" : "NO");
        }

        LOGD("returning matching %d of %d conversations", filtered_conversations.size(),
                source_conversations.size());

        return filtered_conversations;
    }
    else {
        return source_conversations;
    }
}

void ConversationsChangeCallback::added(
        const ConversationPtrVector& conversations)
{
    LOGD("Entered conversations.size()=%d", conversations.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    ConversationPtrVector filtered = filterConversations(m_filter, conversations);
    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<ConversationPtr,
            JSMessageConversation>(ctx, filtered);

    LOGD("Calling:%s with:%d added conversations", CONVERSATIONSADDED,
        filtered.size());

    m_callback_data.invokeCallback(CONVERSATIONSADDED, js_obj);
}

void ConversationsChangeCallback::updated(
        const ConversationPtrVector& conversations)
{
    LOGD("Entered conversations.size()=%d", conversations.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    ConversationPtrVector filtered = filterConversations(m_filter, conversations);
    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<ConversationPtr,
            JSMessageConversation>(ctx, filtered);

    LOGD("Calling:%s with:%d updated conversations", CONVERSATIONSUPDATED,
            filtered.size());

    m_callback_data.invokeCallback(CONVERSATIONSUPDATED, js_obj);
}

void ConversationsChangeCallback::removed(
        const ConversationPtrVector& conversations)
{
    LOGD("Entered conversations.size()=%d", conversations.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    ConversationPtrVector filtered = filterConversations(m_filter, conversations);
    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<ConversationPtr,
            JSMessageConversation>(ctx, filtered);

    LOGD("Calling:%s with:%d removed conversations", CONVERSATIONSREMOVED,
            filtered.size());

    m_callback_data.invokeCallback(CONVERSATIONSREMOVED, js_obj);
}

void ConversationsChangeCallback::setFilter(DeviceAPI::Tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

DeviceAPI::Tizen::AbstractFilterPtr ConversationsChangeCallback::getFilter() const
{
    return m_filter;
}

int ConversationsChangeCallback::getServiceId() const
{
    return m_id;
}

MessageType ConversationsChangeCallback::getServiceType() const
{
    return m_msg_type;
}

void ConversationsChangeCallback::setActive(bool act) {
    m_is_act = act;
}

bool ConversationsChangeCallback::isActive() {
    return m_is_act;
}

void ConversationsChangeCallback::setItems(ConversationPtrVector& items)
{
    m_items = items;
}
ConversationPtrVector ConversationsChangeCallback::getItems()
{
    return m_items;
}

JSContextRef ConversationsChangeCallback::getContext() const
{
    return m_callback_data.getContext();
}


} // Messaging
} // DeviceAPI
