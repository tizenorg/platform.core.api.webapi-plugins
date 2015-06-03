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

//#include <JSWebAPIErrorFactory.h>
//#include <PlatformException.h>
//#include <JSUtil.h>
//#include <GlobalContextManager.h>

//#include "MessagingUtil.h"
//#include "ConversationsChangeCallback.h"
//#include "JSMessageConversation.h"
//#include "MessagingUtil.h"

#include "common/logger.h"
#include "common/platform_exception.h"
#include "messaging_instance.h"
#include "messaging_util.h"

#include "conversations_change_callback.h"

namespace extension {
namespace messaging {


const char* CONVERSATIONSADDED = "conversationsadded";
const char* CONVERSATIONSUPDATED = "conversationsupdated";
const char* CONVERSATIONSREMOVED = "conversationsremoved";

ConversationsChangeCallback::ConversationsChangeCallback(
        long cid,
        int service_id,
        MessageType service_type,
        PostQueue& queue) :
        m_callback_data(cid, queue, true),
        m_id(service_id),
        m_msg_type(service_type),
        m_is_act(true)
{
    LoggerD("Entered");
}

ConversationsChangeCallback::~ConversationsChangeCallback()
{
    LoggerD("Entered");
}

ConversationPtrVector ConversationsChangeCallback::filterConversations(
        AbstractFilterPtr filter,
        const ConversationPtrVector& source_conversations)
{
    LoggerD("Entered");
    if (filter) {
        LoggerD("filter pointer is valid");
        ConversationPtrVector filtered_conversations;
        ConversationPtrVector::const_iterator it = source_conversations.begin();
        ConversationPtrVector::const_iterator end_it = source_conversations.end();

        for(int i = 0; it != end_it; ++i, ++it) {
            const ConversationPtr& conversation = *it;
            const bool matched = filter->isMatching(conversation.get());
            if(matched) {
                filtered_conversations.push_back(conversation);
            }

            LoggerD("[%d] conversation id:%d", i, conversation->getConversationId());
            LoggerD("[%d] conversation subject :%s", i, conversation->getSubject().c_str());
            LoggerD("[%d] matched filter: %s", i, matched ? "YES" : "NO");
        }

        LoggerD("returning matching %d of %d conversations", filtered_conversations.size(),
                source_conversations.size());

        return filtered_conversations;
    }
    else {
        LoggerD("filter pointer is not valid");
        return source_conversations;
    }
}

void ConversationsChangeCallback::added(
        const ConversationPtrVector& conversations)
{
    LoggerD("Entered conversations.size()=%d", conversations.size());
    if (!m_is_act) {
        return;
    }

    ConversationPtrVector filtered = filterConversations(m_filter, conversations);

    picojson::array array;
    auto each = [&array] (std::shared_ptr<MessageConversation> c)->void {
        array.push_back(MessagingUtil::conversationToJson(c));
    };
    for_each(filtered.begin(), filtered.end(), each);

    LoggerD("Calling:%s with:%d added conversations", CONVERSATIONSADDED,
        filtered.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(CONVERSATIONSADDED);
    obj[JSON_DATA] = picojson::value(array);

    m_callback_data.getQueue().addAndResolve(obj.at(
                JSON_CALLBACK_ID).get<double>(), PostPriority::MEDIUM, json->serialize());
}

void ConversationsChangeCallback::updated(
        const ConversationPtrVector& conversations)
{
    LoggerD("Entered conversations.size()=%d", conversations.size());
    if (!m_is_act) {
        return;
    }

    ConversationPtrVector filtered = filterConversations(m_filter, conversations);

    picojson::array array;
    auto each = [&array] (std::shared_ptr<MessageConversation> c)->void {
        array.push_back(MessagingUtil::conversationToJson(c));
    };
    for_each(filtered.begin(), filtered.end(), each);

    LoggerD("Calling:%s with:%d added conversations", CONVERSATIONSUPDATED,
        filtered.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(CONVERSATIONSUPDATED);
    obj[JSON_DATA] = picojson::value(array);

    m_callback_data.getQueue().addAndResolve(obj.at(
                JSON_CALLBACK_ID).get<double>(), PostPriority::LOW, json->serialize());
}

void ConversationsChangeCallback::removed(
        const ConversationPtrVector& conversations)
{
    LoggerD("Entered conversations.size()=%d", conversations.size());
    if (!m_is_act) {
        return;
    }

    ConversationPtrVector filtered = filterConversations(m_filter, conversations);

    picojson::array array;
    auto each = [&array] (std::shared_ptr<MessageConversation> c)->void {
        array.push_back(MessagingUtil::conversationToJson(c));
    };
    for_each(filtered.begin(), filtered.end(), each);

    LoggerD("Calling:%s with:%d added conversations", CONVERSATIONSREMOVED,
        filtered.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(CONVERSATIONSREMOVED);
    obj[JSON_DATA] = picojson::value(array);

    m_callback_data.getQueue().addAndResolve(obj.at(
                JSON_CALLBACK_ID).get<double>(), PostPriority::LAST, json->serialize());
}

void ConversationsChangeCallback::setFilter(tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

tizen::AbstractFilterPtr ConversationsChangeCallback::getFilter() const
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

} //namespace messaging
} //namespace extension
