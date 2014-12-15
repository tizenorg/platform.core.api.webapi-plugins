// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "messaging_util.h"

#include <fstream>
#include <map>
#include <stdexcept>
#include <streambuf>
#include <sstream>

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
const char* JSON_DATA = "data";

namespace {
const std::string TYPE_SMS = "messaging.sms";
const std::string TYPE_MMS = "messaging.mms";
const std::string TYPE_EMAIL = "messaging.email";
const std::string SENT = "SENT";
const std::string SENDING = "SENDING";
const std::string FAILED = "FAILED";
const std::string DRAFT = "DRAFT";

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

std::string MessagingUtil::ltrim(const std::string& input)
{
    std::string str = input;
    std::string::iterator i;
    for (i = str.begin(); i != str.end(); i++) {
        if (!isspace(*i)) {
            break;
        }
    }
    if (i == str.end()) {
        str.clear();
    } else {
        str.erase(str.begin(), i);
    }
    return str;
}

std::string MessagingUtil::extractSingleEmailAddress(const std::string& address)
{
    std::size_t found_begin = address.rfind('<');
    std::size_t found_end = address.rfind('>');
    // if both '<' and '>' bracket found and '<' is before '>'
    // then extract email address from the inside
    if(found_begin != std::string::npos &&
            found_end != std::string::npos &&
            found_begin < found_end) {
        return address.substr(found_begin+1, found_end-found_begin-1);
    }
    else {
        // return unmodified source string
        return address;
    }
}

std::vector<std::string> MessagingUtil::extractEmailAddresses(
        const std::vector<std::string>& addresses)
{
    std::vector<std::string> extractedAddresses;
    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        extractedAddresses.push_back(MessagingUtil::extractSingleEmailAddress(*it));
    }

    return extractedAddresses;
}

std::string MessagingUtil::loadFileContentToString(const std::string& file_path)
{
    std::ifstream input_file;
    input_file.open(file_path, std::ios::in);

    if (input_file.is_open()) {
        std::string outString;
        input_file.seekg(0, std::ios::end);
        outString.reserve(input_file.tellg());
        input_file.seekg(0, std::ios::beg);

        outString.assign((std::istreambuf_iterator<char>(input_file)),
                std::istreambuf_iterator<char>());
        input_file.close();
        return outString;
    } else {
        std::stringstream ss_error_msg;
        ss_error_msg << "Failed to open file: " << file_path;
        throw common::IOException(ss_error_msg.str().c_str());
    }
}

std::string MessagingUtil::messageStatusToString(MessageStatus status) {
    LOGD("Converting MessageStatus %d to string.", (int)status);
    switch(status) {
        case STATUS_SENT:
            return SENT;
        case STATUS_SENDING:
            return SENDING;
        case STATUS_FAILED:
            return FAILED;
        case STATUS_DRAFT:
            return DRAFT;
        default:
        // According to Web API documentation: If the status of the current
        // message does not correspond to any item from the list, an empty
        // value is returned.
            LOGD("Unsupported or undefined MessageStatus");
            return "";
    }
}

} // messaging
} // extension
