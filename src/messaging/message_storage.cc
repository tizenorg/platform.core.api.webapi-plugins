/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
 
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

std::string MessageStorage::getMsgServiceTypeString() const {
  return MessagingUtil::messageTypeToString(getMsgServiceType());
}

long MessageStorage::addMessagesChangeListener(std::shared_ptr<MessagesChangeCallback> callback)
{
    LoggerD("Entered");
    return ChangeListenerContainer::getInstance().addMessageChangeListener(callback);
}

long MessageStorage::addConversationsChangeListener(
        std::shared_ptr<ConversationsChangeCallback> callback)
{
    LoggerD("Entered");
    return ChangeListenerContainer::getInstance().addConversationChangeListener(callback);
}

long MessageStorage::addFoldersChangeListener(std::shared_ptr<FoldersChangeCallback> callback)
{
    LoggerD("Entered");
    return ChangeListenerContainer::getInstance().addFolderChangeListener(callback);
}

void MessageStorage::removeChangeListener(long watchId)
{
    LoggerD("Entered");
    return ChangeListenerContainer::getInstance().removeChangeListener(watchId);
}

} //messaging
} //extension
