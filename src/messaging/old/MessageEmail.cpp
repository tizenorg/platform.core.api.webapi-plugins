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
 * @file        MessageEmail.cpp
 */

#include <PlatformException.h>
#include <Logger.h>
#include "MessageEmail.h"
#include <GlobalContextManager.h>

namespace DeviceAPI {
namespace Messaging {

MessageEmail::MessageEmail():
    Message()
{
    LOGD("MessageEmail constructor.");
    this->m_type = MessageType(EMAIL);
}

MessageEmail::~MessageEmail()
{
    LOGD("MessageEmail destructor.");
}

// *** overrided methods
void MessageEmail::setCC(std::vector<std::string> &cc)
{
    // CC recipient's format validation should be done by email service
    m_cc = cc;

    if(m_cc.empty()) {
        LOGD("Recipient's list cleared");
        return;
    }
}

void MessageEmail::setBCC(std::vector<std::string> &bcc)
{
    // BCC recipient's format validation should be done by email service
    m_bcc = bcc;

    if(m_bcc.empty()) {
        LOGD("Recipient's list cleared");
        return;
    }
}

void MessageEmail::setSubject(std::string subject)
{
    m_subject = subject;
}

void MessageEmail::setIsHighPriority(bool highpriority)
{
    m_high_priority = highpriority;
}

void MessageEmail::setMessageAttachments(AttachmentPtrVector &attachments)
{
    m_attachments = attachments;

    m_has_attachment = true;
    if(m_attachments.empty()) {
        LOGD("Recipient's list cleared");
        m_has_attachment = false;
    }
}

bool MessageEmail::getHasAttachment() const
{
    LOGD("MessageEmail::getHasAttachment()");
    return m_has_attachment || !m_body->getInlineAttachments().empty();
}

void MessageEmail::updateEmailMessage(email_mail_data_t& mail)
{
    LOGD("Enter");
    std::vector<std::string> recp_list;

    setId(mail.mail_id);

    setFolderId(mail.mailbox_id);

    setConversationId(mail.thread_id);

    if(mail.full_address_from) {
        setFrom(MessagingUtil::extractSingleEmailAddress(mail.full_address_from));
    }

    if(mail.full_address_to) {
        recp_list = Message::getEmailRecipientsFromStruct(mail.full_address_to);
        setTO(recp_list);
    }

    if(mail.full_address_cc) {
        recp_list = Message::getEmailRecipientsFromStruct(mail.full_address_cc);
        setCC(recp_list);
    }

    if(mail.full_address_bcc) {
        recp_list = Message::getEmailRecipientsFromStruct(mail.full_address_bcc);
        setBCC(recp_list);
    }

    setTimeStamp(mail.date_time);

    setIsRead(mail.flags_seen_field);

    setIsHighPriority((EMAIL_MAIL_PRIORITY_HIGH == mail.priority) ? true : false);

    if (mail.subject == NULL) {
        LOGW("Subject is null");
    } else {
        LOGD("Subject: %s", mail.subject);
        setSubject(mail.subject);
    }

    getBody()->updateBody(mail);

    if (mail.mail_id != mail.thread_id) {
        setInResponseTo(mail.thread_id);
    }

    switch(mail.save_status)
    {
        case EMAIL_MAIL_STATUS_SENT:
            setMessageStatus(MessageStatus::STATUS_SENT);
        break;
        case EMAIL_MAIL_STATUS_SENDING:
            setMessageStatus(MessageStatus::STATUS_SENDING);
        break;
        case EMAIL_MAIL_STATUS_SAVED:
            setMessageStatus(MessageStatus::STATUS_DRAFT);
        break;
        case EMAIL_MAIL_STATUS_SEND_FAILURE:
            setMessageStatus(MessageStatus::STATUS_FAILED);
        break;
        default:
            setMessageStatus(MessageStatus::STATUS_UNDEFINED);
        break;
    }

    AttachmentPtrVector att = convertEmailToMessageAttachment(mail);

    setMessageAttachments(att);
}

} // Messaging
} // DeviceAPI
