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
 
#ifndef MESSAGING_MESSAGE_H_
#define MESSAGING_MESSAGE_H_

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <memory>
#include <msg.h>
#include <msg_storage.h>
#include <email-api.h>
#include <TelNetwork.h>

#include "message_attachment.h"
#include "messaging_util.h"
#include "message_body.h"
#include "MsgCommon/AbstractFilter.h"
#include "common/platform_result.h"

namespace extension {
namespace messaging {

class Message;

struct MessageHolder {
    std::shared_ptr<Message> ptr;
};

typedef std::shared_ptr<Message> MessagePtr;
typedef std::vector<MessagePtr> MessagePtrVector;

enum AttachmentType {
    EXTERNAL = 0, INLINE = 1
};

using namespace tizen;

class Message : public FilterableObject {
public:
// constructor
    Message();
    virtual ~Message();

// attributes getters
    int getId() const;
    int getOldId() const;
    int getConversationId() const;
    int getFolderId() const;
    MessageType getType() const;
    std::string getTypeString() const;
    time_t getTimestamp() const;
    std::string getFrom() const;
    std::vector<std::string> getTO() const;
    std::vector<std::string> getCC() const;
    std::vector<std::string> getBCC() const;
    std::shared_ptr<MessageBody> getBody() const;
    bool getIsRead() const;
    // getHasAttachment() is virtual to support MMS and email differently
    virtual bool getHasAttachment() const;
    bool getIsHighPriority() const;
    std::string getSubject() const;
    int getInResponseTo() const;
    MessageStatus getMessageStatus() const;
    AttachmentPtrVector getMessageAttachments() const;
    int getServiceId() const;
    TelNetworkDefaultDataSubs_t getSimIndex() const;

// attributes setters (virtual because some of them can be overriden in sub classes)
    virtual void setId(int id);
    virtual void setOldId(int id);
    virtual void setConversationId(int id);
    virtual void setFolderId(int id);
    // type setting not allowed so no setter provided
    virtual void setTimeStamp(time_t timestamp);
    virtual void setFrom(std::string from);
    virtual void setTO(std::vector<std::string> &to);
    virtual void setCC(std::vector<std::string> &cc);
    virtual void setBCC(std::vector<std::string> &bcc);
    virtual void setBody(std::shared_ptr<MessageBody>& body);
    virtual void setIsRead(bool read);
    // has attachment can't be set explicity -> no setter for this flag
    virtual void setIsHighPriority(bool highpriority);
    virtual void setSubject(std::string subject);
    virtual void setInResponseTo(int inresp);
    virtual void setMessageStatus(MessageStatus status);
    virtual void setMessageAttachments(AttachmentPtrVector &attachments);
    virtual void setServiceId(int service_id);
    virtual void setSimIndex(TelNetworkDefaultDataSubs_t sim_index);

// support for optional, nullable (at JS layer) attibutes
    // message id
    bool is_id_set() const;
    // conversation id
    bool is_conversation_id_set() const;
    // folder id
    bool is_folder_id_set() const;
    // timestamp
    bool is_timestamp_set() const;
    // message sender
    bool is_from_set() const;
    // related message ("parent" message)
    bool is_in_response_set() const;
    // service id
    bool is_service_is_set() const;
    // gets recipients list for SMS message
    common::PlatformResult addSMSRecipientsToStruct(const std::vector<std::string> &recipients,
            msg_struct_t &msg);
    // gets recipients list for SMS message
    common::PlatformResult addMMSRecipientsToStruct(const std::vector<std::string> &recipients,
            msg_struct_t &msg,
            int type);
    /**
     * Updates message with data from email_mail_data_t structure.
     * @param mail
     */
    virtual common::PlatformResult updateEmailMessage(email_mail_data_t& mail);

    // gets from(sender) address from short msg struct
    static std::string getShortMsgSenderFromStruct(msg_struct_t &msg);
    // function for filling msg_struct_t fields
    static common::PlatformResult convertPlatformShortMessageToStruct(Message* message,
            msg_handle_t handle, msg_struct_t* result);
    // gets recipients list for SMS message
    common::PlatformResult getSMSRecipientsFromStruct(msg_struct_t &msg,
                                              std::vector<std::string>* result_address);
    // gets recipients list for MMS message
    static common::PlatformResult getMMSRecipientsFromStruct(msg_struct_t &msg,
            int type, std::vector<std::string>* result_address);
    // function for filling Message attributes
    static common::PlatformResult convertPlatformShortMessageToObject(msg_struct_t msg, Message** message);
    static common::PlatformResult findShortMessageById(const int id, MessagePtr* message);
    static common::PlatformResult addMMSBodyAndAttachmentsToStruct(const AttachmentPtrVector &attach,
            msg_struct_t &mms_struct,
            Message* message);
    static common::PlatformResult setMMSBodyAndAttachmentsFromStruct(Message *message,
            msg_struct_t &msg);

    static common::PlatformResult convertPlatformEmail(std::shared_ptr<Message> message,
                                                   email_mail_data_t** result);
    static common::PlatformResult addEmailAttachments(std::shared_ptr<Message> message);
    static std::string convertEmailRecipients(const std::vector<std::string> &recipients);
    static std::vector<std::string> getEmailRecipientsFromStruct(const char *recipients);
    static common::PlatformResult convertPlatformEmailToObject(email_mail_data_t& mail,
                                                               std::shared_ptr<Message>* result);
    static std::shared_ptr<MessageBody> convertEmailToMessageBody(email_mail_data_t& mail);
    static common::PlatformResult convertEmailToMessageAttachment(email_mail_data_t& mail,
                                                                  AttachmentPtrVector* result);

    // tizen::FilterableObject
    virtual bool isMatchingAttribute(const std::string& attribute_name,
          const FilterMatchFlag match_flag,
          AnyPtr match_value) const;

    virtual bool isMatchingAttributeRange(const std::string& attribute_name,
          AnyPtr initial_value,
          AnyPtr end_value) const;

protected:
    //! Message id
    int m_id;
    //! Old Message id - for email update hack
    int m_old_id;
    //! Flag for checking if id is set (false means: not set)
    bool m_id_set;
    //! Conversation id
    int m_conversation_id;
    //! Flag for checking if conversation id is set (false means: not set)
    bool m_conversation_id_set;
    //! Folder id
    int m_folder_id;
    //! Flag for checking if folder id is set (false means: not set)
    bool m_folder_id_set;
    //! Message type (messaging.sms, messaging.mms, messaging.email)
    MessageType m_type;
    //! Timestamp - time when message has been sent/received
    time_t m_timestamp;
    //! Flag for checking if timestamp is set (false means: not set)
    bool m_timestamp_set;
    //! Message sender address (email) or number (SMS, MMS)
    std::string m_from;
    //! Flag for checking if sender is set (false means: not set)
    bool m_from_set;
    //! Message recipients
    std::vector<std::string> m_to;
    //! Message CarbonCopy recipients (used only for email)
    std::vector<std::string> m_cc;
    //! Message BlindCarbonCopy recipients (used only for email)
    std::vector<std::string> m_bcc;
    //! MessageBody (object containg plainBody and htmlBody for emails)
    std::shared_ptr<MessageBody> m_body;
    //! Service id
    int m_service_id;
    //! Message isRead flag
    bool m_is_read;
    //! Message hasAttachment flag
    bool m_has_attachment;
    //! Message isHighPriority flag
    bool m_high_priority;
    //! Message subject (used in MMS and email)
    std::string m_subject;
    //! Id of original message when message is a reply or forward
    int m_in_response;
    //! Flag for checking if id of related message is set (false means: not set)
    bool m_in_response_set;
    //! Flag for checking if service id is set
    bool m_service_id_set;
    //! Outgoing Message status (SENT, SENDING, DRAFT etc)
    MessageStatus m_status;
    //! Attachments attached to this message
    AttachmentPtrVector m_attachments;
    //! SIM index which indicate a sim to send message.
    TelNetworkDefaultDataSubs_t m_sim_index;

private:
    static std::vector<std::string> split(const std::string& input,
            char delimiter);
};

}    //messaging
}    //extension

#endif /* MESSAGING_MESSAGE_H_ */

