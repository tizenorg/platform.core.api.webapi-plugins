
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "message_service_email.h"

#include "common/logger.h"

namespace extension {
namespace messaging {

MessageServiceEmail::MessageServiceEmail(int id, std::string name)
        : MessageService(id,
                MessageType::EMAIL,
                name)
{
    LoggerD("Entered");
}

MessageServiceEmail::~MessageServiceEmail()
{
    LoggerD("Entered");
}

void MessageServiceEmail::sendMessage()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageServiceEmail::loadMessageBody()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageServiceEmail::loadMessageAttachment()
{
    LoggerD("Entered");
    //TODO add implementation
}

long MessageServiceEmail::sync(const double callbackId, long limit)
{
    LoggerD("Entered");
    //TODO add implementation
    return 0;
}

long MessageServiceEmail::syncFolder()
{
    LoggerD("Entered");
    //TODO add implementation
    return 0;
}

void MessageServiceEmail::stopSync()
{
    LoggerD("Entered");
    //TODO add implementation
}

} // extension
} // messaging

