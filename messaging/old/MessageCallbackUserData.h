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

#ifndef __TIZEN_ADD_DRAFT_MESSAGE_CALLBACK_USER_DATA_H
#define __TIZEN_ADD_DRAFT_MESSAGE_CALLBACK_USER_DATA_H

#include <CallbackUserData.h>
#include <memory>
#include <string>

namespace DeviceAPI {
namespace Messaging {

class Message;

class MessageCallbackUserData: public Common::CallbackUserData {
public:
    MessageCallbackUserData(JSContextRef globalCtx);
    virtual ~MessageCallbackUserData();

    void setMessage(std::shared_ptr<Message> message);
    std::shared_ptr<Message> getMessage() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void setAccountId(int account_id);
    int getAccountId() const;
private:
    std::shared_ptr<Message> m_message;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    int m_account_id;
};

}//Messaging
}//DeviceAPI

#endif /* __TIZEN_ADD_DRAFT_MESSAGE_CALLBACK_USER_DATA_H */

