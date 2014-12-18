
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_SERVICE_EMAIL_H_
#define MESSAGING_MESSAGE_SERVICE_EMAIL_H_

#include "message_service.h"

namespace extension {
namespace messaging {

class MessageServiceEmail : public MessageService {
public:
    MessageServiceEmail(int id, std::string name);
    virtual ~MessageServiceEmail();

    virtual void sendMessage(MessageRecipientsCallbackData *callback);
    virtual void loadMessageBody(MessageBodyCallbackData* callback);
    virtual void loadMessageAttachment();
    virtual long sync(SyncCallbackData *callback);
    virtual long syncFolder();
    virtual void stopSync(long op_id);
};

} // messaging
} // extension

#endif // MESSAGING_MESSAGE_SERVICE_EMAIL_H_
