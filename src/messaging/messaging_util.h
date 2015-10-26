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

#ifndef MESSAGING_MESSAGING_UTIL_H_
#define MESSAGING_MESSAGING_UTIL_H_

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <stdexcept>
#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

#include "MsgCommon/SortMode.h"
#include "MsgCommon/AttributeFilter.h"
#include "MsgCommon/AbstractFilter.h"

#include "message_folder.h"
#include "message_attachment.h"
#include "common/platform_result.h"

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
extern const char* JSON_DATA_MESSAGE_ATTACHMENT;
extern const char* JSON_DATA_RECIPIENTS;
extern const char* JSON_ERROR_MESSAGE;
extern const char* JSON_ERROR_NAME;
extern const char* JSON_ERROR_CODE;

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
class MessagingInstance;

class MessagingUtil {
public:
    static std::string messageFolderTypeToString(MessageFolderType);
    static MessageFolderType stringToMessageFolderType(std::string type);
    static common::PlatformResult stringToMessageType(const std::string& str, MessageType* out);
    static common::PlatformResult messageTypeToString(MessageType type, std::string* out);
    static std::string messageTypeToString(MessageType type);
    static std::string ltrim(const std::string& input);
    static std::string extractSingleEmailAddress(const std::string& address);
    static std::vector<std::string> extractEmailAddresses(
            const std::vector<std::string>& addresses);

    static picojson::value messageBodyToJson(std::shared_ptr<MessageBody> body);
    static picojson::value messageToJson(std::shared_ptr<Message> message);
    static picojson::value messageAttachmentToJson(std::shared_ptr<MessageAttachment> attachment);
    static picojson::value conversationToJson(std::shared_ptr<MessageConversation> conversation);
    static picojson::value folderToJson(std::shared_ptr<MessageFolder> folder);
    static common::PlatformResult jsonToMessage(const picojson::value& json,
                                                std::shared_ptr<Message>* result);
    static std::shared_ptr<MessageBody> jsonToMessageBody(const picojson::value& json);
    static std::shared_ptr<MessageFolder> jsonToMessageFolder(const picojson::value& json);
    static tizen::SortModePtr jsonToSortMode(const picojson::object& json);
    static common::PlatformResult jsonToAbstractFilter(const picojson::object& json,
                                                       tizen::AbstractFilterPtr* result);
    static common::PlatformResult jsonToMessageConversation(const picojson::value& json,
                                      std::shared_ptr<MessageConversation>* result_conversation);
    static std::shared_ptr<MessageAttachment> jsonToMessageAttachment(const picojson::value& json);

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
    static common::PlatformResult loadFileContentToString(const std::string& file_path, std::string* result);
    static std::string messageStatusToString(MessageStatus status);

private:
    static common::PlatformResult jsonFilterToAbstractFilter(const picojson::object& json,
                                                         tizen::AbstractFilterPtr* result);
    static common::PlatformResult jsonFilterToAttributeFilter(const picojson::object& json,
                                                         tizen::AbstractFilterPtr* result);
    static common::PlatformResult jsonFilterToAttributeRangeFilter(const picojson::object& json,
                                                         tizen::AbstractFilterPtr* result);
    static common::PlatformResult jsonFilterToCompositeFilter(const picojson::object& json,
                                                         tizen::AbstractFilterPtr* result);

    static std::string ConvertToUtf8(const std::string& file_path, const std::string& contents);
};

enum PostPriority {
    LAST = 0,
    LOW,
    MEDIUM,
    HIGH
};

class PostQueue {
public:
    explicit PostQueue(MessagingInstance& instance);
    ~PostQueue();

    void addAndResolve(const long cid, PostPriority priority, const std::string &json);
    void add(const long cid, PostPriority priority = PostPriority::LAST);
    void resolve(const long cid, const std::string &json);

    enum TaskState {
        NEW = 0,
        READY,
        DONE
    };

private:
    class PostTask;
    typedef std::pair<long, std::shared_ptr<PostTask>> TasksCollectionItem;
    typedef std::vector<TasksCollectionItem> TasksCollection;

    PostQueue(const PostQueue &);
    void operator=(const PostQueue &);
    void resolve(PostPriority p);
    TasksCollection tasks_;
    std::mutex tasks_mutex_;

    MessagingInstance& instance_;

    class PostTask {
    public:
        PostTask();
        PostTask(PostPriority p);
        ~PostTask();
        void attach(const std::string &j);
        PostPriority priority();
        TaskState state();
        std::string json();
        void resolve();
    private:
        std::string json_;
        PostPriority priority_;
        TaskState state_;
    };
};

} // messaging
} // extension
#endif // MESSAGING_MESSAGING_UTIL_H_
