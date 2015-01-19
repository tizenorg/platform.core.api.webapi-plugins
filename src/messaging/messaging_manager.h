// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_MANAGER_H_
#define MESSAGING_MESSAGING_MANAGER_H_

#include <msg.h>
#include <string>
#include <map>

#include "message_service_email.h"
#include "message_service_short_msg.h"

namespace extension {
namespace messaging {

class MsgManagerCallbackData {
public:
    std::shared_ptr<picojson::value> json;
    std::map<int, MessageService*>* services_map;
    std::pair<int, MessageService*>* sms_service;
};

class MessagingManager {
public:
    static MessagingManager& getInstance();
    void getMessageServices(const std::string& type, double callbackId);
    MessageService* getMessageService(const int id);

private:
    MessagingManager();
    MessagingManager(const MessagingManager &);
    void operator=(const MessagingManager &);
    virtual ~MessagingManager();

    msg_handle_t m_msg_handle;
    std::map<int, MessageService*> m_email_services;
    std::pair<int, MessageService*> m_sms_service;
};

} // namespace messaging
} // namespace extension

#endif // MESSAGING_MESSAGING_MANAGER_H_

