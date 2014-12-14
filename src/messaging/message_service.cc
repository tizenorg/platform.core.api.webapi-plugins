
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

long MessageService::sync(const double callbackId, long limit)
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

void MessageService::stopSync()
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LoggerE("Cannot stop sync with external server");
    throw common::NotSupportedException("Cannot stop sync with external server");
}

} // messaging
} // extension

