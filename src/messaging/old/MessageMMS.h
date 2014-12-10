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
 * @file        MessageMMS.h
 */

#ifndef __TIZEN_MESSAGE_MMS_H__
#define __TIZEN_MESSAGE_MMS_H__

// Header with core msg-service declarations
#include <msg.h>

#include "Message.h"
#include "MessageAttachment.h"

namespace DeviceAPI {
namespace Messaging {

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

} // Messaging
} // DeviceAPI

#endif // __TIZEN_MESSAGE_MMS_H__
