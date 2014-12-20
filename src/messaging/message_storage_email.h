// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_STORAGE_EMAIL_H_
#define MESSAGING_MESSAGE_STORAGE_EMAIL_H_

#include "message_storage.h"

namespace extension {
namespace messaging {

class MessageStorageEmail: public MessageStorage {
public:
    MessageStorageEmail(int id);
    virtual ~MessageStorageEmail();

    virtual void addDraftMessage(MessageCallbackUserData* callback);
    virtual void removeMessages(MessagesCallbackUserData* callback);
    virtual void updateMessages();
    virtual void findMessages();
    virtual void findConversations();
    virtual void removeConversations();
    virtual void findFolders();
};

} //messaging
} //extension

#endif /* MESSAGING_MESSAGE_STORAGE_EMAIL_H_ */

