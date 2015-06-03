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

#include "message_service_short_msg.h"
#include "messaging_instance.h"
#include "short_message_manager.h"
#include "common/logger.h"
#include "common/platform_exception.h"

#include <tapi_common.h>
#include <ITapiSim.h>
#include <ITapiNetwork.h>

//#include <JSWebAPIErrorFactory.h>
//#include <JSWebAPIError.h>
//#include <JSUtil.h>
//#include "JSMessage.h"

//using namespace DeviceAPI::Common;

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {

MessageServiceShortMsg::MessageServiceShortMsg(int id, MessageType msgType)
        : MessageService(id,
                msgType,
                MessagingUtil::messageTypeToString(msgType))
{
    LoggerD("Entered");
}

MessageServiceShortMsg::~MessageServiceShortMsg()
{
    LoggerD("Entered");
}

static gboolean sendMessageThread(void* data)
{
  LoggerD("Entered");

  auto ret = ShortMsgManager::getInstance().sendMessage(static_cast<MessageRecipientsCallbackData*>(data));

  if (!ret) {
    LoggerE("Error: %d - %s", ret.error_code(), ret.message().c_str());
  }

  return FALSE;
}

PlatformResult MessageServiceShortMsg::sendMessage(MessageRecipientsCallbackData *callback)
{
    LoggerD("Entered");
    if (!callback) {
        LoggerE("Callback is null");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
    }

    /*
     * Set sim index.
     * If user has set sim index manually, check sim index is valid.
     * Otherwise, use default sim which is already set.
     */
    TelNetworkDefaultDataSubs_t default_sim =
        TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN;
    TapiHandle *handle = tel_init(NULL);

    int ret = tel_get_network_default_data_subscription(handle, &default_sim);
    if (ret != TAPI_API_SUCCESS) {
        LoggerE("Failed to find default sim index %d", ret);
    }

    LoggerD("Default sim index: %d", default_sim);
    callback->setDefaultSimIndex(default_sim);
    tel_deinit(handle);

    // simIndex parameter is only available for sms message.
    // In case of mms, the parameter is silently ignored.
    if (callback->isSetSimIndex() &&
        callback->getMessage()->getType() == MessageType::SMS) {
        char **cp_list = tel_get_cp_name_list();
        TelNetworkDefaultDataSubs_t sim_index = callback->getSimIndex();
        int sim_count = 0;

        if (cp_list) {
            while (cp_list[sim_count]) {
                sim_count++;
            }
            g_strfreev(cp_list);
        } else {
            LoggerD("Empty cp name list");
        }

        if (sim_index >= sim_count) {
            LoggerE("Sim index out of count %d : %d", sim_index, sim_count);
            delete callback;
            return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "The index of sim is out of bound");
        }

        callback->getMessage()->setSimIndex(sim_index);
    } else {
        callback->getMessage()->setSimIndex(default_sim);
    }

    if(!g_idle_add(sendMessageThread, static_cast<void*>(callback))) {
        LoggerE("g_idle_add fails");
        delete callback;
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean loadMessageBodyTask(void* data)
{
    LoggerD("Entered");
    MessageBodyCallbackData* callback = static_cast<MessageBodyCallbackData*>(data);
    if(!callback) {
        LoggerE("callback is NULL");
        return FALSE;
    }

    std::shared_ptr<MessageBody> body = callback->getMessage()->getBody();
    auto json = callback->getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      picojson::object args;
      args[JSON_DATA_MESSAGE_BODY] = MessagingUtil::messageBodyToJson(body);
      obj[JSON_DATA] = picojson::value(args);

      callback->getQueue().resolve(
          obj.at(JSON_CALLBACK_ID).get<double>(),
          json->serialize()
      );
    } else {
      LoggerE("json is incorrect - missing required member");
    }
    return FALSE;
}

PlatformResult MessageServiceShortMsg::loadMessageBody(MessageBodyCallbackData *callback)
{
    LoggerD("Entered");
    if (!callback) {
        LoggerE("Callback is null");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
    }

    guint id = g_idle_add(loadMessageBodyTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add fails");
        delete callback;
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

MessageServiceShortMsg* MessageServiceShortMsg::GetMmsMessageService() {
  return new (std::nothrow) MessageServiceShortMsg(
      MessageServiceAccountId::MMS_ACCOUNT_ID, MessageType::MMS);
}

MessageServiceShortMsg* MessageServiceShortMsg::GetSmsMessageService() {
  return new (std::nothrow) MessageServiceShortMsg(
      MessageServiceAccountId::SMS_ACCOUNT_ID, MessageType::SMS);
}

} // namespace messaging
} // namespace extension

