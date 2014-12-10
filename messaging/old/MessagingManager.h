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

#ifndef __TIZEN_MESSAGING_MANAGER_H__
#define __TIZEN_MESSAGING_MANAGER_H__

#include <CallbackUserData.h>
#include <PlatformException.h>
#include <Security.h>

#include "MessagingUtil.h"
#include "MessageService.h"

namespace DeviceAPI {
namespace Messaging {

class MessageServiceCallbackData : public Common::CallbackUserData {
public:
    MessageServiceCallbackData(JSContextRef globalCtx);
    ~MessageServiceCallbackData();

    void setMessageType(MessageType msgType);
    MessageType getMessageType() const;

    void setMessageServices(const std::vector<MessageService*>& msgServices);
    const std::vector<MessageService*>& getMessageServices() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void clearServices();

private:
    MessageType m_msg_type;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    std::vector<MessageService*> m_msg_services;
};

class MessagingManager : public Common::SecurityAccessor
{
public:
    static MessagingManager& getInstance();

    void getMessageServices(MessageServiceCallbackData *callback);

private:
    MessagingManager();
    MessagingManager(const MessagingManager &);
    void operator=(const MessagingManager &);
    virtual ~MessagingManager();

    msg_handle_t m_msg_handle;

};

} // Messaging
} // DeviceAPI
#endif // __TIZEN_MESSAGING_MANAGER_H__
