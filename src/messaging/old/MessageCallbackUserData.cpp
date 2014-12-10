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

#include "MessageCallbackUserData.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {


MessageCallbackUserData::MessageCallbackUserData(JSContextRef globalCtx):
        CallbackUserData(globalCtx),
        m_is_error(false)
{
}

MessageCallbackUserData::~MessageCallbackUserData() {
}

void MessageCallbackUserData::setMessage(std::shared_ptr<Message> message) {
    m_message = message;
}

std::shared_ptr<Message> MessageCallbackUserData::getMessage() const {
    return m_message;
}

void MessageCallbackUserData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
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

void MessageCallbackUserData::setAccountId(int account_id){
    m_account_id = account_id;
}

int MessageCallbackUserData::getAccountId() const
{
    return m_account_id;
}

}//Messaging
}//DeviceAPI
