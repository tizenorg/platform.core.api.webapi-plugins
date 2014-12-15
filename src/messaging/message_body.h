// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_BODY_H_
#define MESSAGING_MESSAGE_BODY_H_

#include <email-types.h>
#include <vector>
#include <string>

#include "message_attachment.h"

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
    void updateBody(email_mail_data_t& mail);

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
