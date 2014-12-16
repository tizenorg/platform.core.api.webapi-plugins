// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "messaging_util.h"

#include <fstream>
#include <map>
#include <stdexcept>
#include <streambuf>
#include <sstream>
#include <cstdlib>

#include <email-api-account.h>
#include "message_email.h"

#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace messaging {

const char* JSON_ACTION = "action";
const char* JSON_CALLBACK_ID = "cid";
const char* JSON_CALLBACK_SUCCCESS = "success";
const char* JSON_CALLBACK_ERROR = "error";
const char* JSON_CALLBACK_PROGRESS = "progress";
const char* JSON_CALLBACK_KEEP = "keep";
const char* JSON_DATA = "args";

const char* JSON_RET_DATA = "data";
const char* JSON_RET_ERR_MESSAGE = "message";
const char* JSON_RET_ERR_NAME = "name";

const char* MESSAGE_ATTRIBUTE_ID = "id";
const char* MESSAGE_ATTRIBUTE_CONVERSATION_ID = "conversationId";
const char* MESSAGE_ATTRIBUTE_FOLDER_ID = "folderId";
const char* MESSAGE_ATTRIBUTE_TYPE = "type";
const char* MESSAGE_ATTRIBUTE_TIMESTAMP = "timestamp";
const char* MESSAGE_ATTRIBUTE_FROM = "from";
const char* MESSAGE_ATTRIBUTE_TO = "to"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_CC = "cc"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_BCC = "bcc"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_BODY = "body";
const char* MESSAGE_ATTRIBUTE_IS_READ = "isRead";
const char* MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY = "isHighPriority"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_SUBJECT = "subject"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_IN_RESPONSE_TO = "inResponseTo";
const char* MESSAGE_ATTRIBUTE_MESSAGE_STATUS = "messageStatus";
const char* MESSAGE_ATTRIBUTE_ATTACHMENTS = "attachments";
const char* MESSAGE_ATTRIBUTE_HAS_ATTACHMENT = "hasAttachment";

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

} // namespace

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

std::shared_ptr<Message> MessagingUtil::jsonToMessage(const picojson::value& json)
{
    LoggerD("Entered");
    std::shared_ptr<Message> message;
    picojson::object data = json.get<picojson::object>();
    std::string type = data.at("type").get<std::string>();

    MessageType mtype = MessagingUtil::stringToMessageType(type);

    switch (mtype) {
    case MessageType::SMS:
        LoggerD("Currently unsupported");
        // TODO add class which will extended message_service and call message_service_short_msg
        break;
    case MessageType::MMS:
        LoggerD("Currently unsupported");
        // TODO add class which will extended message_service and call message_service_short_msg
        break;
    case MessageType::EMAIL:

        if (!data.at(MESSAGE_ATTRIBUTE_ID).is<picojson::null>()) {
            std::string mid = data.at(MESSAGE_ATTRIBUTE_ID).get<std::string>();
            int mail_id = std::atoi(mid.c_str());
            email_mail_data_t* mail = NULL;
            if (EMAIL_ERROR_NONE != email_get_mail_data(mail_id, &mail)) {
                // TODO what should happen?
            } else {
                message = Message::convertPlatformEmailToObject(*mail);
                email_free_mail_data(&mail,1);
                return message;
            }
        } else {
            message = std::shared_ptr<Message>(new MessageEmail());
        }
        break;
    }

    std::vector<std::string> result;
    auto arrayVectorStringConverter = [&result] (picojson::value& v)->void {
        result.push_back(v.get<std::string>());
    };

    auto subject = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_ATTRIBUTE_SUBJECT);
    message->setSubject(subject);

    auto toJS = MessagingUtil::getValueFromJSONObject<std::vector<picojson::value>>(data,
            MESSAGE_ATTRIBUTE_TO);
    for_each(toJS.begin(), toJS.end(), arrayVectorStringConverter);
    message->setTO(result);
    result.clear();

    auto ccJS = MessagingUtil::getValueFromJSONObject<std::vector<picojson::value>>(data,
            MESSAGE_ATTRIBUTE_CC);
    for_each(ccJS.begin(), ccJS.end(), arrayVectorStringConverter);
    message->setCC(result);
    result.clear();

    auto bccJS = MessagingUtil::getValueFromJSONObject<std::vector<picojson::value>>(data,
            MESSAGE_ATTRIBUTE_BCC);
    for_each(bccJS.begin(), bccJS.end(), arrayVectorStringConverter);
    message->setBCC(result);
    result.clear();

    auto priority = MessagingUtil::getValueFromJSONObject<bool>(data,
            MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY);
    message->setIsHighPriority(priority);

    // TODO MessageBody

    return message;

}
} // messaging
} // extension
