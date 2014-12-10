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
 * @file        MessageMMS.cpp
 */

#include <PlatformException.h>
#include <Logger.h>
#include "MessageMMS.h"
#include <GlobalContextManager.h>

namespace DeviceAPI {
namespace Messaging {

MessageMMS::MessageMMS():
    Message()
{
    LOGD("MessageMMS constructor.");
    this->m_type = MessageType(MessageType(MMS));
}

MessageMMS::~MessageMMS()
{
    LOGD("MessageMMS destructor.");
}

// *** overrided methods
void MessageMMS::setCC(std::vector<std::string> &cc)
{
    // CC recipient's format validation should be done by email service
    m_cc = cc;

    if(m_cc.empty()) {
        LOGD("Recipient's list cleared");
        return;
    }
}

void MessageMMS::setBCC(std::vector<std::string> &bcc)
{
    // BCC recipient's format validation should be done by email service
    m_bcc = bcc;

    if(m_bcc.empty()) {
        LOGD("Recipient's list cleared");
        return;
    }
}

void MessageMMS::setSubject(std::string subject)
{
    m_subject = subject;
}

void MessageMMS::setMessageAttachments(AttachmentPtrVector &attachments)
{
    m_attachments = attachments;

    m_has_attachment = true;
    if(m_attachments.empty()) {
        LOGD("Recipient's list cleared");
        m_has_attachment = false;
    }
}

bool MessageMMS::getHasAttachment() const
{
    LOGD("MessageMMS::getHasAttachment()");
    // TODO: Analyze relation between hasAttachment flag and inlineAttachments
    return m_has_attachment;
}

} // Messaging
} // DeviceAPI
