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

#ifndef __TIZEN_MESSAGE_MMS_H__
#define __TIZEN_MESSAGE_MMS_H__

// Header with core msg-service declarations
#include <msg.h>

#include "message.h"
#include "message_attachment.h"

namespace extension {
namespace messaging {

class MessageMMS: public Message {
public:
// constructor
    MessageMMS();
    ~MessageMMS();

//overrided base class functions
    void setCC(std::vector<std::string> &cc);
    void setBCC(std::vector<std::string> &bcc);
    void setSubject(std::string subject);
    void setMessageAttachments(AttachmentPtrVector &attachments);

    bool getHasAttachment() const;
};

} // messaging
} // extension

#endif // __TIZEN_MESSAGE_MMS_H__
