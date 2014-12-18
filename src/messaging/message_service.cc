
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "message_service.h"

#include <sstream>

#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"
#include "message_storage_email.h"

namespace extension {
namespace messaging {

namespace{
const char* JSON_SERVICE_ID = "id";
const char* JSON_SERVICE_TYPE = "type";
const char* JSON_SERVICE_NAME = "name";
const char* JSON_SERVICE_STORAGE = "messageStorage";
}

//#################### BaseMessageServiceCallbackData ####################

BaseMessageServiceCallbackData::BaseMessageServiceCallbackData():
//        CallbackUserData(globalCtx),
        m_is_error(false),
        m_op_handle(-1)
{
    LOGD("Entered");
}

BaseMessageServiceCallbackData::~BaseMessageServiceCallbackData()
{
    LOGD("Entered");
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

//#################### SyncCallbackData ####################

SyncCallbackData::SyncCallbackData():
//        BaseMessageServiceCallbackData(globalCtx),
        m_is_limit(false),
        m_limit(0),
        m_account_id(-1)
{
    LOGD("Entered");
}

SyncCallbackData::~SyncCallbackData()
{
    LOGD("Entered");
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

void MessageService::sendMessage()
{
    // this method should be overwritten be specific services
    LoggerE("Cannot send message");
    throw common::NotSupportedException("Cannot send message");
}

void MessageService::loadMessageBody()
{
    // this method should be overwritten by specific services
    LoggerE("Cannot load message body");
    throw common::NotSupportedException("Cannot load message body");
}

void MessageService::loadMessageAttachment()
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

long MessageService::syncFolder()
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

