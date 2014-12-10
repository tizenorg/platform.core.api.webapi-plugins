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
 * @file        Message.h
 */

#ifndef __TIZEN_MESSAGE_H__
#define __TIZEN_MESSAGE_H__

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <memory>
#include <msg_product.h>
#include <msg_storage.h>
#include "MessagingUtil.h"
#include "MessageAttachment.h"
#include "MessageBody.h"
#include "JSVector.h"
#include <email-api.h>
#include <AbstractFilter.h>
#include <TelNetwork.h>

namespace DeviceAPI {
namespace Messaging {

class Message;

struct MessageHolder {
    std::shared_ptr<Message> ptr;
};

typedef std::shared_ptr<Message> MessagePtr;
typedef std::vector<MessagePtr> MessagePtrVector;

enum AttachmentType {
    EXTERNAL = 0,
    INLINE = 1
};

class Message : public Tizen::FilterableObject {
public:
// constructor
    Message();
    virtual ~Message();

// attributes getters
    int getId() const;
    int getConversationId() const;
    int getFolderId() const;
    MessageType getType() const;
    time_t getTimestamp() const;
    std::string getFrom() const;
    std::vector<std::string> getTO() const;
    JSObjectRef getJSTO(JSContextRef global_ctx);
    std::vector<std::string> getCC() const;
    JSObjectRef getJSCC(JSContextRef global_ctx);
    std::vector<std::string> getBCC() const;
    JSObjectRef getJSBCC(JSContextRef global_ctx);
    std::shared_ptr<MessageBody> getBody() const;
    bool getIsRead() const;
     // getHasAttachment() is virtual to support MMS and email differently
    virtual bool getHasAttachment() const;
    bool getIsHighPriority() const;
    std::string getSubject() const;
    int getInResponseTo() const;
    MessageStatus getMessageStatus() const;
    AttachmentPtrVector getMessageAttachments() const;
    JSObjectRef getJSMessageAttachments(JSContextRef global_ctx);
    int getServiceId() const;
    TelNetworkDefaultDataSubs_t getSimIndex() const;

// attributes setters (virtual because some of them can be overriden in sub classes)
    virtual void setId(int id);
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
    void addSMSRecipientsToStruct(const std::vector<std::string> &recipients,
            msg_struct_t &msg);
    // gets recipients list for SMS message
    void addMMSRecipientsToStruct(const std::vector<std::string> &recipients,
            msg_struct_t &msg, int type);
    /**
     * Updates message with data from email_mail_data_t structure.
     * @param mail
     */
    virtual void updateEmailMessage(email_mail_data_t& mail);

    // gets from(sender) address from short msg struct
    static std::string getShortMsgSenderFromStruct(msg_struct_t &msg);
    // function for filling msg_struct_t fields
    static msg_struct_t convertPlatformShortMessageToStruct(Message* message,
            msg_handle_t handle);
    // gets recipients list for SMS message
    std::vector<std::string> getSMSRecipientsFromStruct(msg_struct_t &msg);
    // gets recipients list for MMS message
    static std::vector<std::string> getMMSRecipientsFromStruct(msg_struct_t &msg, int type);
    // function for filling Message attributes
    static Message* convertPlatformShortMessageToObject(msg_struct_t msg);
    static void addMMSBodyAndAttachmentsToStruct(const AttachmentPtrVector attach,
            msg_struct_t &mms_struct, Message* message);
    static void setMMSBodyAndAttachmentsFromStruct(Message *message, msg_struct_t &msg);

    static email_mail_data_t* convertPlatformEmail(std::shared_ptr<Message> message);
    static void addEmailAttachments(std::shared_ptr<Message> message);
    static std::string convertEmailRecipients(const std::vector<std::string> &recipients);
    static std::vector<std::string> getEmailRecipientsFromStruct(const char *recipients);
    static std::shared_ptr<Message> convertPlatformEmailToObject(email_mail_data_t& mail);
    static std::shared_ptr<MessageBody> convertEmailToMessageBody(email_mail_data_t& mail);
    static AttachmentPtrVector convertEmailToMessageAttachment(email_mail_data_t& mail);

    // Tizen::FilterableObject
    virtual bool isMatchingAttribute(const std::string& attribute_name,
            const Tizen::FilterMatchFlag match_flag,
            Tizen::AnyPtr match_value) const;

    virtual bool isMatchingAttributeRange(const std::string& attribute_name,
            Tizen::AnyPtr initial_value,
            Tizen::AnyPtr end_value) const;
protected:
    //! Message id
    int m_id;
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
    Common::JSStringVector m_to;
    //! Message CarbonCopy recipients (used only for email)
    Common::JSStringVector m_cc;
    //! Message BlindCarbonCopy recipients (used only for email)
    Common::JSStringVector m_bcc;
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
    JSAttachmentsVector m_attachments;
    //! SIM index which indicate a sim to send message.
    TelNetworkDefaultDataSubs_t m_sim_index;
private:
    static std::vector<std::string> split(const std::string& input,
            char delimiter);
};

} // Messaging
} // DeviceAPI

#endif // __TIZEN_MESSAGE_H__
