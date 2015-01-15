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
#include "message_sms.h"
#include "message_mms.h"
#include "message_conversation.h"

#include "tizen/tizen.h"
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
const char* JSON_DATA_MESSAGE = "message";
const char* JSON_DATA_MESSAGE_BODY = "messageBody";
const char* JSON_DATA_MESSAGE_ATTACHMENT = "messageAttachment";
const char* JSON_DATA_RECIPIENTS = "recipients";
const char* JSON_ERROR_MESSAGE = "message";
const char* JSON_ERROR_NAME = "name";

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
const char* MESSAGE_ATTRIBUTE_MESSAGE_BODY = "body";

const char* MESSAGE_BODY_ATTRIBUTE_MESSAGE_ID = "messageId";
const char* MESSAGE_BODY_ATTRIBUTE_LOADED = "loaded";
const char* MESSAGE_BODY_ATTRIBUTE_PLAIN_BODY = "plainBody";
const char* MESSAGE_BODY_ATTRIBUTE_HTML_BODY = "htmlBody";

const char* MESSAGE_ATTRIBUTE_MESSAGE_ATTACHMENTS = "attachments";
const char* MESSAGE_ATTACHMENT_ATTRIBUTE_ID = "id";
const char* MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID = "messageId";
const char* MESSAGE_ATTACHMENT_ATTRIBUTE_MIME_TYPE = "mimeType";
const char* MESSAGE_ATTACHMENT_ATTRIBUTE_FILE_PATH = "filePath";

const char* MESSAGE_FOLDER_ATTRIBUTE_ID = "id";
const char* MESSAGE_FOLDER_ATTRIBUTE_PARENT_ID = "parentId";
const char* MESSAGE_FOLDER_ATTRIBUTE_SERVICE_ID = "serviceId";
const char* MESSAGE_FOLDER_ATTRIBUTE_CONTENT_TYPE = "contentType";
const char* MESSAGE_FOLDER_ATTRIBUTE_NAME = "name";
const char* MESSAGE_FOLDER_ATTRIBUTE_PATH = "path";
const char* MESSAGE_FOLDER_ATTRIBUTE_TYPE = "type";
const char* MESSAGE_FOLDER_ATTRIBUTE_SYNCHRONIZABLE = "synchronizable";

const char* MESSAGE_CONVERSATION_ATTRIBUTE_ID = "id";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_TYPE = "type";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_TIMESTAMP = "timestamp";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_MESSAGE_COUNT = "messageCount";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_UNREAD_MESSAGES = "unreadMessages";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_PREVIEW = "preview";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_SUBJECT = "subject";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_IS_READ = "isRead";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_FROM = "from";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_TO = "to";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_CC = "cc";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_BCC = "bcc";
const char* MESSAGE_CONVERSATION_ATTRIBUTE_LAST_MESSAGE_ID = "lastMessageId";

namespace {
const std::string TYPE_SMS = "messaging.sms";
const std::string TYPE_MMS = "messaging.mms";
const std::string TYPE_EMAIL = "messaging.email";
const std::string SENT = "SENT";
const std::string SENDING = "SENDING";
const std::string FAILED = "FAILED";
const std::string DRAFT = "DRAFT";

const std::string JSON_TO_ATTRIBUTE_NAME = "attributeName";
const std::string JSON_TO_ORDER = "order";
const std::string JSON_TO_SORT = "sort";
const std::string JSON_TO_FILTER = "filter";
const std::string JSON_TO_MATCH_FLAG = "matchFlag";
const std::string JSON_TO_MATCH_VALUE = "matchValue";
const std::string JSON_TO_INITIAL_VALUE = "initialValue";
const std::string JSON_TO_END_VALUE = "endValue";

const char* JSON_FILTER_ATTRIBUTE_TYPE = "AttributeFilter";
const char* JSON_FILTER_ATTRIBUTERANGE_TYPE = "AttributeRangeFilter";
const char* JSON_FILTER_COMPOSITE_TYPE = "CompositeFilter";

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

const std::string FOLDER_TYPE_INBOX = "INBOX";
const std::string FOLDER_TYPE_OUTBOX = "OUTBOX";
const std::string FOLDER_TYPE_DRAFTS = "DRAFTS";
const std::string FOLDER_TYPE_SENTBOX = "SENTBOX";

} // namespace

std::string MessagingUtil::messageFolderTypeToString(MessageFolderType type)
{
    switch(type) {
        case MessageFolderType::MESSAGE_FOLDER_TYPE_INBOX:
            return FOLDER_TYPE_INBOX;
        case MessageFolderType::MESSAGE_FOLDER_TYPE_OUTBOX:
            return FOLDER_TYPE_OUTBOX;
        case MessageFolderType::MESSAGE_FOLDER_TYPE_DRAFTS:
            return FOLDER_TYPE_DRAFTS;
        case MessageFolderType::MESSAGE_FOLDER_TYPE_SENTBOX:
            return FOLDER_TYPE_SENTBOX;
        default:
            return "";
    }
}

MessageFolderType MessagingUtil::stringToMessageFolderType(std::string type)
{
    if (FOLDER_TYPE_INBOX == type) {
        return MessageFolderType::MESSAGE_FOLDER_TYPE_INBOX;
    }
    if (FOLDER_TYPE_OUTBOX == type) {
        return MessageFolderType::MESSAGE_FOLDER_TYPE_OUTBOX;
    }
    if (FOLDER_TYPE_DRAFTS == type) {
        return MessageFolderType::MESSAGE_FOLDER_TYPE_DRAFTS;
    }
    if (FOLDER_TYPE_SENTBOX == type) {
        return MessageFolderType::MESSAGE_FOLDER_TYPE_SENTBOX;
    }
    return MessageFolderType::MESSAGE_FOLDER_TYPE_NOTSTANDARD;
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
    LoggerD("Converting MessageStatus %d to string.", (int)status);
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
            LoggerD("Unsupported or undefined MessageStatus");
            return "";
    }
}

picojson::value MessagingUtil::messageBodyToJson(std::shared_ptr<MessageBody> body)
{
    picojson::object b;
    b[MESSAGE_BODY_ATTRIBUTE_MESSAGE_ID] = picojson::value(std::to_string(body->getMessageId()));
    b[MESSAGE_BODY_ATTRIBUTE_LOADED] = picojson::value(body->getLoaded());
    b[MESSAGE_BODY_ATTRIBUTE_PLAIN_BODY] = picojson::value(body->getPlainBody());
    b[MESSAGE_BODY_ATTRIBUTE_HTML_BODY] = picojson::value(body->getHtmlBody());
    picojson::value v(b);
    return v;
}

picojson::value MessagingUtil::messageToJson(std::shared_ptr<Message> message)
{
    picojson::object o;

    std::vector<picojson::value> array;
    auto vectorToArray = [&array] (std::string& s)->void {
        array.push_back(picojson::value(s));
    };

    switch (message->getType()) {
    case MessageType::SMS:
        break;
    case MessageType::MMS:
        o[MESSAGE_ATTRIBUTE_HAS_ATTACHMENT] = picojson::value(message->getHasAttachment());
        o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(message->getSubject());
        break;
    case MessageType::EMAIL:

        std::vector<std::string> cc = message->getCC();
        for_each(cc.begin(), cc.end(), vectorToArray);
        o[MESSAGE_ATTRIBUTE_CC] = picojson::value(array);
        array.clear();

        std::vector<std::string> bcc = message->getBCC();
        for_each(bcc.begin(), bcc.end(), vectorToArray);
        o[MESSAGE_ATTRIBUTE_BCC] = picojson::value(array);
        array.clear();

        o[MESSAGE_ATTRIBUTE_HAS_ATTACHMENT] = picojson::value(message->getHasAttachment());
        o[MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY] = picojson::value(message->getIsHighPriority());
        o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(message->getSubject());

        break;
    }

    o[MESSAGE_ATTRIBUTE_ID] =
            message->is_id_set()
            ? picojson::value(std::to_string(message->getId()))
            : picojson::value();
    o[MESSAGE_ATTRIBUTE_CONVERSATION_ID]=
            message->is_conversation_id_set()
            ? picojson::value(std::to_string(message->getConversationId()))
            : picojson::value();
    o[MESSAGE_ATTRIBUTE_FOLDER_ID] =
            message->is_folder_id_set()
            ? picojson::value(std::to_string(message->getFolderId()))
            : picojson::value();
    o[MESSAGE_ATTRIBUTE_TYPE] =
            picojson::value(MessagingUtil::messageTypeToString(message->getType()));
    o[MESSAGE_ATTRIBUTE_TIMESTAMP] =
            message->is_timestamp_set()
            ? picojson::value(static_cast<double>(message->getTimestamp()))
            : picojson::value();
    o[MESSAGE_ATTRIBUTE_FROM] =
            message->is_from_set()
            ? picojson::value(message->getFrom())
            : picojson::value();

    std::vector<std::string> to = message->getTO();
    for_each(to.begin(), to.end(), vectorToArray);
    o[MESSAGE_ATTRIBUTE_TO] = picojson::value(array);
    array.clear();

    o[MESSAGE_ATTRIBUTE_IS_READ] = picojson::value(message->getIsRead());
    o[MESSAGE_ATTRIBUTE_IN_RESPONSE_TO] =
            message->is_in_response_set()
            ? picojson::value(std::to_string(message->getInResponseTo()))
            : picojson::value();

    // TODO MessageStatus has type MessageStatus
    //o[MESSAGE_ATTRIBUTE_MESSAGE_STATUS] = picojson::value(message->getMessageStatus());

    std::shared_ptr<MessageBody> body = message->getBody();
    o[MESSAGE_ATTRIBUTE_BODY] = MessagingUtil::messageBodyToJson(body);


    auto vectorToAttachmentArray = [&array] (std::shared_ptr<MessageAttachment>& a)->void {
        array.push_back(MessagingUtil::messageAttachmentToJson(a));
    };
    auto attachments = message->getMessageAttachments();
    for_each(attachments.begin(), attachments.end(), vectorToAttachmentArray);
    o[MESSAGE_ATTRIBUTE_ATTACHMENTS] = picojson::value(array);
    array.clear();

    picojson::value v(o);
    return v;
}

picojson::value MessagingUtil::conversationToJson(std::shared_ptr<MessageConversation> conversation)
{
    picojson::object o;

    o[MESSAGE_CONVERSATION_ATTRIBUTE_ID] = picojson::value(std::to_string(conversation->getConversationId()));

    o[MESSAGE_CONVERSATION_ATTRIBUTE_TYPE] =
            picojson::value(MessagingUtil::messageTypeToString(conversation->getType()));

    o[MESSAGE_CONVERSATION_ATTRIBUTE_TIMESTAMP] =
            picojson::value(static_cast<double>(conversation->getTimestamp()));

    o[MESSAGE_CONVERSATION_ATTRIBUTE_MESSAGE_COUNT] =
            picojson::value(static_cast<double>(conversation->getMessageCount()));

    o[MESSAGE_CONVERSATION_ATTRIBUTE_UNREAD_MESSAGES] =
            picojson::value(static_cast<double>(conversation->getUnreadMessages()));

    o[MESSAGE_CONVERSATION_ATTRIBUTE_PREVIEW] =
            picojson::value(conversation->getPreview());

    o[MESSAGE_CONVERSATION_ATTRIBUTE_IS_READ] =
            picojson::value(conversation->getIsRead());

    o[MESSAGE_CONVERSATION_ATTRIBUTE_FROM] =
            picojson::value(conversation->getFrom());

    o[MESSAGE_CONVERSATION_ATTRIBUTE_LAST_MESSAGE_ID] =
            picojson::value(std::to_string(conversation->getLastMessageId()));

    std::vector<picojson::value> array;
    auto vectorToArray = [&array] (std::string& s)->void {
        array.push_back(picojson::value(s));
    };

    switch (conversation->getType()) {
        case MessageType::SMS:
            break;
        case MessageType::MMS:
            o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(conversation->getSubject());
            break;
        case MessageType::EMAIL:
            o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(conversation->getSubject());

            std::vector<std::string> to = conversation->getTo();
            for_each(to.begin(), to.end(), vectorToArray);
            o[MESSAGE_ATTRIBUTE_TO] = picojson::value(array);
            array.clear();

            std::vector<std::string> cc = conversation->getCC();
            for_each(cc.begin(), cc.end(), vectorToArray);
            o[MESSAGE_ATTRIBUTE_CC] = picojson::value(array);
            array.clear();

            std::vector<std::string> bcc = conversation->getBCC();
            for_each(bcc.begin(), bcc.end(), vectorToArray);
            o[MESSAGE_ATTRIBUTE_BCC] = picojson::value(array);
            array.clear();

            break;
        }

    picojson::value v(o);
    return v;
}

picojson::value MessagingUtil::folderToJson(std::shared_ptr<MessageFolder> folder)
{
    LoggerD("Entered");

    picojson::object o;

    o[MESSAGE_FOLDER_ATTRIBUTE_ID] = picojson::value(folder->getId());
    o[MESSAGE_FOLDER_ATTRIBUTE_PARENT_ID] =
            folder->isParentIdSet()
            ? picojson::value(folder->getParentId())
            : picojson::value();
    o[MESSAGE_FOLDER_ATTRIBUTE_SERVICE_ID] =  picojson::value(folder->getServiceId());
    o[MESSAGE_FOLDER_ATTRIBUTE_CONTENT_TYPE] = picojson::value(folder->getContentType());
    o[MESSAGE_FOLDER_ATTRIBUTE_NAME] = picojson::value(folder->getName());
    o[MESSAGE_FOLDER_ATTRIBUTE_PATH] = picojson::value(folder->getPath());
    o[MESSAGE_FOLDER_ATTRIBUTE_TYPE] =
            picojson::value(MessagingUtil::messageFolderTypeToString(folder->getType()));
    o[MESSAGE_FOLDER_ATTRIBUTE_SYNCHRONIZABLE] = picojson::value(folder->getSynchronizable());

    picojson::value v(o);
    return v;
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
        LoggerD("SMS type");
        if(data.at(MESSAGE_ATTRIBUTE_ID).is<picojson::null>()) {
            message = std::shared_ptr<Message>(new MessageSMS());
            break;
        }
    case MessageType::MMS:
        LoggerD("MMS type");
        if(data.at(MESSAGE_ATTRIBUTE_ID).is<picojson::null>()) {
            message = std::shared_ptr<Message>(new MessageMMS());
        } else {
            std::string mid = data.at(MESSAGE_ATTRIBUTE_ID).get<std::string>();
            int message_id = std::atoi(mid.c_str());
            message = Message::findShortMessageById(message_id);
        }
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


    auto ccJS = MessagingUtil::getValueFromJSONObject<
            std::vector<picojson::value>>(data, MESSAGE_ATTRIBUTE_CC);
    for_each(ccJS.begin(), ccJS.end(), arrayVectorStringConverter);
    message->setCC(result);
    result.clear();

    auto bccJS = MessagingUtil::getValueFromJSONObject<
            std::vector<picojson::value>>(data, MESSAGE_ATTRIBUTE_BCC);
    for_each(bccJS.begin(), bccJS.end(), arrayVectorStringConverter);
    message->setBCC(result);
    result.clear();

    auto priority = MessagingUtil::getValueFromJSONObject<bool>(data,
            MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY);
    message->setIsHighPriority(priority);

    std::shared_ptr<MessageBody> body = MessagingUtil::jsonToMessageBody(
            data[MESSAGE_ATTRIBUTE_MESSAGE_BODY]);
    message->setBody(body);

    AttachmentPtrVector attachments;
    auto ma = data.at(MESSAGE_ATTRIBUTE_MESSAGE_ATTACHMENTS).get<std::vector<picojson::value>>();

    auto arrayVectorAttachmentConverter = [&attachments] (picojson::value& v)->void
    {
        std::shared_ptr<MessageAttachment> attachment =
                std::shared_ptr<MessageAttachment>(new MessageAttachment());

        auto obj = v.get<picojson::object>();
        int messageAttachmentId = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(obj,
                MESSAGE_ATTACHMENT_ATTRIBUTE_ID).c_str());
        attachment->setId(messageAttachmentId);

        int messageId = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(obj,
                MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID).c_str());
        attachment->setMessageId(messageId);

        std::string mimeType = MessagingUtil::getValueFromJSONObject<std::string>(obj,
                MESSAGE_ATTACHMENT_ATTRIBUTE_MIME_TYPE);
        attachment->setMimeType(mimeType);

        std::string filePath = MessagingUtil::getValueFromJSONObject<std::string>(obj,
                MESSAGE_ATTACHMENT_ATTRIBUTE_FILE_PATH);
        attachment->setFilePath(filePath);

        attachments.push_back(attachment);
    };

    for_each(ma.begin(), ma.end(), arrayVectorAttachmentConverter);
    message->setMessageAttachments(attachments);

    return message;

}

std::shared_ptr<MessageBody> MessagingUtil::jsonToMessageBody(const picojson::value& json)
{
    LoggerD("Entered");

    std::shared_ptr<MessageBody> body = std::shared_ptr<MessageBody>(new MessageBody());
    picojson::object data = json.get<picojson::object>();

    bool loaded = MessagingUtil::getValueFromJSONObject<bool>(data,
            MESSAGE_BODY_ATTRIBUTE_LOADED);
    body->setLoaded(loaded);

    std::string html = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_BODY_ATTRIBUTE_HTML_BODY);
    body->setHtmlBody(html);

    std::string plain = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_BODY_ATTRIBUTE_PLAIN_BODY);
    body->setPlainBody(plain);

    if (!data.at(MESSAGE_BODY_ATTRIBUTE_MESSAGE_ID).is<picojson::null>()) {
        int messageId = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(data,
                    MESSAGE_BODY_ATTRIBUTE_MESSAGE_ID).c_str());
        body->setMessageId(messageId);
    }

    return body;
}

std::shared_ptr<MessageFolder> MessagingUtil::jsonToMessageFolder(const picojson::value& json)
{
    LoggerD("Entered");

    picojson::object data = json.get<picojson::object>();

    std::string id = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_ID).c_str();

    std::string parent_id = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_PARENT_ID).c_str();

    std::string service_id = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_SERVICE_ID).c_str();

    std::string content_type = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_CONTENT_TYPE).c_str();

    std::string name = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_NAME).c_str();

    std::string path = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_PATH).c_str();

    std::string type_str = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_FOLDER_ATTRIBUTE_TYPE).c_str();
    MessageFolderType type = MessagingUtil::stringToMessageFolderType(type_str);

    bool synchronizable = MessagingUtil::getValueFromJSONObject<bool>(data,
            MESSAGE_FOLDER_ATTRIBUTE_SYNCHRONIZABLE);

    std::shared_ptr<MessageFolder> folder = std::shared_ptr<MessageFolder>(
            new MessageFolder(
                    id,
                    parent_id,
                    service_id,
                    content_type,
                    name,
                    path,
                    type,
                    synchronizable));

    return folder;
}

tizen::SortModePtr MessagingUtil::jsonToSortMode(const picojson::object& json)
{
    LoggerD("Entered");
    using namespace tizen;

    try{
        if (json.at(JSON_TO_SORT).is<picojson::null>()) {
            return SortModePtr();
        }
    } catch(const std::out_of_range& e){
        return SortModePtr();
    }

    auto dataSort = getValueFromJSONObject<picojson::object>(json, JSON_TO_SORT);
    auto name = getValueFromJSONObject<std::string>(dataSort, JSON_TO_ATTRIBUTE_NAME);
    auto ord = getValueFromJSONObject<std::string>(dataSort, JSON_TO_ORDER);
    SortModeOrder order = ( ord == STR_SORT_DESC) ? SortModeOrder::DESC : SortModeOrder::ASC;
    return SortModePtr(new SortMode(name, order));
}

tizen::AbstractFilterPtr MessagingUtil::jsonToAbstractFilter(const picojson::object& json)
{
    LoggerD("Entered");

    if (json.at(JSON_TO_FILTER).is<picojson::null>()) {
        return AbstractFilterPtr();
    }

    auto filter = getValueFromJSONObject<picojson::object>(json, JSON_TO_FILTER);
    std::string type = getValueFromJSONObject<std::string>(filter, "type");

    if( JSON_FILTER_ATTRIBUTE_TYPE == type ){
        return jsonFilterToAttributeFilter(filter);
    }
    if( JSON_FILTER_ATTRIBUTERANGE_TYPE == type ){
        return jsonFilterToAttributeRangeFilter(filter);
    }
    if( JSON_FILTER_COMPOSITE_TYPE == type ) {
        //TODO jsonToCompositeFilter
        LoggerD("Composite filter currently not supported");
    }

    LoggerE("Unsupported filter type");
    throw common::TypeMismatchException("Unsupported filter type");
    return AbstractFilterPtr();
}

tizen::AttributeFilterPtr MessagingUtil::jsonFilterToAttributeFilter(const picojson::object& filter)
{
    LoggerD("Entered");

    using namespace tizen;

    auto name = getValueFromJSONObject<std::string>(filter, JSON_TO_ATTRIBUTE_NAME);
    auto matchFlagStr = getValueFromJSONObject<std::string>(filter, JSON_TO_MATCH_FLAG);

    FilterMatchFlag filterMatch;

    if (STR_MATCH_EXACTLY == matchFlagStr) {
        filterMatch = FilterMatchFlag::EXACTLY;
    }
    else if (STR_MATCH_FULLSTRING == matchFlagStr) {
        filterMatch = FilterMatchFlag::FULLSTRING;
    }
    else if (STR_MATCH_CONTAINS == matchFlagStr) {
        filterMatch = FilterMatchFlag::CONTAINS;
    }
    else if (STR_MATCH_STARTSWITH == matchFlagStr) {
        filterMatch = FilterMatchFlag::STARTSWITH;
    }
    else if (STR_MATCH_ENDSWITH == matchFlagStr) {
        filterMatch = FilterMatchFlag::ENDSWITH;
    }
    else if (STR_MATCH_EXISTS == matchFlagStr) {
        filterMatch = FilterMatchFlag::EXISTS;
    }
    else {
        LoggerE("Filter name is not recognized: %s", matchFlagStr.c_str());
        throw common::TypeMismatchException("Filter name is not recognized");
    }

    auto attributePtr = AttributeFilterPtr(new AttributeFilter(name));
    attributePtr->setMatchFlag(filterMatch);
    attributePtr->setMatchValue(AnyPtr(new Any(filter.at(JSON_TO_MATCH_VALUE))));
    return attributePtr;
}

tizen::AttributeRangeFilterPtr MessagingUtil::jsonFilterToAttributeRangeFilter(const picojson::object& filter)
{
    LoggerD("Entered");

    auto name = getValueFromJSONObject<std::string>(filter, JSON_TO_ATTRIBUTE_NAME);

    auto attributeRangePtr = tizen::AttributeRangeFilterPtr(new tizen::AttributeRangeFilter(name));
    attributeRangePtr->setInitialValue(AnyPtr(new Any(filter.at(JSON_TO_INITIAL_VALUE))));
    attributeRangePtr->setEndValue(AnyPtr(new Any(filter.at(JSON_TO_END_VALUE))));

    return  attributeRangePtr;
}

std::shared_ptr<MessageAttachment> MessagingUtil::jsonToMessageAttachment(const picojson::value& json)
{
    LoggerD("Entered");

    picojson::object data = json.get<picojson::object>();
    int attachmentId =
            static_cast<int>(getValueFromJSONObject<double>(data, MESSAGE_ATTACHMENT_ATTRIBUTE_ID));
    int messageId = static_cast<int>(
            getValueFromJSONObject<double>(data, MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID));
    std::string mimeType =
            getValueFromJSONObject<std::string>(data, MESSAGE_ATTACHMENT_ATTRIBUTE_MIME_TYPE);
    std::string filePath =
            getValueFromJSONObject<std::string>(data, MESSAGE_ATTACHMENT_ATTRIBUTE_FILE_PATH);
    auto attachmentPtr = std::shared_ptr<MessageAttachment>(new MessageAttachment());

    attachmentPtr->setId(attachmentId);
    attachmentPtr->setMessageId(messageId);
    attachmentPtr->setMimeType(mimeType);
    attachmentPtr->setFilePath(filePath);

    return attachmentPtr;
}

picojson::value MessagingUtil::messageAttachmentToJson(std::shared_ptr<MessageAttachment> attachment)
{
    LoggerD("Entered");

    picojson::object o;
    o[MESSAGE_ATTACHMENT_ATTRIBUTE_ID] =
            attachment->isIdSet()
            ? picojson::value(static_cast<double>(attachment->getId()))
            : picojson::value();

    o[MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID] =
            attachment->isMessageIdSet()
            ? picojson::value(static_cast<double>(attachment->getMessageId()))
            : picojson::value();

    o[MESSAGE_ATTACHMENT_ATTRIBUTE_MIME_TYPE] =
            attachment->isMimeTypeSet()
            ? picojson::value(attachment->getMimeType())
            : picojson::value();

    o[MESSAGE_ATTACHMENT_ATTRIBUTE_FILE_PATH] =
            attachment->isFilePathSet()
            ? picojson::value(attachment->getFilePath())
            : picojson::value();

    return picojson::value(o);
}

std::shared_ptr<MessageConversation> MessagingUtil::jsonToMessageConversation(const picojson::value& json)
{
    LoggerD("Entered");
    std::shared_ptr<MessageConversation> conversation;
    picojson::object data = json.get<picojson::object>();
    std::string type = data.at("type").get<std::string>();
    MessageType mtype = MessagingUtil::stringToMessageType(type);

    conversation = std::shared_ptr<MessageConversation>(new MessageConversation());

    conversation->setType(mtype);

    int id = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_ID).c_str());
    conversation->setConversationId(id);

    /// MESSAGE_CONVERSATION_ATTRIBUTE_TIMESTAMP ?

    int messageCount = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_MESSAGE_COUNT).c_str());
    conversation->setMessageCount(messageCount);

    int unreadMessages = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_UNREAD_MESSAGES).c_str());
    conversation->setUnreadMessages(unreadMessages);

    auto preview = MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_PREVIEW).c_str();
    conversation->setPreview(preview);

    auto subject = MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_SUBJECT).c_str();
    conversation->setSubject(subject);

    /// MESSAGE_CONVERSATION_ATTRIBUTE_IS_READ ?

    std::vector<std::string> result;
    auto arrayVectorStringConverter = [&result] (picojson::value& v)->void {
        result.push_back(v.get<std::string>());
    };

    auto from = MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_FROM).c_str();
    conversation->setFrom(from);

    auto toJS = MessagingUtil::getValueFromJSONObject<std::vector<picojson::value>>(
        data, MESSAGE_CONVERSATION_ATTRIBUTE_TO);
    for_each(toJS.begin(), toJS.end(), arrayVectorStringConverter);
    conversation->setTo(result);
    result.clear();

    auto ccJS = MessagingUtil::getValueFromJSONObject<
            std::vector<picojson::value>>(data, MESSAGE_ATTRIBUTE_CC);
    for_each(ccJS.begin(), ccJS.end(), arrayVectorStringConverter);
    conversation->setCC(result);
    result.clear();

    auto bccJS = MessagingUtil::getValueFromJSONObject<
            std::vector<picojson::value>>(data, MESSAGE_ATTRIBUTE_BCC);
    for_each(bccJS.begin(), bccJS.end(), arrayVectorStringConverter);
    conversation->setBCC(result);
    result.clear();

    int lastMessageId = std::atoi(MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_LAST_MESSAGE_ID).c_str());
    conversation->setLastMessageId(lastMessageId);

    return conversation;
}

} // messaging
} // extension
