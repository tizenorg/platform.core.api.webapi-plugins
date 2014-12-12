
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_SERVICE_H_
#define MESSAGING_MESSAGE_SERVICE_H_

#include <memory>
#include <string>

#include "common/picojson.h"

#include "messaging_util.h"

namespace extension {
namespace messaging {

enum MessageServiceAccountId
{
    UNKNOWN_ACCOUNT_ID = 0,
    SMS_ACCOUNT_ID = 101,
    MMS_ACCOUNT_ID = 102
};

class MessageService
{
public:
    virtual ~MessageService();

    virtual int getMsgServiceId() const;
    virtual std::string getMsgServiceIdStr() const;
    virtual MessageType getMsgServiceType() const;
    virtual std::string getMsgServiceName() const;
    // FIXME MessageStorage
    // virtual std::shared_ptr<MessageStorage> getMsgStorage() const;

    virtual void sendMessage();
    virtual void loadMessageBody();
    virtual void loadMessageAttachment();
    virtual long sync();
    virtual long syncFolder();
    virtual void stopSync();

    picojson::object toPicoJS() const;

protected:
    /**
     * We have child classes MessageServiceEmail and MessageServiceShortMsg which
     * should provide specialized implementation.
     */
    MessageService(int id,
            MessageType msgType,
            std::string name);

    int m_id;
    MessageType m_msg_type;
    std::string m_name;
    //FIXME
    //std::shared_ptr<MessageStorage> m_storage;
};

} // messaging
} // extension
#endif // MESSAGING_MESSAGE_SERVICE_EMAIL_H_
