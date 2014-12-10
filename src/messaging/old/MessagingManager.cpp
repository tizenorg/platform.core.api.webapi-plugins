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
#include <GlobalContextManager.h>

#include <system_info.h>

#include <email-types.h>
#include <email-api.h>

#include "MessagingManager.h"
#include "MessagingUtil.h"
#include "JSMessageService.h"
#include "MessageService.h"
#include "MessageServiceShortMsg.h"
#include "MessageServiceEmail.h"
#include "ShortMsgManager.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

MessageServiceCallbackData::MessageServiceCallbackData(JSContextRef globalCtx):
    CallbackUserData(globalCtx),
    m_msg_type(MessageType::UNDEFINED),
    m_is_error(false)
{
    LOGD("Entered");
}

MessageServiceCallbackData::~MessageServiceCallbackData()
{
    LOGD("Entered");
}

void MessageServiceCallbackData::setMessageType(MessageType msgType)
{
    m_msg_type = msgType;
}

MessageType MessageServiceCallbackData::getMessageType() const
{
    return m_msg_type;
}

void MessageServiceCallbackData::setMessageServices(
        const std::vector<MessageService*>& msgServices)
{
    m_msg_services = msgServices;
}

const std::vector<MessageService*>& MessageServiceCallbackData::getMessageServices() const
{
    return m_msg_services;
}

void MessageServiceCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

bool MessageServiceCallbackData::isError() const
{
    return m_is_error;
}

std::string MessageServiceCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string MessageServiceCallbackData::getErrorMessage() const
{
    return m_err_message;
}

void MessageServiceCallbackData::clearServices()
{
    unsigned int count = m_msg_services.size();
    for (unsigned int i = 0; i < count; ++i) {
        delete m_msg_services.at(i);
    }
    m_msg_services.clear();
}


static gboolean getMsgServicesCompleteCB(void* data)
{
    LOGD("Entered");

    MessageServiceCallbackData* callback = static_cast<MessageServiceCallbackData*>(data);
    if (!callback) {
        LOGE("Callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }

    try {
        if (callback->isError()) {
            JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(errobj);
        }
        else {
            std::vector<MessageService*> msgServices = callback->getMessageServices();
            unsigned int count = msgServices.size();
            JSValueRef result = NULL;

            JSObjectRef jsMsgServicesObject[count];
            for (unsigned int i = 0; i < count; ++i) {
                jsMsgServicesObject[i] = JSMessageService::createJSObject(context,
                        msgServices.at(i));
                if (NULL == jsMsgServicesObject[i]) {
                    LOGE("Message service object creation failed");
                    throw Common::UnknownException(
                            "Message service object creation failed");
                }
            }
            result = JSObjectMakeArray(context, count,
                    count > 0 ? jsMsgServicesObject : NULL, NULL);

            if (!result) {
                JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                        JSWebAPIErrorFactory::UNKNOWN_ERROR,
                        "Could not create JS array object");
                callback->clearServices();
                callback->callErrorCallback(errobj);
            }
            else {
                callback->callSuccessCallback(result);
            }
        }
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context, err);
        callback->clearServices();
        callback->callErrorCallback(errobj);
    }
    catch (...) {
        JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                JSWebAPIErrorFactory::UNKNOWN_ERROR,
                "Cannot retrieve JS message services");
        callback->clearServices();
        callback->callErrorCallback(errobj);
    }

    delete callback;
    callback = NULL;

    return false;
}

static void* getMsgServicesThread(void* data)
{
    LOGD("Entered");

    MessageServiceCallbackData* callback = static_cast<MessageServiceCallbackData*>(data);
    if(!callback){
        LOGE("Callback is null");
        return NULL;
    }

    MessageType msgType = callback->getMessageType();
    std::vector<MessageService*> msgServices;
    MessageService* messageService = NULL;
    email_account_t* email_accounts = NULL;
    int count = 0;
    bool isSupported = false;

    try {
        switch (msgType) {
        case MessageType::SMS:
            messageService = new(std::nothrow) MessageServiceShortMsg(
                    MessageServiceAccountId::SMS_ACCOUNT_ID,
                    MessageType::SMS);
            if (!messageService) {
                callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                        "MessageService for SMS creation failed");
            } else {
                msgServices.push_back(messageService);
            }
            callback->setMessageServices(msgServices);
            break;
        case MessageType::MMS:
            messageService = new(std::nothrow) MessageServiceShortMsg(
                    MessageServiceAccountId::MMS_ACCOUNT_ID,
                    MessageType::MMS);
            if (!messageService) {
                callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                        "MessageService for MMS creation failed");
            } else {
                msgServices.push_back(messageService);
            }
            callback->setMessageServices(msgServices);
            break;
        case MessageType::EMAIL:
            if (email_get_account_list(&email_accounts, &count) != EMAIL_ERROR_NONE) {
                callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                        "Cannot get email accounts");
            } else {
                std::stringstream stream_name;
                for (int i = 0; i < count; ++i) {
                    stream_name << "[" << email_accounts[i].account_name << "] "
                            << email_accounts[i].incoming_server_user_name;
                    SLOGD("Account[%d/%d] id: %d, name: %s", i, count,
                            email_accounts[i].account_id, stream_name.str().c_str());

                    messageService = new(std::nothrow) MessageServiceEmail(
                            email_accounts[i].account_id, stream_name.str());
                    if (!messageService) {
                        LOGE("message service[%d] is NULL", i);
                        unsigned int count_srvcs = msgServices.size();
                        for (unsigned int j = 0; j < count_srvcs; ++j) {
                            delete msgServices.at(j);
                        }
                        msgServices.clear();
                        callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                                "MessageService for email creation failed");
                        break;
                    }
                    else {
                        msgServices.push_back(messageService);
                    }

                    messageService = NULL;
                    stream_name.str("");
                }
                callback->setMessageServices(msgServices);
            }

            if (email_accounts != NULL) {
                email_free_account(&email_accounts, count);
                email_accounts = NULL;
            }
            break;
        default:
            callback->clearServices();
            callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                    "Invalid message service tag");
            break;
        }
    }
    catch (...) {
        unsigned int count_srvcs = msgServices.size();
        for (unsigned int j = 0; j < count_srvcs; ++j) {
            delete msgServices.at(j);
        }
        msgServices.clear();
        callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                "Cannot retrieve message services");
    }

    if (!g_idle_add(getMsgServicesCompleteCB, static_cast<void*>(callback))) {
        LOGE("g_idle addition failed");
        callback->clearServices();
        delete callback;
        callback = NULL;
    }

    return NULL;
}

MessagingManager& MessagingManager::getInstance()
{
    LOGD("Entered");

    static MessagingManager instance;
    return instance;
}

void MessagingManager::getMessageServices(MessageServiceCallbackData* callback)
{
    LOGD("Entered");

    pthread_t thread;
    if (pthread_create(&thread, NULL, getMsgServicesThread, static_cast<void*>(callback))) {
        LOGE("Thread creation failed");
        throw Common::UnknownException("Thread creation failed");
    }
    if (pthread_detach(thread)) {
        LOGE("Thread detachment failed");
    }
}

MessagingManager::MessagingManager():
        SecurityAccessor()
{
    LOGD("Entered");

    int ret = msg_open_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LOGE("Cannot get message handle: %d", ret);
    }
    else {
        ShortMsgManager::getInstance().registerStatusCallback(m_msg_handle);
    }
}

MessagingManager::~MessagingManager()
{
    LOGD("Entered");
    int ret = msg_close_msg_handle(&m_msg_handle);
    if (ret != MSG_SUCCESS) {
        LOGW("Cannot close message handle: %d", ret);
    }
}

} // Messaging
} // DeviceAPI
