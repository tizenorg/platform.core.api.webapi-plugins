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
 
#include "message_callback_user_data.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

MessageCallbackUserData::MessageCallbackUserData(PostQueue& queue, long cid) :
    CallbackUserData(queue, cid),
    m_account_id(0) {
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

}//messaging
}//extension
