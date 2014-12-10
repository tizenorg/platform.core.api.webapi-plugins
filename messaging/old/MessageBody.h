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
 * @file        MessageBody.h
 */

#ifndef __TIZEN_MESSAGING_MESSAGE_BODY_H__
#define __TIZEN_MESSAGING_MESSAGE_BODY_H__

#include <vector>
#include <string>
#include "MessageAttachment.h"
#include <email-types.h>

namespace DeviceAPI {
namespace Messaging {

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
    JSObjectRef getJSInlineAttachments(JSContextRef global_ctx);
    void setInlineAttachments(const AttachmentPtrVector& attachments);

    // support for optional, nullable (at JS layer) attibutes
    bool is_message_id_set() const;
    /**
     * Updates body with data from email_mail_data_t structure.
     * @param mail
     */
    void updateBody(email_mail_data_t& mail);

private:
    int m_messageId;
    bool m_messageId_set;
    bool m_loaded;
    std::string m_plainBody;
    std::string m_htmlBody;
    JSAttachmentsVector m_inlineAttachments;
    JSContextRef m_context;
};

} // Messaging
} // DeviceAPI

#endif // __TIZEN_MESSAGING_MESSAGE_BODY_H__
