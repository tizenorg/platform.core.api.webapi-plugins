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

MessagesCallbackUserData::MessagesCallbackUserData(PostQueue& queue):
        common::CallbackUserData(),
        m_is_error(false),
        m_service_type(MessageType::UNDEFINED),
        queue_(queue)
{
    LoggerD("Entered");
}

MessagesCallbackUserData::MessagesCallbackUserData(long cid, PostQueue& queue, bool keep):
        common::CallbackUserData(),
        m_is_error(false),
        m_service_type(MessageType::UNDEFINED),
        queue_(queue)
{
    LoggerD("Entered");
    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& o = json->get<picojson::object>();
    o[JSON_CALLBACK_ID] = picojson::value(static_cast<double>(cid));
    o[JSON_CALLBACK_KEEP] = picojson::value(keep);
    setJson(json);
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

void MessagesCallbackUserData::setError(const std::string& err_name,
        const std::string& err_message)
{
    LoggerD("Entered");
    // keep only first error in chain
    if (!m_is_error) {
        LoggerD("Error has not been set yet");
        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
        auto objData = picojson::object();

        objData[JSON_ERROR_NAME] = picojson::value(err_name);
        objData[JSON_ERROR_MESSAGE] = picojson::value(err_message);

        obj[JSON_DATA] = picojson::value(objData);

        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

void MessagesCallbackUserData::SetError(const common::PlatformResult& error)
{
  // keep only first error in chain
  if (!m_is_error) {
    LoggerD("Error has not been set yet");
    m_is_error = true;
    picojson::object& obj = m_json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
    auto obj_data = picojson::object();
    obj_data[JSON_ERROR_CODE] = picojson::value(static_cast<double>(error.error_code()));
    obj_data[JSON_ERROR_MESSAGE] = picojson::value(error.message());
    obj[JSON_DATA] = picojson::value(obj_data);
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

}//messaging
}//extension
