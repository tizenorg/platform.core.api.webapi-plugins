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
 
#ifndef MESSAGING_MESSAGES_CALLBACK_USER_DATA_H_
#define MESSAGING_MESSAGES_CALLBACK_USER_DATA_H_

#include "common/callback_user_data.h"
#include "common/platform_result.h"

#include <memory>
#include <string>

#include "messaging_util.h"

namespace extension {
namespace messaging {

class Message;

class MessagesCallbackUserData: public common::CallbackUserData {
public:
    MessagesCallbackUserData(PostQueue& queue);
    MessagesCallbackUserData(long cid, PostQueue& queue, bool keep = false);
    virtual ~MessagesCallbackUserData();

    void addMessage(std::shared_ptr<Message> msg);
    std::vector<std::shared_ptr<Message>> getMessages() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    void SetError(const common::PlatformResult& error);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void setMessageServiceType(MessageType m_msg_type);
    MessageType getMessageServiceType() const;

    PostQueue& getQueue() { return queue_;};

private:
    std::vector<std::shared_ptr<Message>> m_messages;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    MessageType m_service_type;
    PostQueue& queue_;
};

}//messaging
}//extension

#endif /* MESSAGING_MESSAGES_CALLBACK_USER_DATA_H_ */
