// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_CALLBACK_USER_DATA_H_
#define MESSAGING_MESSAGE_CALLBACK_USER_DATA_H_

#include "common/callback_user_data.h"
#include "common/platform_result.h"

#include <memory>
#include <string>

namespace extension {
namespace messaging {

class Message;

class MessageCallbackUserData: public common::CallbackUserData {
public:
    MessageCallbackUserData();
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

private:
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    std::shared_ptr<Message> m_message;
    int m_account_id;
};

}//messaging
}//extension

#endif /* MESSAGING_MESSAGE_CALLBACK_USER_DATA_H_ */

