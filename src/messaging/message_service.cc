
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "message_service.h"

#include <sstream>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"
#include "message_storage_email.h"
#include "message.h"

namespace extension {
namespace messaging {

namespace{
const char* JSON_SERVICE_ID = "id";
const char* JSON_SERVICE_TYPE = "type";
const char* JSON_SERVICE_NAME = "name";
const char* JSON_SERVICE_STORAGE = "messageStorage";
}

//#################### MessageRecipientsCallbackData ####################

MessageRecipientsCallbackData::MessageRecipientsCallbackData():
        m_is_error(false),
        m_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN),
        m_default_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN)
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
    // keep only first error in chain
    if (!m_is_error) {

        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
        auto objData = picojson::object();

        objData[JSON_ERROR_NAME] = picojson::value(err_name);
        objData[JSON_ERROR_MESSAGE] = picojson::value(err_message);

        obj[JSON_DATA] = picojson::value(objData);

        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
        if (m_message) {
            m_err_message += " for: ";
            // platform issue: we cannot get error per recipient
            // so all recipients are added to error message
            std::vector<std::string> recp_list = m_message->getTO();
            unsigned int count = recp_list.size();
            for (unsigned int i = 0; i < count; ++i) {
                m_err_message += recp_list.at(i) + ", ";
            }
            recp_list = m_message->getCC();
            count = recp_list.size();
            for (unsigned int i = 0; i < count; ++i) {
                m_err_message += recp_list.at(i) + ", ";
            }
            recp_list = m_message->getBCC();
            count = recp_list.size();
            for (unsigned int i = 0; i < count; ++i) {
                m_err_message += recp_list.at(i) + ", ";
            }

        }
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
        m_op_handle(-1)
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
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;

        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

        auto obj_error = picojson::object();
        obj_error[JSON_ERROR_NAME] = picojson::value(err_name);
        obj_error[JSON_ERROR_MESSAGE] = picojson::value(err_message);
        obj[JSON_DATA] = picojson::value(obj_error);
    }
}

bool BaseMessageServiceCallbackData::isError() const
{
    return m_is_error;
}

std::string BaseMessageServiceCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string BaseMessageServiceCallbackData::getErrorMessage() const
{
    return m_err_message;
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

MessageBodyCallbackData::MessageBodyCallbackData()
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

MessageAttachmentCallbackData::MessageAttachmentCallbackData():
        m_nth(0)
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

SyncCallbackData::SyncCallbackData():
//        BaseMessageServiceCallbackData(globalCtx),
        m_is_limit(false),
        m_limit(0),
        m_account_id(-1)
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

SyncFolderCallbackData::SyncFolderCallbackData()
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
                    std::string name):
        m_id(id),
        m_msg_type(msgType),
        m_name(name)
{
    LoggerD("Entered");
    switch (msgType) {
        case MessageType::SMS:
        case MessageType::MMS:
            // TODO need to add class representing message storage for short messages
            //m_storage.reset(new MessageStorageShortMsg(id, msgType));
            break;
        case MessageType::EMAIL:
            m_storage.reset(new MessageStorageEmail(id));
            break;
        default:
            LoggerE("Undefined message type");
            throw common::InvalidValuesException("Undefined message type");
    }
}

MessageService::~MessageService()
{
    LoggerD("Entered");
}

picojson::object MessageService::toPicoJS() const
{
    picojson::object picojs = picojson::object();
    picojs[JSON_SERVICE_ID] = picojson::value(static_cast<double>(m_id));
    picojs[JSON_SERVICE_TYPE] = picojson::value(MessagingUtil::messageTypeToString(m_msg_type));
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

std::string MessageService::getMsgServiceName() const
{
    return m_name;
}

MessageStoragePtr MessageService::getMsgStorage() const
{
    return m_storage;
}

void MessageService::sendMessage(MessageRecipientsCallbackData *callback)
{
    // this method should be overwritten be specific services
    LoggerE("Cannot send message");
    throw common::NotSupportedException("Cannot send message");
}

void MessageService::loadMessageBody(MessageBodyCallbackData* callback)
{
    // this method should be overwritten by specific services
    LoggerE("Cannot load message body");
    throw common::NotSupportedException("Cannot load message body");
}

void MessageService::loadMessageAttachment(MessageAttachmentCallbackData* callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot load message attachment");
    throw common::NotSupportedException("Cannot load message attachment");
}

long MessageService::sync(SyncCallbackData *callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot sync with external server");
    throw common::NotSupportedException("Cannot sync with external server");
}

long MessageService::syncFolder(SyncFolderCallbackData *callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot sync folder with external server");
    throw common::NotSupportedException("Cannot sync folder with external server");
}

void MessageService::stopSync(long op_id)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot stop sync with external server");
    throw common::NotSupportedException("Cannot stop sync with external server");
}

} // messaging
} // extension

