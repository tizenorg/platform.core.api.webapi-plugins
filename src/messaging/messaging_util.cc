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
 
#include "messaging_util.h"

#include <glib.h>

#include <fstream>
#include <stdexcept>
#include <streambuf>
#include <sstream>
#include <cstdlib>

#include <email-api-account.h>
#include "message_email.h"
#include "message_sms.h"
#include "message_mms.h"
#include "message_conversation.h"
#include "messaging_instance.h"
#include "messaging/email_manager.h"

#include "tizen/tizen.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/scope_exit.h"
#include "common/assert.h"

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {
using namespace common;

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
const char* JSON_ERROR_CODE = "code";

const char* MESSAGE_ATTRIBUTE_ID = "id";
const char* MESSAGE_ATTRIBUTE_OLD_ID = "oldId";
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
const char* MESSAGE_BODY_ATTRIBUTE_INLINE_ATTACHMENTS = "inlineAttachments";

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
const std::string JSON_TO_TYPE = "type";
const std::string JSON_TO_FILTER_ARRAY = "filters";

const char* JSON_FILTER_TYPE = "filterType";
const char* JSON_FILTER_ATTRIBUTE_TYPE = "AttributeFilter";
const char* JSON_FILTER_ATTRIBUTE_RANGE_TYPE = "AttributeRangeFilter";
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
    LoggerD("Entered");
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
    LoggerD("Entered");
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

PlatformResult MessagingUtil::stringToMessageType(const std::string& str, MessageType* out)
{
  LoggerD("Entered");
  const auto it = stringToTypeMap.find(str);

  if (it == stringToTypeMap.end()) {
    LoggerE("Not supported type: %s", str.c_str());
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Not supported type: " + str);
  } else {
    *out = it->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }
}

common::PlatformResult MessagingUtil::messageTypeToString(MessageType type, std::string* out)
{
  LoggerD("Entered");
  const auto it = typeToStringMap.find(type);

  if (it == typeToStringMap.end()) {
    LoggerE("Invalid MessageType: %d", type);
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Invalid MessageType");
  } else {
    *out = it->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }
}

std::string MessagingUtil::messageTypeToString(MessageType type) {
  LoggerD("Entered");
  std::string type_str;
  PlatformResult platform_result = messageTypeToString(type, &type_str);
  Assert(platform_result);
  return type_str;
}

std::string MessagingUtil::ltrim(const std::string& input)
{
    LoggerD("Entered");
    std::string str = input;
    std::string::iterator i;
    for (i = str.begin(); i != str.end(); ++i) {
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
    LoggerD("Entered");
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
    LoggerD("Entered");
    std::vector<std::string> extractedAddresses;
    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        extractedAddresses.push_back(MessagingUtil::extractSingleEmailAddress(*it));
    }

    return extractedAddresses;
}

PlatformResult MessagingUtil::loadFileContentToString(const std::string& file_path, std::string* result)
{
    LoggerD("Entered");
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
        *result = ConvertToUtf8(file_path, outString);
    } else {
        std::stringstream ss_error_msg;
        ss_error_msg << "Failed to open file: " << file_path;
        return PlatformResult(ErrorCode::IO_ERR, ss_error_msg.str().c_str());
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

namespace {

std::string GetFilename(const std::string& file_path) {
  LoggerD("Entered");
  const auto start = file_path.find_last_of("/\\");
  const auto basename = file_path.substr(std::string::npos == start ? 0 : start + 1);
  return basename.substr(0, basename.find_last_of("."));
}

std::string PerformConversion(const std::string& input, const gchar* from_charset) {
  LoggerD("Entered");

  GIConv cd = g_iconv_open("UTF-8//IGNORE", from_charset);

  if ((GIConv)-1 == cd) {
    LoggerE("Failed to open iconv.");
    return input;
  }

  SCOPE_EXIT {
    g_iconv_close(cd);
  };

  // copied from glib/gconvert.c, g_convert does not handle "//IGNORE" properly
  static const gsize kNulTerminatorLength = 4;
  const gchar* str = input.c_str();
  gssize len = input.size();

  gchar* p = const_cast<gchar*>(str);
  gsize inbytes_remaining = len;
  gsize outbuf_size = len + kNulTerminatorLength;
  gsize outbytes_remaining = outbuf_size - kNulTerminatorLength;
  gchar* dest = nullptr;
  gchar* outp = nullptr;
  gboolean have_error = FALSE;
  gboolean done = FALSE;
  gboolean reset = FALSE;

  outp = dest = static_cast<gchar*>(g_malloc(outbuf_size));

  if (!dest) {
    LoggerE("Failed to allocate memory.");
    return input;
  }

  SCOPE_EXIT {
    g_free(dest);
  };

  while (!done && !have_error) {
    gsize err = 0;

    if (reset) {
      err = g_iconv(cd, nullptr, &inbytes_remaining, &outp, &outbytes_remaining);
    } else {
      err = g_iconv(cd, &p, &inbytes_remaining, &outp, &outbytes_remaining);
    }

    if (static_cast<gsize>(-1) == err) {
      switch (errno) {
        case EINVAL:
          LoggerD("EINVAL");
          // Incomplete text, do not report an error
          done = TRUE;
          break;

        case E2BIG:
          {
            LoggerD("E2BIG");
            gsize used = outp - dest;

            outbuf_size *= 2;
            dest = static_cast<gchar*>(g_realloc(dest, outbuf_size));

            outp = dest + used;
            outbytes_remaining = outbuf_size - used - kNulTerminatorLength;
          }
          break;

        case EILSEQ:
          if (0 == inbytes_remaining) {
            LoggerD("EILSEQ reported, but whole input buffer was processed, assuming it's OK");
          } else {
            LoggerE("EILSEQ");
            have_error = TRUE;
          }
          break;

        default:
          LoggerE("Conversion error: %d", errno);
          have_error = TRUE;
          break;
      }
    } else {
      if (!reset) {
        // call g_iconv with NULL inbuf to cleanup shift state
        reset = TRUE;
        inbytes_remaining = 0;
      } else {
        done = TRUE;
      }
    }
  }

  memset(outp, 0, kNulTerminatorLength);

  if ((p - str) != len) {
    LoggerE("Partial character sequence at end of input");
    have_error = TRUE;
  }

  std::string result;

  if (!have_error) {
    result = dest;
  } else {
    LoggerE("Conversion error");
  }

  return result;
}

}  // namespace

std::string MessagingUtil::ConvertToUtf8(const std::string& file_path, const std::string& contents) {
  LoggerD("Entered");

  // in case of messages, encoding of the file contents is stored as its filename
  // is case of draft messages, it is not...
  std::string encoding = GetFilename(file_path);

  LoggerD("encoding: %s", encoding.c_str());

  // implementation taken from apps/home/email.git,
  // file Project-Files/common/src/email-utils.c

  gchar* from_charset = g_ascii_strup(encoding.c_str(), -1);

  if (0 == g_ascii_strcasecmp(from_charset, "KS_C_5601-1987")) {
    // "ks_c_5601-1987" is not an encoding name. It's just a charset.
    // There's no code page on IANA for "ks_c_5601-1987".
    // So we should convert this to encoding name "EUC-KR"
    // CP949 is super set of EUC-KR, we use CP949 first
    LoggerD("change: KS_C_5601-1987 ===> CP949");
    g_free(from_charset);
    from_charset = g_strdup("CP949");
  } else if (0 == g_ascii_strcasecmp(from_charset, "ISO-2022-JP")) {
    // iso-2022-jp-2 is a superset of iso-2022-jp. In some email,
    // iso-2022-jp is not converted to utf8 correctly. So in this case,
    // we use iso-2022-jp-2 instead.
    LoggerD("change: ISO-2022-JP ===> ISO-2022-JP-2");
    g_free(from_charset);
    from_charset = g_strdup("ISO-2022-JP-2");
  }

  std::string output;

  // if charset is unknown, conversion is not needed
  if ((0 != g_ascii_strcasecmp(from_charset, UNKNOWN_CHARSET_PLAIN_TEXT_FILE))) {
    // we're performing UTF-8 to UTF-8 conversion to remove malformed data
    LoggerD("performing conversion");

    output = PerformConversion(contents, from_charset);

    if ("" == output && 0 == g_ascii_strcasecmp(from_charset, "CP949")) {
      LoggerD("change: CP949 ===> EUC-KR, try again");
      output = PerformConversion(contents, "EUC-KR");
    }

    if ("" == output) {
      LoggerE("Conversion failed");
      // conversion failed, use original contents
      output = contents;
    }
  } else {
    // no conversion
    output = contents;
  }

  g_free(from_charset);

  return output;
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
    LoggerD("Entered");
    picojson::object b;
    b[MESSAGE_BODY_ATTRIBUTE_MESSAGE_ID] = picojson::value(std::to_string(body->getMessageId()));
    b[MESSAGE_BODY_ATTRIBUTE_LOADED] = picojson::value(body->getLoaded());
    b[MESSAGE_BODY_ATTRIBUTE_PLAIN_BODY] = picojson::value(body->getPlainBody());
    b[MESSAGE_BODY_ATTRIBUTE_HTML_BODY] = picojson::value(body->getHtmlBody());

    std::vector<picojson::value> array;

    auto vectorToAttachmentArray = [&array] (std::shared_ptr<MessageAttachment>& a)->void {
        array.push_back(MessagingUtil::messageAttachmentToJson(a));
    };
    auto inlineAttachments = body->getInlineAttachments();
    for_each(inlineAttachments.begin(), inlineAttachments.end(), vectorToAttachmentArray);

    b[MESSAGE_BODY_ATTRIBUTE_INLINE_ATTACHMENTS] = picojson::value(array);
    array.clear();

    picojson::value v(b);
    return v;
}

picojson::value MessagingUtil::messageToJson(std::shared_ptr<Message> message)
{
    LoggerD("Entered");
    picojson::object o;

    std::vector<picojson::value> array;
    std::vector<std::string> bcc, cc;
    auto vectorToArray = [&array] (std::string& s)->void {
        array.push_back(picojson::value(s));
    };

    switch (message->getType()) {
    case MessageType::SMS:
        break;
    case MessageType::MMS:
        o[MESSAGE_ATTRIBUTE_HAS_ATTACHMENT] = picojson::value(message->getHasAttachment());
        o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(message->getSubject());
        o[MESSAGE_ATTRIBUTE_OLD_ID] = picojson::value(std::to_string(message->getOldId()));
        break;
    case MessageType::EMAIL:

        cc = message->getCC();
        for_each(cc.begin(), cc.end(), vectorToArray);
        o[MESSAGE_ATTRIBUTE_CC] = picojson::value(array);
        array.clear();

        bcc = message->getBCC();
        for_each(bcc.begin(), bcc.end(), vectorToArray);
        o[MESSAGE_ATTRIBUTE_BCC] = picojson::value(array);
        array.clear();

        o[MESSAGE_ATTRIBUTE_HAS_ATTACHMENT] = picojson::value(message->getHasAttachment());
        o[MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY] = picojson::value(message->getIsHighPriority());
        o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(message->getSubject());
        o[MESSAGE_ATTRIBUTE_OLD_ID] = picojson::value(std::to_string(message->getOldId()));

        break;
    default:
        LoggerW("Unsupported message type");
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
    o[MESSAGE_ATTRIBUTE_TYPE] = picojson::value(message->getTypeString());
    o[MESSAGE_ATTRIBUTE_TIMESTAMP] =
            message->is_timestamp_set()
            ? picojson::value(static_cast<double>(message->getTimestamp()))
            : picojson::value();
    o[MESSAGE_ATTRIBUTE_FROM] =
            message->is_from_set()
            ? picojson::value(message->getFrom())
            : picojson::value(std::string(""));

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
    LoggerD("Entered");
    picojson::object o;

    o[MESSAGE_CONVERSATION_ATTRIBUTE_ID] = picojson::value(std::to_string(conversation->getConversationId()));

    o[MESSAGE_CONVERSATION_ATTRIBUTE_TYPE] = picojson::value(conversation->getTypeString());

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

    std::vector<std::string> to = conversation->getTo();
    for_each(to.begin(), to.end(), vectorToArray);
    o[MESSAGE_ATTRIBUTE_TO] = picojson::value(array);
    array.clear();
    std::vector<std::string> cc, bcc;

    switch (conversation->getType()) {
        case MessageType::SMS:
            break;
        case MessageType::MMS:
            o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(conversation->getSubject());
            break;
        case MessageType::EMAIL:
            o[MESSAGE_ATTRIBUTE_SUBJECT] = picojson::value(conversation->getSubject());

            cc = conversation->getCC();
            for_each(cc.begin(), cc.end(), vectorToArray);
            o[MESSAGE_ATTRIBUTE_CC] = picojson::value(array);
            array.clear();

            bcc = conversation->getBCC();
            for_each(bcc.begin(), bcc.end(), vectorToArray);
            o[MESSAGE_ATTRIBUTE_BCC] = picojson::value(array);
            array.clear();

            break;
        default:
            LoggerW("Unsupported message type");
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

PlatformResult MessagingUtil::jsonToMessage(const picojson::value& json,
                                            std::shared_ptr<Message>* result_message)
{
    LoggerD("Entered");
    std::shared_ptr<Message> message;
    picojson::object data = json.get<picojson::object>();
    std::string type = data.at("type").get<std::string>();
    MessageType mtype = UNDEFINED;
    auto platform_result = MessagingUtil::stringToMessageType(type, &mtype);

    if (!platform_result) {
      return platform_result;
    }

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
            platform_result = Message::findShortMessageById(message_id, &message);
            if (!platform_result) return platform_result;
        }
        break;
    case MessageType::EMAIL:
        if (!data.at(MESSAGE_ATTRIBUTE_ID).is<picojson::null>()) {
            std::string mid = data.at(MESSAGE_ATTRIBUTE_ID).get<std::string>();
            int mail_id = std::atoi(mid.c_str());
            email_mail_data_t* mail = NULL;
            if (EMAIL_ERROR_NONE != email_get_mail_data(mail_id, &mail)) {
                // TODO what should happen?
                LoggerE("Fatal error: message not found: %d!", mail_id);
                return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR,
                                      "Failed to find specified email.");
            } else {
                platform_result = Message::convertPlatformEmailToObject(*mail, &message);
                email_free_mail_data(&mail,1);
                if (!platform_result) return platform_result;
            }
        } else {
            message = std::shared_ptr<Message>(new MessageEmail());
        }
        break;
    default:
        LoggerE("Not supported message type");
        break;
    }

    std::vector<std::string> result;
    PlatformResult conv_res(ErrorCode::NO_ERROR);
    auto arrayVectorStringConverter = [&result, &conv_res] (picojson::value& v)->void {
      if (!v.is<std::string>()) {
        const std::string message = "Passed array holds incorrect values "
            + v.serialize() + " is not a correct string value";
        LoggerE("Error: %s", message.c_str());
        conv_res = PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
      }
      if (conv_res.IsError()) {
        return;
      }
      result.push_back(v.get<std::string>());
    };

    auto subject = MessagingUtil::getValueFromJSONObject<std::string>(data,
            MESSAGE_ATTRIBUTE_SUBJECT);
    message->setSubject(subject);

    auto toJS = MessagingUtil::getValueFromJSONObject<std::vector<picojson::value>>(data,
            MESSAGE_ATTRIBUTE_TO);
    for_each(toJS.begin(), toJS.end(), arrayVectorStringConverter);
    if (conv_res.IsError()) {
      return conv_res;
    }
    message->setTO(result);
    result.clear();


    auto ccJS = MessagingUtil::getValueFromJSONObject<
            std::vector<picojson::value>>(data, MESSAGE_ATTRIBUTE_CC);
    for_each(ccJS.begin(), ccJS.end(), arrayVectorStringConverter);
    if (conv_res.IsError()) {
      return conv_res;
    }
    message->setCC(result);
    result.clear();

    auto bccJS = MessagingUtil::getValueFromJSONObject<
            std::vector<picojson::value>>(data, MESSAGE_ATTRIBUTE_BCC);
    for_each(bccJS.begin(), bccJS.end(), arrayVectorStringConverter);
    if (conv_res.IsError()) {
      return conv_res;
    }
    message->setBCC(result);
    result.clear();

    auto priority = MessagingUtil::getValueFromJSONObject<bool>(data,
            MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY);
    message->setIsHighPriority(priority);

    auto isRead = MessagingUtil::getValueFromJSONObject<bool>(data,
            MESSAGE_ATTRIBUTE_IS_READ);
    message->setIsRead(isRead);

    std::shared_ptr<MessageBody> body = MessagingUtil::jsonToMessageBody(
            data[MESSAGE_ATTRIBUTE_MESSAGE_BODY]);
    message->setBody(body);

    AttachmentPtrVector attachments;
    auto ma = data.at(MESSAGE_ATTRIBUTE_MESSAGE_ATTACHMENTS).get<std::vector<picojson::value>>();

    auto arrayVectorAttachmentConverter = [&attachments] (picojson::value& v)->void
    {
        std::shared_ptr<MessageAttachment> attachment =
                MessagingUtil::jsonToMessageAttachment(v);

        attachments.push_back(attachment);
    };

    for_each(ma.begin(), ma.end(), arrayVectorAttachmentConverter);
    message->setMessageAttachments(attachments);

    *result_message = message;
    return PlatformResult(ErrorCode::NO_ERROR);
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

    AttachmentPtrVector inlineAttachments;
    auto ma = data.at(MESSAGE_BODY_ATTRIBUTE_INLINE_ATTACHMENTS ).get<picojson::array>();

    auto arrayVectorAttachmentConverter = [&inlineAttachments] (picojson::value& v)->void
    {
        inlineAttachments.push_back(jsonToMessageAttachment(v));
    };

    for_each(ma.begin(), ma.end(), arrayVectorAttachmentConverter);
    body->setInlineAttachments(inlineAttachments);

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

    const auto it = json.find(JSON_TO_SORT);

    if (json.end() == it || it->second.is<picojson::null>()) {
      return SortModePtr();
    }

    auto dataSort = getValueFromJSONObject<picojson::object>(json, JSON_TO_SORT);
    auto name = getValueFromJSONObject<std::string>(dataSort, JSON_TO_ATTRIBUTE_NAME);
    auto ord = getValueFromJSONObject<std::string>(dataSort, JSON_TO_ORDER);
    SortModeOrder order = ( ord == STR_SORT_DESC) ? SortModeOrder::DESC : SortModeOrder::ASC;
    return SortModePtr(new SortMode(name, order));
}

PlatformResult MessagingUtil::jsonToAbstractFilter(const picojson::object& json,
                                                   tizen::AbstractFilterPtr* result)
{
    LoggerD("Entered");

    const auto it = json.find(JSON_TO_FILTER);

    if (json.end() == it || it->second.is<picojson::null>()) {
        *result = AbstractFilterPtr();
        return PlatformResult(ErrorCode::NO_ERROR);
    }

    return jsonFilterToAbstractFilter(json.at(JSON_TO_FILTER).get<picojson::object>(), result);
}

PlatformResult MessagingUtil::jsonFilterToAbstractFilter(const picojson::object& filter,
                                                         tizen::AbstractFilterPtr* result)
{
    LoggerD("Entered");
    const auto& type = filter.at(JSON_FILTER_TYPE).get<std::string>();

    if (JSON_FILTER_ATTRIBUTE_TYPE == type) {

        return jsonFilterToAttributeFilter(filter, result);
    }
    if (JSON_FILTER_ATTRIBUTE_RANGE_TYPE == type) {
        return jsonFilterToAttributeRangeFilter(filter, result);
    }
    if (JSON_FILTER_COMPOSITE_TYPE == type) {
        return jsonFilterToCompositeFilter(filter, result);
    }

    LoggerE("Unsupported filter type: %s", type.c_str());
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unsupported filter type");
}

PlatformResult MessagingUtil::jsonFilterToAttributeFilter(const picojson::object& filter,
                                                          tizen::AbstractFilterPtr* result)
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
        return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Filter name is not recognized");
    }

    auto attributePtr = new AttributeFilter(name);
    attributePtr->setMatchFlag(filterMatch);
    attributePtr->setMatchValue(AnyPtr(new Any(filter.at(JSON_TO_MATCH_VALUE))));
    (*result).reset(attributePtr);
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingUtil::jsonFilterToAttributeRangeFilter(const picojson::object& filter,
                                                       tizen::AbstractFilterPtr* result)
{
    LoggerD("Entered");

    auto name = getValueFromJSONObject<std::string>(filter, JSON_TO_ATTRIBUTE_NAME);

    auto attributeRangePtr = new tizen::AttributeRangeFilter(name);
    attributeRangePtr->setInitialValue(AnyPtr(new Any(filter.at(JSON_TO_INITIAL_VALUE))));
    attributeRangePtr->setEndValue(AnyPtr(new Any(filter.at(JSON_TO_END_VALUE))));

    (*result).reset(attributeRangePtr);
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MessagingUtil::jsonFilterToCompositeFilter(const picojson::object& filter,
                                                          tizen::AbstractFilterPtr* result)
{
    LoggerD("Entered");

    using namespace tizen;

    const auto& type = filter.at(JSON_TO_TYPE).get<std::string>();

    CompositeFilterType filterType = CompositeFilterType::UNION;

    if (STR_FILTEROP_OR == type) {
        filterType = CompositeFilterType::UNION;
    }
    else if (STR_FILTEROP_AND == type) {
        filterType = CompositeFilterType::INTERSECTION;
    }
    else {
        LoggerE("Composite filter type is not recognized: %s", type.c_str());
        return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR,
                              "Composite filter type is not recognized");
    }

    auto compositeFilter = new CompositeFilter(filterType);

    for (const auto& a : filter.at(JSON_TO_FILTER_ARRAY).get<picojson::array>()) {
      AbstractFilterPtr filter;
      PlatformResult ret = jsonFilterToAbstractFilter(a.get<picojson::object>(), &filter);
      if (ret.IsError()) {
          delete compositeFilter;
          LoggerD("Convert JSON filter to Abstract filter failed (%s)", ret.message().c_str());
          return ret;
      }
      compositeFilter->addFilter(filter);
    }

    (*result).reset(compositeFilter);
    return PlatformResult(ErrorCode::NO_ERROR);
}

std::shared_ptr<MessageAttachment> MessagingUtil::jsonToMessageAttachment(const picojson::value& json)
{
    LoggerD("Entered");

    picojson::object data = json.get<picojson::object>();
    int attachmentId = std::atoi(getValueFromJSONObject<std::string>(data,
            MESSAGE_ATTACHMENT_ATTRIBUTE_ID).c_str());
    int messageId = std::atoi(getValueFromJSONObject<std::string>(data,
            MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID).c_str());
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
            ? picojson::value(std::to_string(attachment->getId()))
            : picojson::value();

    o[MESSAGE_ATTACHMENT_ATTRIBUTE_MESSAGE_ID] =
            attachment->isMessageIdSet()
            ? picojson::value(std::to_string(attachment->getMessageId()))
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

PlatformResult MessagingUtil::jsonToMessageConversation(const picojson::value& json,
                                        std::shared_ptr<MessageConversation>* result_conversation)
{
    LoggerD("Entered");
    std::shared_ptr<MessageConversation> conversation;
    picojson::object data = json.get<picojson::object>();
    std::string type = data.at("type").get<std::string>();
    MessageType mtype = UNDEFINED;
    auto platform_result = MessagingUtil::stringToMessageType(type, &mtype);

    if (!platform_result) return platform_result;

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

    std::string preview = MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_PREVIEW);
    conversation->setPreview(preview);

    std::string subject = MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_SUBJECT);
    conversation->setSubject(subject);

    /// MESSAGE_CONVERSATION_ATTRIBUTE_IS_READ ?

    std::vector<std::string> result;
    auto arrayVectorStringConverter = [&result] (picojson::value& v)->void {
        result.push_back(v.get<std::string>());
    };

    std::string from = MessagingUtil::getValueFromJSONObject<std::string>(data,
        MESSAGE_CONVERSATION_ATTRIBUTE_FROM);
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

    *result_conversation = conversation;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PostQueue::PostQueue(MessagingInstance& instance): instance_(instance)
{
    LoggerD("Entered: [%p]", this);
}
PostQueue::~PostQueue()
{
    LoggerD("Entered: [%p]", this);

    EmailManager::getInstance().RemoveCallbacksByQueue(*this);
}

void PostQueue::addAndResolve(const long cid, PostPriority priority, const std::string &json)
{
    LoggerD("Entered");

    std::shared_ptr<PostTask> t(new PostTask(priority));
    t->attach(json);
    tasks_mutex_.lock();
    tasks_.push_back(std::make_pair(cid, t));
    tasks_mutex_.unlock();

    resolve(PostPriority::HIGH);

    return;
}

void PostQueue::add(const long cid, PostPriority priority)
{
    LoggerD("Entered");

    tasks_mutex_.lock();
    tasks_.push_back(std::make_pair(cid, std::shared_ptr<PostTask>(new PostTask(priority))));
    tasks_mutex_.unlock();

    return;
}

void PostQueue::resolve(const long cid, const std::string &json)
{
    LoggerD("Entered: [%p]", this);

    tasks_mutex_.lock();

    TasksCollection::iterator i;
    i = std::find_if(tasks_.begin(), tasks_.end(), [&cid] (TasksCollectionItem item)->bool {
        return (cid == item.first);
    });

    if (tasks_.end() == i) {
        LoggerD("Not found cid");
        tasks_mutex_.unlock();
        return;
    }

    i->second->attach(json);
    tasks_mutex_.unlock();

    resolve(PostPriority::HIGH);
    return;
}

void PostQueue::resolve(PostPriority p)
{
    LoggerD("Entered: [%p]", this);

    TasksCollection::iterator i;

    tasks_mutex_.lock();
    i = std::find_if(tasks_.begin(), tasks_.end(), [&p] (TasksCollectionItem item)->bool {
        return (p == item.second->priority());
    });

    if (tasks_.end() == i) {
        // not found
        tasks_mutex_.unlock();

        if (PostPriority::LAST != p) {
            return resolve(static_cast<PostPriority>(p-1));
        } else {
            return;
        }
    }

    if (TaskState::READY == i->second->state()) {
        i->second->resolve();
        std::string json = i->second->json();

        i = tasks_.erase(i);
        tasks_mutex_.unlock();

        Instance::PostMessage(&instance_, json.c_str());
    } else if (TaskState::NEW == i->second->state()) {
        tasks_mutex_.unlock();

        return;
    } else if (TaskState::DONE == i->second->state()) {
        i = tasks_.erase(i);
        tasks_mutex_.unlock();
    }

    return resolve(static_cast<PostPriority>(p));
}

PostQueue::PostTask::PostTask()
{
    LoggerD("Entered");
    priority_ = PostPriority::LOW;
    state_ = TaskState::NEW;
}
PostQueue::PostTask::PostTask(PostPriority p)
{
    LoggerD("Entered");
    priority_ = p;
    state_ = TaskState::NEW;
}
PostQueue::PostTask::~PostTask()
{
    LoggerD("Entered");
}

void PostQueue::PostTask::attach(const std::string &j)
{
    LoggerD("Entered");
    if (TaskState::DONE == state_) {
        return;
    }
    json_ = j;
    state_ = TaskState::READY;
    return;
}

PostPriority PostQueue::PostTask::priority()
{
    return priority_;
}

PostQueue::TaskState PostQueue::PostTask::state()
{
    return state_;
}

std::string PostQueue::PostTask::json()
{
    return json_;
}

void PostQueue::PostTask::resolve()
{
    LoggerD("Entered");
    if (TaskState::READY == state_) {
        state_ = TaskState::DONE;
    }
    return;
}


} // messaging
} // extension
