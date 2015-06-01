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
 
#ifndef MESSAGING_MESSAGING_MANAGER_H_
#define MESSAGING_MESSAGING_MANAGER_H_

#include <msg.h>
#include <string>
#include <map>

#include "message_service_email.h"
#include "message_service_short_msg.h"

namespace extension {
namespace messaging {

class MessagingInstance;

class MsgManagerCallbackData {
public:
    explicit MsgManagerCallbackData(MessagingInstance& instance_);
    std::shared_ptr<picojson::value> json;
    std::map<int, MessageService*>* services_map;
    std::pair<int, MessageService*>* sms_service;
    std::pair<int, MessageService*>* mms_service;
    MessagingInstance& instance_;
};

class MessagingManager {
public:
  explicit MessagingManager(MessagingInstance& instance);
  MessagingManager(const MessagingManager &);
  virtual ~MessagingManager();

  void getMessageServices(const std::string& type, double callbackId);
  MessageService* getMessageService(const int id);

private:
    void operator=(const MessagingManager &);

    msg_handle_t m_msg_handle;
    std::map<int, MessageService*> m_email_services;
    std::pair<int, MessageService*> m_sms_service;
    std::pair<int, MessageService*> m_mms_service;

    MessagingInstance& instance_;
};

} // namespace messaging
} // namespace extension

#endif // MESSAGING_MESSAGING_MANAGER_H_

