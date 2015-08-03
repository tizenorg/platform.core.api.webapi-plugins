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


#ifndef __TIZEN_MESSAGE_CONVERSATION_H__
#define __TIZEN_MESSAGE_CONVERSATION_H__

#include <memory>
#include <string>
#include <time.h>
#include <vector>
#include <email-api.h>
#include <msg.h>
#include <msg_storage.h>
#include "messaging_util.h"
#include "MsgCommon/AbstractFilter.h"

namespace extension {
namespace messaging {

class MessageConversation;

struct MessageConversationHolder {
    std::shared_ptr<MessageConversation>ptr;
};

typedef std::shared_ptr<MessageConversation> ConversationPtr;

typedef std::vector<ConversationPtr> ConversationPtrVector;

class MessageConversation : public tizen::FilterableObject {
public:
    MessageConversation();
    ~MessageConversation();

    // attributes getters
    int getConversationId() const;
    MessageType getType() const;
    std::string getTypeString() const;
    time_t getTimestamp() const;
    unsigned long getMessageCount() const;
    unsigned long getUnreadMessages() const;
    std::string getPreview() const;
    std::string getSubject() const;
    bool getIsRead() const;
    std::string getFrom() const;
    std::vector<std::string> getTo() const;
    std::vector<std::string> getCC() const;
    std::vector<std::string> getBCC() const;
    int getLastMessageId() const;

    static common::PlatformResult convertEmailConversationToObject(
        unsigned int threadId, std::shared_ptr<MessageConversation>* result);
    /**
     *
     * @param threadId Id of Message (not Conversation)
     * @param handle
     * @return
     */
    static common::PlatformResult convertMsgConversationToObject(
        unsigned int threadId, msg_handle_t handle, std::shared_ptr<MessageConversation>* result);

    virtual void setConversationId(int id);
    virtual void setType(MessageType type);
    virtual void setTimestamp(time_t timestamp);
    virtual void setMessageCount(int count);
    virtual void setUnreadMessages(int unread_messages);
    virtual void setPreview(std::string preview);
    virtual void setSubject(std::string conversation_subject);
    virtual void setIsRead(bool is_read);
    virtual void setFrom(std::string from);
    virtual void setTo(std::vector<std::string> &to);
    virtual void setCC(std::vector<std::string> &cc);
    virtual void setBCC(std::vector<std::string> &bcc);
    virtual void setLastMessageId(int last_message_id);

    // tizen::FilterableObject
    virtual bool isMatchingAttribute(const std::string& attribute_name,
            const tizen::FilterMatchFlag match_flag,
            tizen::AnyPtr match_value) const;

    virtual bool isMatchingAttributeRange(const std::string& attribute_name,
            tizen::AnyPtr initial_value,
            tizen::AnyPtr end_value) const;

private:
    std::string SanitizeUtf8String(const std::string& input);

    int m_conversation_id;
    MessageType m_conversation_type;
    time_t m_timestamp;
    unsigned long m_count;
    unsigned long m_unread_messages;
    std::string m_preview;
    std::string m_conversation_subject;
    bool m_is_read;
    std::string m_from;
    std::vector<std::string> m_to;
    std::vector<std::string> m_cc;
    std::vector<std::string> m_bcc;
    int m_last_message_id;
};

}    //messaging
}    //extension

#endif // __TIZEN_MESSAGE_CONVERSATION_H__
