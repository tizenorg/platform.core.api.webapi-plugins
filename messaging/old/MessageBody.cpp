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
 * @file        MessageBody.cpp
 */

#include "MessageBody.h"
#include "MessagingUtil.h"
#include <Logger.h>
#include <PlatformException.h>

namespace DeviceAPI {
namespace Messaging {

MessageBody::MessageBody() : m_messageId(1),
                             m_messageId_set(false),
                             m_loaded(false),
                             m_plainBody(""),
                             m_htmlBody("")
{
}

MessageBody::~MessageBody()
{
}

// messageId

int MessageBody::getMessageId() const
{

    return m_messageId;
}

void MessageBody::setMessageId(int value)
{
    m_messageId = value;
    m_messageId_set = true;
}

// loaded

bool MessageBody::getLoaded() const
{
    return m_loaded;
}

void MessageBody::setLoaded(bool value)
{
    m_loaded = value;
}

// plainBody

std::string MessageBody::getPlainBody() const
{
    return m_plainBody;
}

void MessageBody::setPlainBody(const std::string &value)
{
    m_plainBody = value;
}

// htmlBody

std::string MessageBody::getHtmlBody() const
{
    return m_htmlBody;
}

void MessageBody::setHtmlBody(const std::string &value)
{
    m_htmlBody = value;
}

// inlineAttachments

AttachmentPtrVector MessageBody::getInlineAttachments() const
{
    return m_inlineAttachments;
}

JSObjectRef MessageBody::getJSInlineAttachments(JSContextRef global_ctx)
{
    return m_inlineAttachments.getJSArray(global_ctx);
}

void MessageBody::setInlineAttachments(const AttachmentPtrVector& attachments)
{
    m_inlineAttachments = attachments;
}

// ***  support for optional, nullable (at JS layer) attibutes
bool MessageBody::is_message_id_set() const
{
    return m_messageId_set;
}

void MessageBody::updateBody(email_mail_data_t& mail)
{
    LOGD("Enter");
    setMessageId(mail.mail_id);
    setLoaded(mail.body_download_status);

    if (mail.file_path_plain) {
        try {
            LOGD("Plain body");
            setPlainBody(MessagingUtil::loadFileContentToString(mail.file_path_plain));
        } catch (...) {
            LOGE("Fail to open plain body.");
            throw Common::UnknownException("Fail to open plain body.");
        }
    }

    if (mail.file_path_html) {
        try {
            LOGD("Html body");
            setHtmlBody(MessagingUtil::loadFileContentToString(mail.file_path_html));
        } catch (...) {
            LOGE("Fail to open html body.");
            throw Common::UnknownException("Fail to open html body.");
        }
    }
}

} // Messaging
} // DeviceAPI
