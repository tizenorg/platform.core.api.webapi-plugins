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

/**
 * @file: FindMsgCallbackUserData.cpp
 */

#include "FindMsgCallbackUserData.h"

namespace DeviceAPI {
namespace Messaging {

FindMsgCallbackUserData::FindMsgCallbackUserData(JSContextRef globalCtx):
        CallbackUserData(globalCtx),
        m_limit(0),
        m_offset(0),
        m_is_error(false),
        m_account_id(0),
        m_service_type(UNDEFINED)
{
}

FindMsgCallbackUserData::~FindMsgCallbackUserData()
{
}

void FindMsgCallbackUserData::setFilter(Tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

void FindMsgCallbackUserData::setSortMode(Tizen::SortModePtr sortMode)
{
    m_sort = sortMode;
}

void FindMsgCallbackUserData::setLimit(long limit)
{
    m_limit = limit;
}

void FindMsgCallbackUserData::setOffset(long offset)
{
    m_offset = offset;
}

void FindMsgCallbackUserData::addMessage(std::shared_ptr<Message> msg)
{
    m_messages.push_back(msg);
}

std::vector<std::shared_ptr<Message>> FindMsgCallbackUserData::getMessages() const
{
    return m_messages;
}

void FindMsgCallbackUserData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

bool FindMsgCallbackUserData::isError() const
{
    return m_is_error;
}

std::string FindMsgCallbackUserData::getErrorName() const
{
    return m_err_name;
}

std::string FindMsgCallbackUserData::getErrorMessage() const
{
    return m_err_message;
}

void FindMsgCallbackUserData::setAccountId(int account_id){
    m_account_id = account_id;
}

int FindMsgCallbackUserData::getAccountId() const
{
    return m_account_id;
}

void FindMsgCallbackUserData::setMessageServiceType(MessageType m_msg_type)
{
    m_service_type = m_msg_type;
}

MessageType FindMsgCallbackUserData::getMessageServiceType() const
{
    return m_service_type;
}

Tizen::AbstractFilterPtr FindMsgCallbackUserData::getFilter() const
{
    return m_filter;
}

Tizen::SortModePtr FindMsgCallbackUserData::getSortMode() const
{
    return m_sort;
}

long FindMsgCallbackUserData::getLimit() const
{
    return m_limit;
}

long FindMsgCallbackUserData::getOffset() const
{
    return m_offset;
}
}//Messaging
}//DeviceAPI
