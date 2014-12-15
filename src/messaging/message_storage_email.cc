// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_storage_email.h"

namespace extension {
namespace messaging {

MessageStorageEmail::MessageStorageEmail(int id) :
        MessageStorage(id, MessageType::EMAIL)
{
    LoggerD("Entered");
}

MessageStorageEmail::~MessageStorageEmail()
{
    LoggerD("Entered");
}

void MessageStorageEmail::addDraftMessage()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageStorageEmail::removeMessages()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageStorageEmail::updateMessages()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageStorageEmail::findMessages()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageStorageEmail::findConversations()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageStorageEmail::removeConversations()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageStorageEmail::findFolders()
{
    LoggerD("Entered");
    //TODO add implementation
}

} //messaging
} //extension
