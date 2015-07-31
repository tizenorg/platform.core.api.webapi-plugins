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
    LoggerD("Entered");
}

MessageBody::~MessageBody()
{
    LoggerD("Entered");
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
    LoggerD("Entered");
    setMessageId(mail.mail_id);
    setLoaded(mail.body_download_status);

    if (mail.file_path_plain) {
      SLoggerD("Plain body: %s", mail.file_path_plain);
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
      SLoggerD("HTML body: %s", mail.file_path_html);
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
