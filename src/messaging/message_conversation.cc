// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"
#include "message.h"
#include "message_conversation.h"

#define MAX_THREAD_DATA_LEN 128

namespace extension {
namespace messaging {

using namespace tizen;

// *** constructor
MessageConversation::MessageConversation():
    m_conversation_id(-1),
    m_conversation_type(UNDEFINED),
    m_count(0),
    m_unread_messages(0),
    m_is_read(false)
{
    LoggerD("Message Conversation constructor.");
}

MessageConversation::~MessageConversation()
{
    LoggerD("Message Conversation destructor.");
}
// *** attributes getters
int MessageConversation::getConversationId() const
{
    return m_conversation_id;
}

MessageType MessageConversation::getType() const
{
    return m_conversation_type;
}

time_t MessageConversation::getTimestamp() const
{
    return m_timestamp;
}

unsigned long MessageConversation::getMessageCount() const
{
    return m_count;
}

unsigned long MessageConversation::getUnreadMessages() const
{
    return m_unread_messages;
}

std::string MessageConversation::getPreview() const
{
    return m_preview;
}

std::string MessageConversation::getSubject() const
{
    return m_conversation_subject;
}

bool MessageConversation::getIsRead() const
{
    return m_is_read;
}

std::string MessageConversation::getFrom() const
{
    return m_from;
}

std::vector<std::string> MessageConversation::getTo() const
{
    return m_to;
}

std::vector<std::string> MessageConversation::getCC() const
{
    return m_cc;
}

std::vector<std::string> MessageConversation::getBCC() const
{
    return m_bcc;
}

int MessageConversation::getLastMessageId() const
{
    return m_last_message_id;
}

std::shared_ptr<MessageConversation> MessageConversation::convertMsgConversationToObject(
        unsigned int threadId, msg_handle_t handle)
{
    std::shared_ptr<MessageConversation> conversation (new MessageConversation());

    msg_struct_t msgInfo = NULL;
    msg_struct_t sendOpt = NULL;

    msg_struct_t msg_thread = NULL;

    msg_struct_list_s convViewList;
    msg_list_handle_t addr_list = NULL;
    msg_struct_t addr_info = NULL;

    msg_error_t err = MSG_SUCCESS;

    int tempInt;
    bool tempBool;
    int nToCnt;
    unsigned int lastMsgIndex = 0;
    char msgData[MAX_THREAD_DATA_LEN] = {0,};

    try {
        msgInfo = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
        sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);

        conversation->m_conversation_id = threadId;

        msg_thread = msg_create_struct(MSG_STRUCT_THREAD_INFO);
        err = msg_get_thread(handle, conversation->m_conversation_id, msg_thread);
        if (err != MSG_SUCCESS)
        {
            LoggerE("Failed to retrieve thread.");
            throw common::UnknownException("Failed to retrieve thread.");
        }
        msg_get_int_value(msg_thread, MSG_THREAD_MSG_TYPE_INT, &tempInt);
        switch(tempInt)
        {
            case MSG_TYPE_SMS:
            case MSG_TYPE_SMS_CB:
            case MSG_TYPE_SMS_JAVACB:
            case MSG_TYPE_SMS_WAPPUSH:
            case MSG_TYPE_SMS_MWI:
            case MSG_TYPE_SMS_SYNCML:
            case MSG_TYPE_SMS_REJECT:
                conversation->m_conversation_type = SMS;
                break;
            case MSG_TYPE_MMS:
            case MSG_TYPE_MMS_JAVA:
            case MSG_TYPE_MMS_NOTI:
                conversation->m_conversation_type = MMS;
                break;
        }

        msg_get_int_value(msg_thread, MSG_THREAD_MSG_TIME_INT, &tempInt);
        conversation->m_timestamp = tempInt;

        msg_get_int_value(msg_thread, MSG_THREAD_UNREAD_COUNT_INT, &tempInt);
        conversation->m_unread_messages = tempInt;

        msg_get_str_value(msg_thread, MSG_THREAD_MSG_DATA_STR, msgData, MAX_THREAD_DATA_LEN);

        conversation->m_preview = msgData;

        err = msg_get_conversation_view_list(handle, conversation->m_conversation_id,
            &convViewList);
        if (err != MSG_SUCCESS)
        {
            LoggerE("Get conversation(msg) view list fail.");
            throw common::UnknownException("Get conversation(msg) view list fail.");
        }

        lastMsgIndex = convViewList.nCount - 1;
        conversation->m_count = convViewList.nCount;

        msg_get_bool_value(convViewList.msg_struct_info[lastMsgIndex], MSG_CONV_MSG_READ_BOOL, &tempBool);
        conversation->m_is_read = tempBool;

        msg_get_int_value(convViewList.msg_struct_info[lastMsgIndex], MSG_CONV_MSG_ID_INT, &tempInt);
        conversation->m_last_message_id = tempInt;

        if (msg_get_message(handle, conversation->m_last_message_id, msgInfo,
            sendOpt) != MSG_SUCCESS)
        {
            LoggerE("Get message fail.");
            throw common::UnknownException("get message fail.");
        }

        msg_get_int_value(convViewList.msg_struct_info[lastMsgIndex], MSG_CONV_MSG_DIRECTION_INT, &tempInt);

        msg_get_list_handle(msgInfo, MSG_MESSAGE_ADDR_LIST_HND, (void **)&addr_list);
        nToCnt = msg_list_length(addr_list);

        if (MSG_DIRECTION_TYPE_MT == tempInt)
        {
            if (nToCnt > 0 && nToCnt < MAX_TO_ADDRESS_CNT )
            {
                char strNumber[MAX_ADDRESS_VAL_LEN] = {0,};
                addr_info = (msg_struct_t)msg_list_nth_data(addr_list, nToCnt-1);
                msg_get_str_value(addr_info, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, strNumber, MAX_ADDRESS_VAL_LEN);

                if (strNumber[0] != '\0')
                {
                    conversation->m_from = strNumber;
                }
                else
                {
                    LoggerD("address is null ");
                }
            }
            else
            {
                LoggerD("address count index fail");
            }
        }
        else
        {
            if (nToCnt > 0 && nToCnt < MAX_TO_ADDRESS_CNT )
            {
                for (int index = 0; index < nToCnt; index++)
                {
                    addr_info = (msg_struct_t)msg_list_nth_data(addr_list, index);
                    char strNumber[MAX_ADDRESS_VAL_LEN] = {0,};
                    msg_get_str_value(addr_info, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR, strNumber, MAX_ADDRESS_VAL_LEN);

                    conversation->m_to.push_back(strNumber);
                }
            }
            else
            {
                LoggerD("address fetch fail");
            }
        }

        char strTemp[MAX_SUBJECT_LEN] = {0};
        msg_get_str_value(msgInfo, MSG_MESSAGE_SUBJECT_STR, strTemp, MAX_SUBJECT_LEN);

        conversation->m_conversation_subject = strTemp;
        msg_release_list_struct(&convViewList);
        msg_release_struct(&msgInfo);
        msg_release_struct(&sendOpt);
        msg_release_struct(&msg_thread);
    } catch (const common::PlatformException& ex) {
        msg_release_list_struct(&convViewList);
        msg_release_struct(&msgInfo);
        msg_release_struct(&sendOpt);
        msg_release_struct(&msg_thread);
        LoggerE("%s (%s)", (ex.name()).c_str(), (ex.message()).c_str());
        throw common::UnknownException("Unable to convert short message conversation.");
    } catch (...) {
        msg_release_list_struct(&convViewList);
        msg_release_struct(&msgInfo);
        msg_release_struct(&sendOpt);
        msg_release_struct(&msg_thread);
        throw common::UnknownException("Unable to convert short message conversation.");
    }

    return conversation;
}

std::shared_ptr<MessageConversation> MessageConversation::convertEmailConversationToObject(
        unsigned int threadId)
{
    std::shared_ptr<MessageConversation> conversation (new MessageConversation());

    email_mail_list_item_t *resultMail = NULL;

    if(email_get_thread_information_ex(threadId, &resultMail) != EMAIL_ERROR_NONE)
    {
        LoggerE("Couldn't get conversation");
        throw common::UnknownException("Couldn't get conversation.");
    } else {
        if (!resultMail)
        {
            LoggerE("Data is null");
            throw common::UnknownException("Get email data fail.");
        }

        email_mail_data_t* mailData = NULL;

        if (email_get_mail_data(resultMail->mail_id,
                &mailData) != EMAIL_ERROR_NONE)
        {
            free(resultMail);
            throw common::UnknownException("Get email data fail.");
        }

        if (!mailData) {
            free(resultMail);
            throw common::UnknownException("Get email data fail.");
        }

        int index = 0;
        int count = 0;
        conversation->m_unread_messages = 0;
        email_mail_list_item_t *mailList = NULL;

        if (email_get_mail_list(mailData->account_id, 0, threadId, 0,
            resultMail->thread_item_count, EMAIL_SORT_DATETIME_HIGH, &mailList,
            &count) != EMAIL_ERROR_NONE)
        {
            email_free_mail_data(&mailData , 1);
            free(resultMail);
            throw common::UnknownException("Get email data list fail.");
        }

        for (index = 0; index < count; index++)
        {
            if (mailList[index].flags_seen_field)
            {
                conversation->m_unread_messages++;
            }
        }
        conversation->m_count = resultMail->thread_item_count;

        conversation->m_conversation_id = threadId;

        conversation->m_conversation_type = EMAIL;

        conversation->m_timestamp = resultMail->date_time;

        if (resultMail->preview_text[0] != '\0')
        {
            conversation->m_preview = resultMail->preview_text;
        }

        if (resultMail->subject[0] != '\0')
        {
            conversation->m_conversation_subject = resultMail->subject;
        }

        conversation->m_is_read = (bool)resultMail->flags_seen_field;

        if (resultMail->full_address_from[0] != '\0')
        {
            conversation->m_from = MessagingUtil::extractSingleEmailAddress(
                    resultMail->full_address_from);
        }

        if (mailData->full_address_to != NULL)
        {
            conversation->m_to = Message::getEmailRecipientsFromStruct(
                mailData->full_address_to);
        }

        if (mailData->full_address_cc != NULL)
        {
            conversation->m_cc = Message::getEmailRecipientsFromStruct(
                mailData->full_address_cc);
        }

        if (mailData->full_address_bcc != NULL)
        {
            conversation->m_bcc = Message::getEmailRecipientsFromStruct(
                mailData->full_address_bcc);
        }

        conversation->m_last_message_id = resultMail->mail_id;

        if (mailData != NULL)
        {
            email_free_mail_data(&mailData , 1);
        }
    }

    if (resultMail != NULL)
    {
        free(resultMail);
    }

    return conversation;
}

std::shared_ptr<MessageConversation> MessageConversation::convertConversationStructToObject(
        unsigned int threadId, MessageType msgType, msg_handle_t handle)
{
    std::shared_ptr<MessageConversation> conversation (new MessageConversation());

    if (EMAIL == msgType) {
        conversation = convertEmailConversationToObject(threadId);
    } else {
        if(handle != NULL) {
            conversation = convertMsgConversationToObject(threadId, handle);
        } else {
            LoggerE("Handle has not been sent.");
            throw common::UnknownException("Handle has not been sent.");
        }
    }

    return conversation;
}

void MessageConversation::setConversationId(int id)
{
    m_conversation_id = id;
}

void MessageConversation::setType(MessageType type)
{
    m_conversation_type = type;
}

void MessageConversation::setTimestamp(time_t timestamp)
{
    m_timestamp = timestamp;
}

void MessageConversation::setMessageCount(int count)
{
    m_count = count;
}

void MessageConversation::setUnreadMessages(int unread_messages)
{
    m_unread_messages = unread_messages;
}

void MessageConversation::setPreview(std::string preview)
{
    m_preview = preview;
}

void MessageConversation::setSubject(std::string conversation_subject)
{
    m_conversation_subject = conversation_subject;
}

void MessageConversation::setIsRead(bool is_read)
{
    m_is_read = is_read;
}

void MessageConversation::setFrom(std::string from)
{
    m_from = from;
}

void MessageConversation::setTo(std::vector<std::string> &to)
{
    m_to = to;
}

void MessageConversation::setCC(std::vector<std::string> &cc)
{
    m_cc = cc;
}

void MessageConversation::setBCC(std::vector<std::string> &bcc)
{
    m_bcc = bcc;
}

void MessageConversation::setLastMessageId(int last_message_id)
{
    m_last_message_id = last_message_id;
}

/**
 *
 *  Attribute      | Attribute filter| Attribute range filter
 *                 | supported       | supported
 * ----------------+-----------------+------------------------
 * id              | Yes             | No
 * type            | Yes             | No
 * timestamp       | No              | Yes
 * messageCount    | Yes             | No
 * unreadMessages  | Yes             | No
 * preview         | Yes             | No
 * subject         | No              | No
 * isRead          | No              | No
 * from            | Yes             | No
 * to              | Yes             | No
 * cc              | No              | No
 * bcc             | No              | No
 * lastMessageId   | No              | No
 */

namespace CONVERSATION_FILTER_ATTRIBUTE {
const std::string ID = MESSAGE_CONVERSATION_ATTRIBUTE_ID;
const std::string TYPE = MESSAGE_CONVERSATION_ATTRIBUTE_TYPE;
const std::string TIMESTAMP = MESSAGE_CONVERSATION_ATTRIBUTE_TIMESTAMP;
const std::string MESSAGE_COUNT = MESSAGE_CONVERSATION_ATTRIBUTE_MESSAGE_COUNT;
const std::string UNREAD_MESSAGES = MESSAGE_CONVERSATION_ATTRIBUTE_UNREAD_MESSAGES;
const std::string PREVIEW = MESSAGE_CONVERSATION_ATTRIBUTE_PREVIEW;
const std::string FROM = MESSAGE_CONVERSATION_ATTRIBUTE_FROM;
const std::string TO = MESSAGE_CONVERSATION_ATTRIBUTE_TO;
} //namespace CONVERSATION_FILTER_ATTRIBUTE

bool MessageConversation::isMatchingAttribute(const std::string& attribute_name,
            const FilterMatchFlag match_flag,
            AnyPtr match_value) const
{
    LoggerD("Entered");
    auto key = match_value->toString();
    LoggerD("attribute_name: %s match_flag:%d match_value:%s", attribute_name.c_str(),
            match_flag, key.c_str());

    using namespace CONVERSATION_FILTER_ATTRIBUTE;

    if(ID == attribute_name) {
        return FilterUtils::isStringMatching(key, std::to_string(getConversationId()),
                match_flag);
    }
    else if(TYPE == attribute_name) {
        const MessageType msg_type = getType();
        const std::string msg_type_str = MessagingUtil::messageTypeToString(msg_type);
        return FilterUtils::isStringMatching(key, msg_type_str, match_flag);
    }
    else if(MESSAGE_COUNT == attribute_name) {
        return FilterUtils::isStringMatching(key, std::to_string(getMessageCount()),
                match_flag);
    }
    else if(UNREAD_MESSAGES == attribute_name) {
        return FilterUtils::isStringMatching(key, std::to_string(getUnreadMessages()),
                match_flag);
    }
    else if(PREVIEW == attribute_name) {
        return FilterUtils::isStringMatching(key, getPreview(), match_flag);
    }
    else if(FROM == attribute_name) {
        return FilterUtils::isStringMatching(key, getFrom(), match_flag);
    }
    else if(TO == attribute_name) {
        return FilterUtils::isAnyStringMatching(key, getTo(), match_flag);
    }
    else {
        LoggerD("attribute:%s is NOT SUPPORTED", attribute_name.c_str());
    }

    return false;
}

bool MessageConversation::isMatchingAttributeRange(const std::string& attribute_name,
            AnyPtr initial_value,
            AnyPtr end_value) const
{
    LoggerD("Entered attribute_name: %s", attribute_name.c_str());

    using namespace CONVERSATION_FILTER_ATTRIBUTE;

    if(TIMESTAMP == attribute_name) {
        return FilterUtils::isTimeStampInRange(getTimestamp(), initial_value, end_value);
    }
    else {
        LoggerD("attribute:%s is NOT SUPPORTED", attribute_name.c_str());
    }
    return false;
}

}    //messaging
}    //extension

