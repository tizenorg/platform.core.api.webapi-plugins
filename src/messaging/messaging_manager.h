// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_MANAGER_H_
#define MESSAGING_MESSAGING_MANAGER_H_

#include <msg.h>
#include <string>

namespace extension {
namespace messaging {

class MessagingManager {
public:
    static MessagingManager& getInstance();
    void getMessageServices(const std::string& type, double callbackId);
private:
    MessagingManager();
    MessagingManager(const MessagingManager &);
    void operator=(const MessagingManager &);
    virtual ~MessagingManager();

    msg_handle_t m_msg_handle;
};

} // namespace messaging
} // namespace extension

#endif // MESSAGING_MESSAGING_MANAGER_H_

