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
 * @file        MessageEmail.h
 */


#ifndef __TIZEN_MESSAGE_EMAIL_H__
#define __TIZEN_MESSAGE_EMAIL_H__

#include "Message.h"
#include "MessageAttachment.h"

namespace DeviceAPI {
namespace Messaging {

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
    virtual void updateEmailMessage(email_mail_data_t& mail);

private:
    // function that verifies recipient's list validity
    bool isValidRecpientsVector(std::vector<std::string> &recipients);
};

} // Messaging
} // DeviceAPI

#endif // __TIZEN_MESSAGE_EMAIL_H__
