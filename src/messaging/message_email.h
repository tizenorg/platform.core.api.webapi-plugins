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
