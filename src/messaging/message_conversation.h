// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


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
