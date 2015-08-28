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

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {

namespace {
const int UNDEFINED_MESSAGE_SERVICE = -1;
}

MsgManagerCallbackData::MsgManagerCallbackData(MessagingInstance& instance):
    json(nullptr),
    services_map(nullptr),
    sms_service(nullptr),
    mms_service(nullptr),
    instance_(instance) {
    LoggerD("Entered");
}

MessagingManager::MessagingManager(MessagingInstance& instance):
    instance_(instance)
{
    LoggerD("Entered");
    int ret = msg_open_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LoggerE("Cannot get message handle: %d", ret);
    } else {
        ShortMsgManager::getInstance().registerStatusCallback(m_msg_handle);
    }

    m_sms_service = std::make_pair(UNDEFINED_MESSAGE_SERVICE, nullptr);
    m_mms_service = std::make_pair(UNDEFINED_MESSAGE_SERVICE, nullptr);
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
    if (m_mms_service.second) {
        delete m_mms_service.second;
    }
}

static gboolean callbackCompleted(const std::shared_ptr<MsgManagerCallbackData>& user_data)
{
    LoggerD("Entered");
    std::shared_ptr<picojson::value> response = user_data->json;
    common::Instance::PostMessage(&user_data->instance_, response->serialize().c_str());
    return false;
}

static void* getMsgServicesThread(const std::shared_ptr<MsgManagerCallbackData>& user_data)
{
  LoggerD("Entered");

  std::shared_ptr<picojson::value> response = user_data->json;
  picojson::object& obj = response->get<picojson::object>();
  MessageType type = MessageType::UNDEFINED;

  auto platform_result = MessagingUtil::stringToMessageType(response->get(JSON_DATA).get<std::string>(), &type);

  if (platform_result) {
    switch (type) {
      case MessageType::SMS:
        LoggerD("MessageService for SMS");
        {
          if (user_data->sms_service->second) {
            delete user_data->sms_service->second;
          }

          MessageService* service = MessageServiceShortMsg::GetSmsMessageService();

          if (!service) {
            LoggerE("MessageService for SMS creation failed");
            platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "MessageService for SMS creation failed");
          } else {
            *(user_data->sms_service) = std::make_pair(service->getMsgServiceId(), service);

            // TODO FIXME
            picojson::array array;
            array.push_back(picojson::value(service->toPicoJS()));
            obj[JSON_DATA] = picojson::value(array);
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            // service is stored, so it cannot be deleted
            service = nullptr;
          }
        }
        break;

      case MessageType::MMS:
        LoggerD("MessageService for MMS");
        {
          if (user_data->mms_service->second) {
            delete user_data->mms_service->second;
          }

          MessageService* service = MessageServiceShortMsg::GetMmsMessageService();

          if (!service) {
            LoggerE("MessageService for MMS creation failed");
            platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "MessageService for SMS creation failed");
          } else {
            *(user_data->mms_service) = std::make_pair(service->getMsgServiceId(), service);

            picojson::array array;
            array.push_back(picojson::value(service->toPicoJS()));
            obj[JSON_DATA] = picojson::value(array);
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            // service is stored, so it cannot be deleted
            service = nullptr;
          }
        }
        break;

      case MessageType::EMAIL:
        LoggerD("MessageService for EMAIL");
        {
          email_account_t* email_accounts = nullptr;
          int count = 0;

          // TODO FIXME need to work on readability of that case
          if (email_get_account_list(&email_accounts, &count) != EMAIL_ERROR_NONE) {
            LoggerE("Method failed: email_get_account_list()");
            platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Error during getting account list");
          } else {
            std::vector<MessageService*> msgServices;

            for (int i = 0; i < count && platform_result; ++i) {
              std::string name = "[";
              if (email_accounts[i].account_name) {
                name += email_accounts[i].account_name;
              }
              name += "] ";
              name += email_accounts[i].incoming_server_user_name;
              LoggerD("Account[%d/%d] id: %d, name: %s", i, count,
                      email_accounts[i].account_id, name.c_str());

              MessageService* service = new (std::nothrow) MessageServiceEmail(email_accounts[i].account_id,
                                                                               name.c_str());
              if (!service) {
                LoggerD("message service[%d] is NULL", i);
                std::for_each(msgServices.begin(), msgServices.end(),
                              [](MessageService* service) {
                                delete service;
                              });
                msgServices.clear();
                LoggerE("MessageService for email creation failed");
                platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "MessageService for email creation failed");
              } else {
                msgServices.push_back(service);
              }

              // service is stored, so it cannot be deleted
              service = nullptr;
            }

            email_free_account(&email_accounts, count);

            if (platform_result) {
              std::map<int, MessageService*>& email_services = *(user_data->services_map);
              std::for_each(email_services.begin(), email_services.end(),
                            [](std::pair<int, MessageService*> el) {
                              delete el.second;
                            });
              email_services.clear();

              std::vector<picojson::value> response;
              std::for_each(msgServices.begin(), msgServices.end(),
                            [&response, &email_services](MessageService* service) {
                              response.push_back(picojson::value(service->toPicoJS()));
                              email_services.insert(std::pair<int, MessageService*>(service->getMsgServiceId(), service));
                            });
              obj[JSON_DATA] = picojson::value(response);
              obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);
            }
          }
        }
        break;
      default:
        platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Service type is undefined");
    }
  } else {
    LoggerE("Unsupported service type");
    platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unsupported service type");
  }

  if (!platform_result) {
    LoggerE("Unknown error");
    obj[JSON_DATA] = platform_result.ToJSON();
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

    auto user_data = std::shared_ptr<MsgManagerCallbackData>(new MsgManagerCallbackData(instance_));
    user_data->json = json;
    user_data->services_map = &m_email_services;
    user_data->sms_service = &m_sms_service;
    user_data->mms_service = &m_mms_service;

    common::TaskQueue::GetInstance().Queue<MsgManagerCallbackData>
        (getMsgServicesThread, callbackCompleted, user_data);
}

MessageService* MessagingManager::getMessageService(const int id) {
    if (id == m_sms_service.first) {
        return m_sms_service.second;
    } else if (id == m_mms_service.first) {
        return m_mms_service.second;
    } else {
        return m_email_services[id];
    }
}

} // namespace messaging
} // namespace extension

