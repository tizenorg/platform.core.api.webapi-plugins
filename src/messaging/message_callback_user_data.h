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
 
#ifndef MESSAGING_MESSAGE_CALLBACK_USER_DATA_H_
#define MESSAGING_MESSAGE_CALLBACK_USER_DATA_H_

#include "common/callback_user_data.h"
#include "common/platform_result.h"

#include <memory>
#include <string>

namespace extension {
namespace messaging {

class Message;
class PostQueue;

class MessageCallbackUserData: public common::CallbackUserData {
public:
    MessageCallbackUserData(PostQueue& queue);
    virtual ~MessageCallbackUserData();

    void setMessage(std::shared_ptr<Message> message);
    std::shared_ptr<Message> getMessage() const;

    void setAccountId(int account_id);
    int getAccountId() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    void setError(const common::PlatformResult& error);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    PostQueue& getQueue() { return queue_;};

private:
    bool m_is_error;
    PostQueue& queue_;
    std::string m_err_name;
    std::string m_err_message;
    std::shared_ptr<Message> m_message;
    int m_account_id;

};

}//messaging
}//extension

#endif /* MESSAGING_MESSAGE_CALLBACK_USER_DATA_H_ */

