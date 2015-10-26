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

#include "conversation_callback_data.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

ConversationCallbackData::ConversationCallbackData(PostQueue& queue, long cid, bool keep /* = false */) :
    CallbackUserData(queue, cid, keep),
    m_limit(0),
    m_offset(0),
    m_account_id(0),
    m_service_type(UNDEFINED) {
  LoggerD("Entered");
}

ConversationCallbackData::~ConversationCallbackData()
{
    LoggerD("Entered");
}

void ConversationCallbackData::setFilter(AbstractFilterPtr filter)
{
    m_filter = filter;
}

void ConversationCallbackData::setSortMode(SortModePtr sortMode)
{
    m_sort = sortMode;
}

void ConversationCallbackData::setLimit(long limit)
{
    m_limit = limit;
}

void ConversationCallbackData::setOffset(long offset)
{
    m_offset = offset;
}

void ConversationCallbackData::addConversation(std::shared_ptr<MessageConversation> msg)
{
    m_conversations.push_back(msg);
}

std::vector<std::shared_ptr<MessageConversation>> ConversationCallbackData::getConversations() const
{
    return m_conversations;
}

void ConversationCallbackData::setAccountId(int account_id){
    m_account_id = account_id;
}

int ConversationCallbackData::getAccountId() const
{
    return m_account_id;
}

void ConversationCallbackData::setMessageServiceType(MessageType m_msg_type)
{
    m_service_type = m_msg_type;
}

MessageType ConversationCallbackData::getMessageServiceType() const
{
    return m_service_type;
}

AbstractFilterPtr ConversationCallbackData::getFilter() const
{
    return m_filter;
}

SortModePtr ConversationCallbackData::getSortMode() const
{
    return m_sort;
}

long ConversationCallbackData::getLimit() const
{
    return m_limit;
}

long ConversationCallbackData::getOffset() const
{
    return m_offset;
}

}//messaging
}//extension
