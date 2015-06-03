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
 
#ifndef MESSAGING_MESSAGE_BODY_H_
#define MESSAGING_MESSAGE_BODY_H_

#include <email-types.h>
#include <vector>
#include <string>

#include "message_attachment.h"
#include "common/platform_result.h"

namespace extension {
namespace messaging {

class MessageBody;

struct MessageBodyHolder {
    std::shared_ptr<MessageBody> ptr;
};

class MessageBody {
public:
    explicit MessageBody();
    ~MessageBody();
    int getMessageId()  const;
    void setMessageId(int value);
    bool getLoaded()  const;
    void setLoaded(bool value);
    std::string getPlainBody()  const;
    void setPlainBody(const std::string &value);
    std::string getHtmlBody()  const;
    void setHtmlBody(const std::string &value);
    AttachmentPtrVector getInlineAttachments()  const;
    void setInlineAttachments(const AttachmentPtrVector& attachments);

    // support for optional, nullable (at JS layer) attibutes
    bool is_message_id_set() const;
    /**
     * Updates body with data from email_mail_data_t structure.
     * @param mail
     */
    common::PlatformResult updateBody(email_mail_data_t& mail);

private:
    int m_messageId;
    bool m_messageId_set;
    bool m_loaded;
    std::string m_plainBody;
    std::string m_htmlBody;
    AttachmentPtrVector m_inlineAttachments;
};

} // messaging
} // extension

#endif // MESSAGING_MESSAGE_BODY_H_
