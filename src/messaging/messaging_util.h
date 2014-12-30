// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_UTIL_H_
#define MESSAGING_MESSAGING_UTIL_H_

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "common/logger.h"
#include "common/picojson.h"

#include "MsgCommon/SortMode.h"
#include "MsgCommon/AttributeFilter.h"
#include "MsgCommon/AbstractFilter.h"

#include "message_folder.h"

namespace extension {
namespace messaging {

extern const char* JSON_ACTION;
extern const char* JSON_CALLBACK_ID;
extern const char* JSON_CALLBACK_SUCCCESS;
extern const char* JSON_CALLBACK_ERROR;
extern const char* JSON_CALLBACK_PROGRESS;
extern const char* JSON_CALLBACK_KEEP;
extern const char* JSON_DATA;
extern const char* JSON_DATA_MESSAGE;
extern const char* JSON_DATA_MESSAGE_BODY;
extern const char* JSON_ERROR_MESSAGE;
extern const char* JSON_ERROR_NAME;

extern const char* MESSAGE_ATTRIBUTE_ID;
extern const char* MESSAGE_ATTRIBUTE_CONVERSATION_ID;
extern const char* MESSAGE_ATTRIBUTE_FOLDER_ID;
extern const char* MESSAGE_ATTRIBUTE_TYPE;
extern const char* MESSAGE_ATTRIBUTE_TIMESTAMP;
extern const char* MESSAGE_ATTRIBUTE_FROM;
extern const char* MESSAGE_ATTRIBUTE_TO; // used also in dictionary
extern const char* MESSAGE_ATTRIBUTE_CC; // used also in dictionary
extern const char* MESSAGE_ATTRIBUTE_BCC; // used also in dictionary
extern const char* MESSAGE_ATTRIBUTE_BODY;
extern const char* MESSAGE_ATTRIBUTE_IS_READ;
extern const char* MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY; // used also in dictionary
extern const char* MESSAGE_ATTRIBUTE_SUBJECT; // used also in dictionary
extern const char* MESSAGE_ATTRIBUTE_IN_RESPONSE_TO;
extern const char* MESSAGE_ATTRIBUTE_MESSAGE_STATUS;
extern const char* MESSAGE_ATTRIBUTE_ATTACHMENTS;
extern const char* MESSAGE_ATTRIBUTE_HAS_ATTACHMENT;
extern const char* MESSAGE_ATTRIBUTE_MESSAGE_BODY;
extern const char* MESSAGE_ATTRIBUTE_MESSAGE_ATTACHMENT;

extern const char* MESSAGE_BODY_ATTRIBUTE_MESSAGE_ID;
extern const char* MESSAGE_BODY_ATTRIBUTE_LOADED;
extern const char* MESSAGE_BODY_ATTRIBUTE_PLAIN_BODY;
extern const char* MESSAGE_BODY_ATTRIBUTE_HTML_BODY;

extern const char* MESSAGE_ATTRIBUTE_MESSAGE_ATTACHMENT;
extern const char* MESSAGE_ATTACHMENT_ATTRIBUTE_ID;
extern const char* MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID;
extern const char* MESSAGE_ATTACHMENT_ATTRIBUTE_MIME_TYPE;
extern const char* MESSAGE_ATTACHMENT_ATTRIBUTE_FILE_PATH;

extern const char* MESSAGE_FOLDER_ATTRIBUTE_SERVICE_ID;

extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_ID;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_TYPE;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_TIMESTAMP;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_MESSAGE_COUNT;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_UNREAD_MESSAGES;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_PREVIEW;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_SUBJECT;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_IS_READ;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_FROM;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_TO;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_CC;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_BCC;
extern const char* MESSAGE_CONVERSATION_ATTRIBUTE_LAST_MESSAGE_ID;

enum MessageType {
    UNDEFINED = 0,
    SMS,
    MMS,
    EMAIL
};

enum MessageStatus {
    STATUS_UNDEFINED = 0,
    STATUS_DRAFT,
    STATUS_SENDING,
    STATUS_SENT,
    STATUS_LOADED,
    STATUS_FAILED
};

class Conversation;
class MessageConversation;
class Message;
class MessageBody;

class MessagingUtil {
public:
    static std::string messageFolderTypeToString(MessageFolderType);
    static MessageType stringToMessageType(std::string);
    static std::string messageTypeToString(MessageType);
    static std::string ltrim(const std::string& input);
    static std::string extractSingleEmailAddress(const std::string& address);
    static std::vector<std::string> extractEmailAddresses(
            const std::vector<std::string>& addresses);

    static picojson::value messageBodyToJson(std::shared_ptr<MessageBody> body);
    static picojson::value messageToJson(std::shared_ptr<Message> message);
    static picojson::value conversationToJson(std::shared_ptr<MessageConversation> conversation);
    static picojson::value folderToJson(std::shared_ptr<MessageFolder> folder);
    static std::shared_ptr<Message> jsonToMessage(const picojson::value& json);
    static tizen::SortModePtr jsonToSortMode(const picojson::object& json);
    static tizen::AttributeFilterPtr jsonToAttributeFilter(const picojson::object& json);
    static std::shared_ptr<MessageConversation> jsonToMessageConversation(const picojson::value& json);

    template <class T>
    static T getValueFromJSONObject(const picojson::object& v, const std::string& key)
    {
        picojson::value value;
        try{
            value = v.at(key);
        } catch(const std::out_of_range& e){
            return T();
        }

        if (value.is<T>()) {
            return value.get<T>();
        } else {
            return T();
        }
    }
    /**
    * Throws Common::IOException when file cannot be opened.
    *
    * To increase performance invoke this function this way:
    * std::string result = loadFileContentToString(...);
    * Reason: no copy constructor will be invoked on return.
    */
    static std::string loadFileContentToString(const std::string& file_path);
    static std::string messageStatusToString(MessageStatus status);
};

} // messaging
} // extension
#endif // MESSAGING_MESSAGING_UTIL_H_
