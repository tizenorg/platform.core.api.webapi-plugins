// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_body.h"

#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"

namespace extension {
namespace messaging {

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
    LoggerD("Enter");
    setMessageId(mail.mail_id);
    setLoaded(mail.body_download_status);

    if (mail.file_path_plain) {
        try {
            LoggerD("Plain body");
            setPlainBody(MessagingUtil::loadFileContentToString(mail.file_path_plain));
        } catch (...) {
            LoggerE("Fail to open plain body.");
            throw common::UnknownException("Fail to open plain body.");
        }
    }

    if (mail.file_path_html) {
        try {
            LoggerD("Html body");
            setHtmlBody(MessagingUtil::loadFileContentToString(mail.file_path_html));
        } catch (...) {
            LoggerE("Fail to open html body.");
            throw common::UnknownException("Fail to open html body.");
        }
    }
}

} // messaging
} // extension
