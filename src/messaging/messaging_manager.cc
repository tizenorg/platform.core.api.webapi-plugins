// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messaging_manager.h"

#include <email-api.h>
#include <email-types.h>
#include <glib.h>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "common/extension.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include "messaging_instance.h"
#include "message_service_email.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

namespace{
const char* CMD_GET_MSG_SERVICE = "getMessageServices";
}

MessagingManager::MessagingManager()
{
    LoggerD("Entered");
    int ret = msg_open_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LoggerE("Cannot get message handle: %d", ret);
    }
}

MessagingManager::~MessagingManager()
{
    LoggerD("Entered");
    int ret = msg_close_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LoggerW("Cannot close message handle: %d", ret);
    }
}

MessagingManager& MessagingManager::getInstance()
{
    LoggerD("Entered");
    static MessagingManager instance;
    return instance;
}

static gboolean callbackCompleted(const std::shared_ptr<picojson::value>& response)
{
    LoggerD("Entered");
    std::cout<<response->serialize()<< std::endl;
    MessagingInstance::getInstance().PostMessage(response->serialize().c_str());
    return false;
}

static void* getMsgServicesThread(const std::shared_ptr<picojson::value>& response)
{
    LoggerD("Entered");

    picojson::object& obj = response->get<picojson::object>();
    obj[JSON_CMD] = picojson::value(CMD_GET_MSG_SERVICE);
    MessageType type = MessageType::UNDEFINED;
    try {
        type = MessagingUtil::stringToMessageType(response->get(JSON_DATA).get<std::string>());

        std::vector<MessageService*> msgServices;
        MessageService* messageService = NULL;
        email_account_t* email_accounts = NULL;
        int count = 0;
        bool isSupported = false;
        switch (type) {
        case MessageType::SMS:
            LoggerD("Currently unsupported");
            // TODO add class which will extended message_service and call message_service_short_msg
            break;
        case MessageType::MMS:
            LoggerD("Currently unsupported");
            // TODO add class which will extended message_service and call message_service_short_msg
            break;
        case MessageType::EMAIL:
                if (email_get_account_list(&email_accounts, &count) != EMAIL_ERROR_NONE) {
                    LoggerE("Method failed: email_get_account_list()");
                    throw common::UnknownException("Error during getting account list");
                }
                else {
                    std::stringstream stream_name;
                    for (int i = 0; i < count; ++i) {
                        stream_name << "[" << email_accounts[i].account_name
                                << "] "
                                << email_accounts[i].incoming_server_user_name;
                        LoggerD("Account[%d/%d] id: %d, name: %s", i,
                                count, email_accounts[i].account_id, stream_name.str().c_str());

                        messageService = new (std::nothrow) MessageServiceEmail(
                                email_accounts[i].account_id, stream_name.str());
                        if (!messageService) {
                            LoggerD("message service[%d] is NULL", i);
                            unsigned int count_srvcs = msgServices.size();
                            for (unsigned int j = 0; j < count_srvcs; ++j) {
                                delete msgServices.at(j);
                            }
                            msgServices.clear();
                            LoggerE("MessageService for email creation failed");
                            throw common::UnknownException("MessageService for email creation failed");
                            break;
                        }
                        else {
                            msgServices.push_back(messageService);
                        }
                        messageService = NULL;
                        stream_name.str("");
                    }

                    std::vector<picojson::value> response;
                    std::for_each(msgServices.begin(), msgServices.end(),
                        [&response](MessageService* service) {
                              response.push_back(picojson::value(service->toPicoJS()));
                        }
                    );
                    obj[JSON_DATA] = picojson::value(response);
                    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);
                }
                break;
        default:
            LoggerE("Unsupported services type");
            throw common::UnknownException("Unsupported services type");
        }
    } catch(const common::PlatformException& e) {
          LoggerE("Unknown error");
          obj[JSON_DATA] = e.ToJSON();
          obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
    }

    return nullptr;
}

void MessagingManager::getMessageServices(const std::string& type, double callbackId)
{
    LoggerD("Entered");

    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
    obj[JSON_DATA] = picojson::value(type);

    common::TaskQueue::GetInstance().Queue<picojson::value>
        (getMsgServicesThread, callbackCompleted, json);
}

} // namespace messaging
} // namespace extension

