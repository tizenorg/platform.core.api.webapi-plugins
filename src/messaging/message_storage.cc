// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_storage.h"
#include "messages_change_callback.h"
#include "change_listener_container.h"

#include "common/logger.h"

namespace extension {
namespace messaging {

MessageStorage::MessageStorage(int id, MessageType msgType) :
        m_id(id), m_msg_type(msgType)
{
    LoggerD("Entered");
}

MessageStorage::~MessageStorage()
{
    LoggerD("Entered");
}

int MessageStorage::getMsgServiceId() const
{
    LoggerD("Entered");
    return m_id;
}

MessageType MessageStorage::getMsgServiceType() const
{
    LoggerD("Entered");
    return m_msg_type;
}

long MessageStorage::addMessagesChangeListener(std::shared_ptr<MessagesChangeCallback> callback)
{
    LoggerD("Entered");
    return ChangeListenerContainer::getInstance().addMessageChangeListener(callback);
}

long MessageStorage::addConversationsChangeListener()
{
    LoggerD("Entered");
}

long MessageStorage::addFoldersChangeListener()
{
    LoggerD("Entered");
}

void MessageStorage::removeChangeListener()
{
    LoggerD("Entered");
}

} //messaging
} //extension
