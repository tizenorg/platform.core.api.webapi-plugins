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

#include "ConversationCallbackData.h"
#include "JSMessageConversation.h"

namespace DeviceAPI {
namespace Messaging {

ConversationCallbackData::ConversationCallbackData(JSContextRef globalCtx):
        CallbackUserData(globalCtx),
        m_filter(),
        m_sort(),
        m_limit(0),
        m_offset(0),
        m_is_error(false),
        m_account_id(0),
        m_service_type(UNDEFINED)
{
}

ConversationCallbackData::~ConversationCallbackData()
{
}

void ConversationCallbackData::setFilter(Tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

void ConversationCallbackData::setSortMode(Tizen::SortModePtr sortMode)
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

void ConversationCallbackData::addConversations(JSContextRef context,
        const std::vector<JSValueRef>& jsobject_conservations)
{
    const size_t new_conversations_count = jsobject_conservations.size();
    if(0 == new_conversations_count) {
        return;
    }

    m_conversations.reserve(m_conversations.size() + new_conversations_count);
    for (auto it = jsobject_conservations.begin();
            it != jsobject_conservations.end();
            ++it) {
        m_conversations.push_back(JSMessageConversation::getPrivateObject(context, *it));
    }
}

std::vector<std::shared_ptr<MessageConversation>> ConversationCallbackData::getConversations() const
{
    return m_conversations;
}

void ConversationCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

bool ConversationCallbackData::isError() const
{
    return m_is_error;
}

std::string ConversationCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string ConversationCallbackData::getErrorMessage() const
{
    return m_err_message;
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

Tizen::AbstractFilterPtr ConversationCallbackData::getFilter() const
{
    return m_filter;
}

Tizen::SortModePtr ConversationCallbackData::getSortMode() const
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
}//Messaging
}//DeviceAPI
