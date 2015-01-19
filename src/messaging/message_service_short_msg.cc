// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

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

    auto callback = static_cast<MessageRecipientsCallbackData *>(data);
    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    // TODO
    ShortMsgManager::getInstance().sendMessage(callback);
    return FALSE;
}

void MessageServiceShortMsg::sendMessage(MessageRecipientsCallbackData *callback)
{
    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
        LoggerE("Incorrect message type");
        throw common::TypeMismatchException("Incorrect message type");
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
            throw common::InvalidValuesException("The index of sim is out of bound");
        }

        callback->getMessage()->setSimIndex(sim_index);
    } else {
        callback->getMessage()->setSimIndex(default_sim);
    }

    if(!g_idle_add(sendMessageThread, static_cast<void*>(callback))) {
        LoggerE("g_idle_add fails");
        throw common::UnknownException("Could not add task");
    }
}

static gboolean loadMessageBodyTask(void* data)
{
    LoggerD("Entered");
    MessageBodyCallbackData* callback = static_cast<MessageBodyCallbackData*>(data);
    if(!callback) {
        LoggerE("callback is NULL");
        return FALSE;
    }

    try {
        std::shared_ptr<MessageBody> body = callback->getMessage()->getBody();
        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        picojson::object args;
        args[JSON_DATA_MESSAGE_BODY] = MessagingUtil::messageBodyToJson(body);
        obj[JSON_DATA] = picojson::value(args);

        MessagingInstance::getInstance().PostMessage(json->serialize().c_str());
    } catch (...) {
        LoggerE("Couldn't create JSMessage object!");
        common::UnknownException e("Loade message body failed");
        callback->setError(e.name(), e.message());
        MessagingInstance::getInstance().PostMessage(callback->getJson()->serialize().c_str());
    }

    return FALSE;
}

void MessageServiceShortMsg::loadMessageBody(MessageBodyCallbackData *callback)
{
    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
        LoggerE("Incorrect message type");
        throw common::TypeMismatchException("Incorrect message type");
    }

    guint id = g_idle_add(loadMessageBodyTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add fails");
        throw common::UnknownException("Could not add task");
    }
}

} // namespace messaging
} // namespace extension

