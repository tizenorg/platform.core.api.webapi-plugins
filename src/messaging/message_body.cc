// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_body.h"

#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"

namespace extension {
namespace messaging {

using namespace common;

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

PlatformResult MessageBody::updateBody(email_mail_data_t& mail)
{
    LoggerD("Enter");
    setMessageId(mail.mail_id);
    setLoaded(mail.body_download_status);

    if (mail.file_path_plain) {
      LoggerD("Plain body");
      std::string result = "";
      PlatformResult ret = MessagingUtil::loadFileContentToString(mail.file_path_plain,
                                                                  &result);
      if (ret.IsError()) {
        LoggerE("Fail to open plain body.");
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Fail to open plain body.");
      }
      setPlainBody(result);
    }

    if (mail.file_path_html) {
      std::string result = "";
      PlatformResult ret = MessagingUtil::loadFileContentToString(mail.file_path_html,
                                                                  &result);
      if (ret.IsError()) {
        LoggerE("Fail to open html body.");
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Fail to open html body.");
      }
      setHtmlBody(result);
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

} // messaging
} // extension
