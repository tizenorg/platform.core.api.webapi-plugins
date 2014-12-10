//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @file        MessageConversation.h
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
#include "MessagingUtil.h"
#include <AbstractFilter.h>

namespace DeviceAPI {
namespace Messaging {

class MessageConversation;

struct MessageConversationHolder {
    std::shared_ptr<MessageConversation>ptr;
};

typedef std::shared_ptr<MessageConversation> ConversationPtr;

typedef std::vector<ConversationPtr> ConversationPtrVector;

class MessageConversation : public Tizen::FilterableObject {
public:
    MessageConversation();
    ~MessageConversation();

    // attributes getters
    int getConversationId() const;
    MessageType getType() const;
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

    static std::shared_ptr<MessageConversation> convertConversationStructToObject(
        unsigned int threadId, MessageType msgType, msg_handle_t handle = NULL);
    static std::shared_ptr<MessageConversation> convertEmailConversationToObject(
        unsigned int threadId);
    /**
     *
     * @param threadId Id of Message (not Conversation)
     * @param handle
     * @return
     */
    static std::shared_ptr<MessageConversation> convertMsgConversationToObject(
        unsigned int threadId, msg_handle_t handle);

    virtual void setConversationId(int id);
    virtual void setMessageCount(int count);
    virtual void setUnreadMessages(int count);

    // Tizen::FilterableObject
    virtual bool isMatchingAttribute(const std::string& attribute_name,
            const Tizen::FilterMatchFlag match_flag,
            Tizen::AnyPtr match_value) const;

    virtual bool isMatchingAttributeRange(const std::string& attribute_name,
            Tizen::AnyPtr initial_value,
            Tizen::AnyPtr end_value) const;

private:
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
} // Messaging
} // DeviceAPI

#endif // __TIZEN_MESSAGE_CONVERSATION_H__
