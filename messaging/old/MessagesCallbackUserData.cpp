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

#include "MessagesCallbackUserData.h"
#include "JSMessage.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {


MessagesCallbackUserData::MessagesCallbackUserData(JSContextRef globalCtx):
        CallbackUserData(globalCtx),
        m_is_error(false),
        m_service_type(MessageType::UNDEFINED)
{
}

MessagesCallbackUserData::~MessagesCallbackUserData() {
}

void MessagesCallbackUserData::addMessage(std::shared_ptr<Message> msg)
{
    m_messages.push_back(msg);
}

std::vector<std::shared_ptr<Message>> MessagesCallbackUserData::getMessages() const
{
    return m_messages;
}

void MessagesCallbackUserData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

bool MessagesCallbackUserData::isError() const
{
    return m_is_error;
}

std::string MessagesCallbackUserData::getErrorName() const
{
    return m_err_name;
}

std::string MessagesCallbackUserData::getErrorMessage() const
{
    return m_err_message;
}

void MessagesCallbackUserData::setMessageServiceType(MessageType m_msg_type)
{
    m_service_type = m_msg_type;
}

MessageType MessagesCallbackUserData::getMessageServiceType() const
{
    return m_service_type;
}

void MessagesCallbackUserData::addMessages(JSContextRef context,
            const std::vector<JSValueRef>& jsobject_messages)
{
    const size_t new_messages_count = jsobject_messages.size();
    if(0 == new_messages_count) {
        return;
    }

    m_messages.reserve(m_messages.size() + new_messages_count);
    for (auto it = jsobject_messages.begin() ; it != jsobject_messages.end(); ++it) {
        m_messages.push_back(JSMessage::getPrivateObject(context, *it));
    }
}

}//Messaging
}//DeviceAPI
