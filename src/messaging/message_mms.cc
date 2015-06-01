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
 
#include "message_mms.h"

#include "common/platform_exception.h"
#include "common/logger.h"

namespace extension {
namespace messaging {

MessageMMS::MessageMMS():
    Message()
{
    LoggerD("MessageMMS constructor.");
    this->m_type = MessageType(MessageType(MMS));
}

MessageMMS::~MessageMMS()
{
    LoggerD("MessageMMS destructor.");
}

// *** overrided methods
void MessageMMS::setCC(std::vector<std::string> &cc)
{
    LoggerD("Entered");
    // CC recipient's format validation should be done by email service
    m_cc = cc;

    if(m_cc.empty()) {
        LoggerD("Recipient's list cleared");
        return;
    }
}

void MessageMMS::setBCC(std::vector<std::string> &bcc)
{
    LoggerD("Entered");
    // BCC recipient's format validation should be done by email service
    m_bcc = bcc;

    if(m_bcc.empty()) {
        LoggerD("Recipient's list cleared");
        return;
    }
}

void MessageMMS::setSubject(std::string subject)
{
    m_subject = subject;
}

void MessageMMS::setMessageAttachments(AttachmentPtrVector &attachments)
{
    LoggerD("Entered");
    m_attachments = attachments;

    m_has_attachment = true;
    if(m_attachments.empty()) {
        LoggerD("Recipient's list cleared");
        m_has_attachment = false;
    }
}

bool MessageMMS::getHasAttachment() const
{
    LoggerD("MessageMMS::getHasAttachment()");
    // TODO: Analyze relation between hasAttachment flag and inlineAttachments
    return m_has_attachment;
}

} // messaging
} // extension
