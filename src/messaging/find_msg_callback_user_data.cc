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
/**
 * @file: FindMsgCallbackUserData.cpp
 */

#include "find_msg_callback_user_data.h"
#include "common/picojson.h"

namespace extension {
namespace messaging {

FindMsgCallbackUserData::FindMsgCallbackUserData(PostQueue& queue, long cid) :
    CallbackUserData(queue, cid),
    m_limit(0),
    m_offset(0),
    m_account_id(0),
    m_service_type(UNDEFINED) {
  LoggerD("Entered");
}

FindMsgCallbackUserData::~FindMsgCallbackUserData()
{
    LoggerD("Entered");
}

void FindMsgCallbackUserData::setFilter(AbstractFilterPtr filter)
{
    m_filter = filter;
}

void FindMsgCallbackUserData::setSortMode(SortModePtr sortMode)
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

AbstractFilterPtr FindMsgCallbackUserData::getFilter() const
{
    return m_filter;
}

SortModePtr FindMsgCallbackUserData::getSortMode() const
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
