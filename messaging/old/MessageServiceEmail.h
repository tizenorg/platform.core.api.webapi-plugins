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

#ifndef __TIZEN_MESSAGE_SERVICE_EMAIL_H__
#define __TIZEN_MESSAGE_SERVICE_EMAIL_H__

#include "MessageService.h"
#include "MessagingUtil.h"

namespace DeviceAPI {
namespace Messaging {

class MessageServiceEmail : public MessageService {
public:
    MessageServiceEmail(int id, std::string name);
    virtual ~MessageServiceEmail();

    virtual void sendMessage(MessageRecipientsCallbackData *callback);
    virtual void loadMessageBody(MessageBodyCallbackData *callback);
    virtual void loadMessageAttachment(MessageAttachmentCallbackData *callback);
    virtual long sync(SyncCallbackData *callback);

   /**
     * @param callback - owned by this method unless exception is thrown
     * @return opId - "long Identifier which can be used to stop this service operation"
     *                (form JS documentation)
     *
     */
    virtual long syncFolder(SyncFolderCallbackData *callback);

    virtual void stopSync(long op_id);
};

} // Messaging
} // DeviceAPI
#endif // __TIZEN_MESSAGE_SERVICE_EMAIL_H__
