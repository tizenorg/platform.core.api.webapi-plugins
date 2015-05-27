// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_storage.h"
#include "messages_change_callback.h"
#include "conversations_change_callback.h"
#include "folders_change_callback.h"
#include "change_listener_container.h"

#include "common/logger.h"

namespace extension {
namespace messaging {

MessageStorage::MessageStorage(int id, MessageType msgType) :
        m_id(id), m_msg_type(msgType)
{
    LoggerD("Entered");
}

MessageStorage::~MessageStorage() {
  LoggerD("Entered");

  for (auto id : registered_listeners_) {
    ChangeListenerContainer::getInstance().removeChangeListener(id);
  }
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

std::string MessageStorage::getMsgServiceTypeString() const {
  return MessagingUtil::messageTypeToString(getMsgServiceType());
}

long MessageStorage::addMessagesChangeListener(std::shared_ptr<MessagesChangeCallback> callback)
{
    LoggerD("Entered");
    long id = ChangeListenerContainer::getInstance().addMessageChangeListener(callback);
    registered_listeners_.insert(id);
    return id;
}

long MessageStorage::addConversationsChangeListener(
        std::shared_ptr<ConversationsChangeCallback> callback)
{
    LoggerD("Entered");
    long id = ChangeListenerContainer::getInstance().addConversationChangeListener(callback);
    registered_listeners_.insert(id);
    return id;
}

long MessageStorage::addFoldersChangeListener(std::shared_ptr<FoldersChangeCallback> callback)
{
    LoggerD("Entered");
    long id = ChangeListenerContainer::getInstance().addFolderChangeListener(callback);
    registered_listeners_.insert(id);
    return id;
}

void MessageStorage::removeChangeListener(long watchId)
{
    LoggerD("Entered");
    ChangeListenerContainer::getInstance().removeChangeListener(watchId);
    registered_listeners_.erase(watchId);
}

} //messaging
} //extension
