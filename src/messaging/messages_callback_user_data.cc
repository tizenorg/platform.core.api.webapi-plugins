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
 
#include "messages_callback_user_data.h"

namespace extension {
namespace messaging {

MessagesCallbackUserData::MessagesCallbackUserData(PostQueue& queue, long cid, bool keep /* = false*/) :
    CallbackUserData(queue, cid, keep),
    m_service_type(UNDEFINED) {
  LoggerD("Entered");
}

MessagesCallbackUserData::~MessagesCallbackUserData() {
    LoggerD("Entered");
}

void MessagesCallbackUserData::addMessage(std::shared_ptr<Message> msg)
{
    LoggerD("Entered");
    m_messages.push_back(msg);
}

std::vector<std::shared_ptr<Message>> MessagesCallbackUserData::getMessages() const
{
    return m_messages;
}

void MessagesCallbackUserData::setMessageServiceType(MessageType m_msg_type)
{
    m_service_type = m_msg_type;
}

MessageType MessagesCallbackUserData::getMessageServiceType() const
{
    return m_service_type;
}

}//messaging
}//extension
