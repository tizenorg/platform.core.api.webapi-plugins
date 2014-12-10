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

#include <JSWebAPIErrorFactory.h>
#include <JSWebAPIError.h>
#include <JSUtil.h>
#include <Logger.h>
#include <tapi_common.h>
#include <ITapiSim.h>
#include <ITapiNetwork.h>
#include "MessageServiceShortMsg.h"
#include "ShortMsgManager.h"
#include "JSMessage.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

MessageServiceShortMsg::MessageServiceShortMsg(int id, MessageType msgType)
        : MessageService(id,
                msgType,
                MessagingUtil::messageTypeToString(msgType))
{
    LOGD("Entered");
}

MessageServiceShortMsg::~MessageServiceShortMsg()
{
    LOGD("Entered");
}

static gboolean sendMessageThread(void* data)
{
    LOGD("Entered");

    auto callback = static_cast<MessageRecipientsCallbackData *>(data);
    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    ShortMsgManager::getInstance().sendMessage(callback);
    return FALSE;
}

void MessageServiceShortMsg::sendMessage(MessageRecipientsCallbackData *callback)
{
    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
        LOGE("Incorrect message type");
        throw Common::TypeMismatchException("Incorrect message type");
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
        LOGE("Failed to find default sim index %d", ret);
    }

    LOGD("Default sim index: %d", default_sim);
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
            LOGD("Empty cp name list");
        }

        if (sim_index >= sim_count) {
            LOGE("Sim index out of count %d : %d", sim_index, sim_count);
            throw InvalidValuesException("The index of sim is out of bound");
        }

        callback->getMessage()->setSimIndex(sim_index);
    } else {
        callback->getMessage()->setSimIndex(default_sim);
    }

    if(!g_idle_add(sendMessageThread, static_cast<void*>(callback))) {
        LOGE("g_idle_add fails");
        throw UnknownException("Could not add task");
    }
}

static gboolean loadMessageBodyTask(void* data)
{
    LOGD("Entered");
    MessageBodyCallbackData* callback = static_cast<MessageBodyCallbackData*>(data);
    if(!callback) {
        LOGE("callback is NULL");
        return FALSE;
    }

    try {
        JSContextRef context = callback->getContext();
        JSObjectRef jsMessage = JSMessage::makeJSObject(context, callback->getMessage());
        callback->callSuccessCallback(jsMessage);
    } catch (...) {
        LOGE("Couldn't create JSMessage object!");
        callback->callErrorCallback();
    }

    return FALSE;
}

void MessageServiceShortMsg::loadMessageBody(MessageBodyCallbackData *callback)
{
    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
        LOGE("Incorrect message type");
        throw Common::TypeMismatchException("Incorrect message type");
    }

    guint id = g_idle_add(loadMessageBodyTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add fails");
        throw Common::UnknownException("Could not add task");
    }
}

} // Messaging
} // DeviceAPI

