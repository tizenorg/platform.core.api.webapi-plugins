// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_callback_user_data.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

MessageCallbackUserData::MessageCallbackUserData(PostQueue& queue):
        common::CallbackUserData(),
        m_is_error(false),
        queue_(queue)
{
    LoggerD("Entered");
}

MessageCallbackUserData::~MessageCallbackUserData() {
    LoggerD("Entered");
}

void MessageCallbackUserData::setMessage(std::shared_ptr<Message> message) {
    m_message = message;
}

std::shared_ptr<Message> MessageCallbackUserData::getMessage() const {
    return m_message;
}

void MessageCallbackUserData::setAccountId(int account_id){
    m_account_id = account_id;
}

int MessageCallbackUserData::getAccountId() const
{
    return m_account_id;
}

void MessageCallbackUserData::setError(const std::string& err_name,
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

void MessageCallbackUserData::setError(const common::PlatformResult& error)
{
  LoggerD("Entered");
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

bool MessageCallbackUserData::isError() const
{
    return m_is_error;
}

std::string MessageCallbackUserData::getErrorName() const
{
    return m_err_name;
}

std::string MessageCallbackUserData::getErrorMessage() const
{
    return m_err_message;
}

}//messaging
}//extension
