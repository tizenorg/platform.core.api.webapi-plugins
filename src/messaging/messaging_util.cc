// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "messaging_util.h"

#include <map>
#include <stdexcept>

#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace messaging {

const char* JSON_CMD = "cmd";
const char* JSON_ACTION = "action";
const char* JSON_CALLBACK_ID = "cid";
const char* JSON_CALLBACK_SUCCCESS = "success";
const char* JSON_CALLBACK_ERROR = "error";
const char* JSON_CALLBACK_PROGRESS = "progress";
const char* JSON_CALLBACK_KEEP = "keep";
const char* JSON_DATA = "args";

namespace {
const std::string TYPE_SMS = "messaging.sms";
const std::string TYPE_MMS = "messaging.mms";
const std::string TYPE_EMAIL = "messaging.email";

const std::map<std::string, MessageType> stringToTypeMap = {
    {TYPE_SMS, MessageType::SMS},
    {TYPE_MMS, MessageType::MMS},
    {TYPE_EMAIL, MessageType::EMAIL}
};

const std::map<MessageType, std::string> typeToStringMap = {
    {MessageType::SMS, TYPE_SMS},
    {MessageType::MMS, TYPE_MMS},
    {MessageType::EMAIL, TYPE_EMAIL}
};

}

MessageType MessagingUtil::stringToMessageType(std::string str)
{
    try {
        return stringToTypeMap.at(str);
    }
    catch (const std::out_of_range& e) {
        std::string exceptionMsg = "Not supported type: ";
        exceptionMsg += str;
        LoggerE("%s", exceptionMsg.c_str());
        throw common::TypeMismatchException(exceptionMsg.c_str());
    }
}

std::string MessagingUtil::messageTypeToString(MessageType type)
{
    try {
        return typeToStringMap.at(type);
    }
    catch (const std::out_of_range& e) {
        LoggerE("Invalid MessageType");
        throw common::TypeMismatchException("Invalid MessageType");
    }
}

} // messaging
} // extension
