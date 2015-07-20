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
 
#include "message_service.h"

#include <sstream>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/assert.h"

#include "messaging_util.h"
#include "message_storage_email.h"
#include "message_storage_short_msg.h"
#include "message.h"

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {

namespace{
const char* JSON_SERVICE_ID = "id";
const char* JSON_SERVICE_TYPE = "type";
const char* JSON_SERVICE_NAME = "name";
}

//#################### MessageRecipientsCallbackData ####################

MessageRecipientsCallbackData::MessageRecipientsCallbackData(PostQueue& queue):
        m_is_error(false),
        m_account_id(-1),
        m_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN),
        m_default_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN),
        queue_(queue)
{
    LoggerD("Entered");
    m_msg_recipients = std::vector<std::string>();
}

MessageRecipientsCallbackData::~MessageRecipientsCallbackData()
{
    LoggerD("Entered");
}

void MessageRecipientsCallbackData::setMessage(std::shared_ptr<Message> message)
{
    m_message = message;
}

std::shared_ptr<Message> MessageRecipientsCallbackData::getMessage() const
{
    return m_message;
}

void MessageRecipientsCallbackData::setMessageRecipients(
        const std::vector<std::string>& msgRecipients)
{
    m_msg_recipients = msgRecipients;
}

const std::vector<std::string>& MessageRecipientsCallbackData::getMessageRecipients() const
{
    return m_msg_recipients;
}

void MessageRecipientsCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    LoggerD("Entered");
    // keep only first error in chain
    if (!m_is_error) {
        LoggerD("Error has not been set yet");
        m_is_error = true;

        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
        auto obj_error = picojson::object();

        obj_error[JSON_ERROR_NAME] = picojson::value(err_name);
        obj_error[JSON_ERROR_MESSAGE] = picojson::value(err_message);

        obj[JSON_DATA] = picojson::value(obj_error);
    }
}

void MessageRecipientsCallbackData::setError(const PlatformResult& error)
{
  LoggerD("Entered");
  // keep only first error in chain
  if (!m_is_error) {
    LoggerD("Error has not been set yet");
    m_is_error = true;

    picojson::object& obj = m_json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
    auto obj_error = picojson::object();

    obj_error[JSON_ERROR_CODE] = picojson::value(static_cast<double>(error.error_code()));
    obj_error[JSON_ERROR_MESSAGE] = picojson::value(error.message());

    obj[JSON_DATA] = picojson::value(obj_error);
  }
}

bool MessageRecipientsCallbackData::isError() const
{
    return m_is_error;
}

void MessageRecipientsCallbackData::setAccountId(int account_id){
    m_account_id = account_id;
}

int MessageRecipientsCallbackData::getAccountId() const
{
    return m_account_id;
}

bool MessageRecipientsCallbackData::setSimIndex(
    int sim_index)
{
    LoggerD("Entered");
    char **cp_list = tel_get_cp_name_list();
    int sim_count = 0;
    if (cp_list) {
        while (cp_list[sim_count]) {
            sim_count++;
        }
        g_strfreev(cp_list);
    } else {
        LoggerD("Empty cp name list");
    }

    sim_index--;
    if (sim_index >= sim_count || sim_index < -1) {
        LoggerE("Sim index out of bound %d : %d", sim_index, sim_count);
        common::InvalidValuesException err("The index of sim is out of bound");
        this->setError(err.name(), err.message());
        return false;
    }

    m_sim_index = static_cast<TelNetworkDefaultDataSubs_t>(sim_index);

    return true;
}

TelNetworkDefaultDataSubs_t MessageRecipientsCallbackData::getSimIndex() const
{
    return m_sim_index;
}

bool MessageRecipientsCallbackData::isSetSimIndex() const
{
    return m_sim_index != TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN;
}

void MessageRecipientsCallbackData::setDefaultSimIndex(
    TelNetworkDefaultDataSubs_t sim_index)
{
    m_default_sim_index = sim_index;
}

TelNetworkDefaultDataSubs_t MessageRecipientsCallbackData::getDefaultSimIndex() const
{
    return m_default_sim_index;
}

//#################### BaseMessageServiceCallbackData ####################

BaseMessageServiceCallbackData::BaseMessageServiceCallbackData():
//        CallbackUserData(globalCtx),
        m_is_error(false),
        m_op_handle(-1),
        m_callback_id(-1)
{
    LoggerD("Entered");
}

BaseMessageServiceCallbackData::~BaseMessageServiceCallbackData()
{
    LoggerD("Entered");
}

void BaseMessageServiceCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    LoggerD("Entered");
    // keep only first error in chain
    if (!m_is_error) {
        LoggerD("Error has not been set yet");
        m_is_error = true;

        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

        auto obj_error = picojson::object();
        obj_error[JSON_ERROR_NAME] = picojson::value(err_name);
        obj_error[JSON_ERROR_MESSAGE] = picojson::value(err_message);
        obj[JSON_DATA] = picojson::value(obj_error);
    }
}

void BaseMessageServiceCallbackData::setError(const PlatformResult& error)
{
  LoggerD("Entered");
  // keep only first error in chain
  if (!m_is_error) {
    LoggerD("Error has not been set yet");
    m_is_error = true;

    picojson::object& obj = m_json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
    auto obj_error = picojson::object();

    obj_error[JSON_ERROR_CODE] = picojson::value(static_cast<double>(error.error_code()));
    obj_error[JSON_ERROR_MESSAGE] = picojson::value(error.message());

    obj[JSON_DATA] = picojson::value(obj_error);
  }
}

bool BaseMessageServiceCallbackData::isError() const
{
    return m_is_error;
}

void BaseMessageServiceCallbackData::setOperationHandle(const int op_handle)
{
    m_op_handle = op_handle;
}

int BaseMessageServiceCallbackData::getOperationHandle() const
{
    return m_op_handle;
}

void BaseMessageServiceCallbackData::setCallbackId(const double callback_id)
{
    m_callback_id = callback_id;
}

double BaseMessageServiceCallbackData::getCallbackId() const
{
    return m_callback_id;
}

//#################### MessageBodyCallbackData ####################

MessageBodyCallbackData::MessageBodyCallbackData(PostQueue& queue):
    queue_(queue)
{
    LoggerD("Entered");
}

MessageBodyCallbackData::~MessageBodyCallbackData()
{
    LoggerD("Entered");
}

void MessageBodyCallbackData::setMessage(std::shared_ptr<Message> message)
{
    m_message = message;
}

std::shared_ptr<Message> MessageBodyCallbackData::getMessage() const
{
    return m_message;
}

//#################### MessageAttachmentCallbackData ####################

MessageAttachmentCallbackData::MessageAttachmentCallbackData(PostQueue& queue):
        m_nth(0),
        queue_(queue)
{
    LoggerD("Entered");
}

MessageAttachmentCallbackData::~MessageAttachmentCallbackData()
{
    LoggerD("Entered");
}

void MessageAttachmentCallbackData::setMessageAttachment(
        std::shared_ptr<MessageAttachment> messageAttachment)
{
    m_message_attachment = messageAttachment;
}

std::shared_ptr<MessageAttachment> MessageAttachmentCallbackData::
    getMessageAttachment() const
{
    return m_message_attachment;
}

void MessageAttachmentCallbackData::setNth(const int nth)
{
    m_nth = nth;
}

int MessageAttachmentCallbackData::getNth() const
{
    return m_nth;
}

//#################### SyncCallbackData ####################

SyncCallbackData::SyncCallbackData(PostQueue& queue):
//        BaseMessageServiceCallbackData(globalCtx),
        m_is_limit(false),
        m_limit(0),
        m_op_id(-1),
        m_account_id(-1),
        queue_(queue)
{
    LoggerD("Entered");
}

SyncCallbackData::~SyncCallbackData()
{
    LoggerD("Entered");
}

void SyncCallbackData::setLimit(const unsigned long limit)
{
    m_is_limit = true;
    m_limit = limit;
}

bool SyncCallbackData::isLimit() const
{
    return m_is_limit;
}

unsigned long SyncCallbackData::getLimit() const
{
    return m_limit;
}

void SyncCallbackData::setOpId(long op_id)
{
    m_op_id = op_id;
}

long SyncCallbackData::getOpId()
{
    return m_op_id;
}

void SyncCallbackData::setAccountId(int account_id)
{
    m_account_id = account_id;
}

int SyncCallbackData::getAccountId() const
{
    return m_account_id;
}

//#################### SyncFolderCallbackData ####################

SyncFolderCallbackData::SyncFolderCallbackData(PostQueue& queue):
    SyncCallbackData(queue)
{
    LoggerD("Entered");
}

SyncFolderCallbackData::~SyncFolderCallbackData()
{
    LoggerD("Entered");
}

void SyncFolderCallbackData::setMessageFolder(
        std::shared_ptr<MessageFolder> message_folder)
{
    m_message_folder = message_folder;
}

std::shared_ptr<MessageFolder> SyncFolderCallbackData::getMessageFolder() const
{
    return m_message_folder;
}

//#################### MessageService ####################

MessageService::MessageService(int id,
                    MessageType msgType,
                    const std::string& name):
        m_id(id),
        m_msg_type(msgType),
        m_name(name)
{
    LoggerD("Entered");
    switch (msgType) {
        case MessageType::SMS:
        case MessageType::MMS:
            m_storage.reset(new MessageStorageShortMsg(id, msgType));
            break;
        case MessageType::EMAIL:
            m_storage.reset(new MessageStorageEmail(id));
            break;
        default:
            LoggerE("Undefined message type: %d", msgType);
            Assert(false);
            break;
    }
}

MessageService::~MessageService()
{
    LoggerD("Entered");
}

picojson::object MessageService::toPicoJS() const
{
    LoggerD("Entered");
    picojson::object picojs = picojson::object();
    picojs[JSON_SERVICE_ID] = picojson::value(std::to_string(m_id));
    picojs[JSON_SERVICE_TYPE] = picojson::value(getMsgServiceTypeString());
    picojs[JSON_SERVICE_NAME] = picojson::value(m_name);
    return picojs;
}

int MessageService::getMsgServiceId() const
{
    return m_id;
}

std::string MessageService::getMsgServiceIdStr() const
{
    return std::to_string(m_id);
}

MessageType MessageService::getMsgServiceType() const
{
    return m_msg_type;
}

std::string MessageService::getMsgServiceTypeString() const {
  return MessagingUtil::messageTypeToString(getMsgServiceType());
}

std::string MessageService::getMsgServiceName() const
{
    return m_name;
}

MessageStoragePtr MessageService::getMsgStorage() const
{
    return m_storage;
}

common::PlatformResult MessageService::sendMessage(MessageRecipientsCallbackData *callback)
{
    // this method should be overwritten be specific services
    LoggerE("Cannot send message");
    delete callback;
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Unable to send message.");
}

PlatformResult MessageService::loadMessageBody(MessageBodyCallbackData* callback)
{
    // this method should be overwritten by specific services
    LoggerE("Cannot load message body");
    delete callback;
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Cannot load message body");
}

PlatformResult MessageService::loadMessageAttachment(MessageAttachmentCallbackData* callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot load message attachment");
    delete callback;
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Cannot load message attachment");
}

PlatformResult MessageService::sync(SyncCallbackData *callback, long* operation_id)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot sync with external server");
    delete callback;
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Cannot sync with external server");
}

PlatformResult MessageService::syncFolder(SyncFolderCallbackData *callback, long* operation_id)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot sync folder with external server");
    delete callback;
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Cannot sync folder with external server");
}

PlatformResult MessageService::stopSync(long op_id)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot stop sync with external server");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Cannot stop sync with external server");
}

} // messaging
} // extension

