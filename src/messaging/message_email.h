// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_EMAIL_H_
#define MESSAGING_MESSAGE_EMAIL_H_

#include "message.h"

namespace extension {
namespace messaging {

class MessageEmail: public Message {
public:
// constructor
    MessageEmail();
    ~MessageEmail();

//overrided base class functions
    void setCC(std::vector<std::string> &cc);
    void setBCC(std::vector<std::string> &bcc);
    void setSubject(std::string subject);
    void setIsHighPriority(bool highpriority);

    void setMessageAttachments(AttachmentPtrVector &attachments);

    bool getHasAttachment() const;
    /**
     * Updates message with data from email_mail_data_t structure.
     * @param mail
     */
    virtual common::PlatformResult updateEmailMessage(email_mail_data_t& mail);

private:
    // function that verifies recipient's list validity
    bool isValidRecpientsVector(std::vector<std::string> &recipients);
};

} // messaging
} // extension

#endif // MESSAGING_MESSAGE_EMAIL_H_
