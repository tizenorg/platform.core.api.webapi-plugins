// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGES_CALLBACK_USER_DATA_H_
#define MESSAGING_MESSAGES_CALLBACK_USER_DATA_H_

#include "common/callback_user_data.h"
#include "messaging_util.h"

#include <memory>
#include <string>

namespace extension {
namespace messaging {

class Message;

class MessagesCallbackUserData: public common::CallbackUserData {
public:
    MessagesCallbackUserData();
    virtual ~MessagesCallbackUserData();

    void addMessage(std::shared_ptr<Message> msg);
    std::vector<std::shared_ptr<Message>> getMessages() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void setMessageServiceType(MessageType m_msg_type);
    MessageType getMessageServiceType() const;

private:
    std::vector<std::shared_ptr<Message>> m_messages;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    MessageType m_service_type;
};

}//messaging
}//extension

#endif /* MESSAGING_MESSAGES_CALLBACK_USER_DATA_H_ */

