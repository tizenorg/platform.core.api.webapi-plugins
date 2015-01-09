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
#include "short_message_manager.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

namespace {
const int UNDEFINED_MESSAGE_SERVICE = -1;
}

MessagingManager::MessagingManager()
{
    LoggerD("Entered");
    int ret = msg_open_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LoggerE("Cannot get message handle: %d", ret);
    } else {
        ShortMsgManager::getInstance().registerStatusCallback(m_msg_handle);
    }

    m_sms_service = std::make_pair(UNDEFINED_MESSAGE_SERVICE, nullptr);
}

MessagingManager::~MessagingManager()
{
    LoggerD("Entered");
    int ret = msg_close_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LoggerW("Cannot close message handle: %d", ret);
    }

    std::for_each(m_email_services.begin(), m_email_services.end(),
            [](std::pair<int, MessageService*> el) {
                delete el.second;
        }
    );
    m_email_services.clear();

    if (m_sms_service.second) {
        delete m_sms_service.second;
    }
}

MessagingManager& MessagingManager::getInstance()
{
    LoggerD("Entered");
    static MessagingManager instance;
    return instance;
}

static gboolean callbackCompleted(const std::shared_ptr<MsgManagerCallbackData>& user_data)
{
    LoggerD("Entered");
    std::shared_ptr<picojson::value> response = user_data->json;
    MessagingInstance::getInstance().PostMessage(response->serialize().c_str());
    return false;
}

static void* getMsgServicesThread(const std::shared_ptr<MsgManagerCallbackData>& user_data)
{
    LoggerD("Entered");

    std::shared_ptr<picojson::value> response = user_data->json;
    picojson::object& obj = response->get<picojson::object>();
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
            LoggerD("MessageService for SMS");
            {
                if (user_data->sms_service->second) delete user_data->sms_service->second;

                MessageService* service = new(std::nothrow) MessageServiceShortMsg(
                        MessageServiceAccountId::SMS_ACCOUNT_ID,
                        MessageType::SMS);
                if (!service) {
                    LoggerE("MessageService for SMS creation failed");
                    throw common::UnknownException("MessageService for email creation failed");
                }
                *(user_data->sms_service) = std::make_pair(service->getMsgServiceId(), service);

                // TODO FIXME
                picojson::array array;
                array.push_back(picojson::value(service->toPicoJS()));
                obj[JSON_DATA] = picojson::value(array);
                obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

                service = NULL;
            }
            break;
        case MessageType::MMS:
            LoggerD("Currently unsupported");
            // TODO add class which will extended message_service and call message_service_short_msg
            break;
        case MessageType::EMAIL:
                // TODO FIXME need to work on readability of that case
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

                    std::map<int, MessageService*>& email_services = *(user_data->services_map);
                    std::for_each(email_services.begin(), email_services.end(),
                            [](std::pair<int, MessageService*> el) {
                                delete el.second;
                        }
                    );
                    email_services.clear();

                    std::vector<picojson::value> response;
                    std::for_each(msgServices.begin(), msgServices.end(),
                        [&response, &email_services](MessageService* service) {
                              response.push_back(picojson::value(service->toPicoJS()));
                              email_services.insert(
                                      std::pair<int, MessageService*>(
                                              service->getMsgServiceId(),
                                              service));
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

    auto user_data = std::shared_ptr<MsgManagerCallbackData>(new MsgManagerCallbackData());
    user_data->json = json;
    user_data->services_map = &m_email_services;
    user_data->sms_service = &m_sms_service;

    common::TaskQueue::GetInstance().Queue<MsgManagerCallbackData>
        (getMsgServicesThread, callbackCompleted, user_data);
}

MessageService* MessagingManager::getMessageService(const int id) {
    if (id == m_sms_service.first) {
        return m_sms_service.second;
    } else {
        return m_email_services[id];
    }
}

} // namespace messaging
} // namespace extension

