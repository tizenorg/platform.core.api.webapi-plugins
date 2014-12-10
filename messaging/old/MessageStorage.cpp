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

#include "MessageStorage.h"
#include "ChangeListenerContainer.h"
#include "MessagesChangeCallback.h"
#include "ConversationsChangeCallback.h"
#include "FoldersChangeCallback.h"
#include <Logger.h>

namespace DeviceAPI {
namespace Messaging {

MessageStorage::MessageStorage(int id, MessageType msgType):
        SecurityAccessor(),
        m_id(id),
        m_msg_type(msgType)
{
    LOGD("Entered");
}

MessageStorage::~MessageStorage()
{
    LOGD("Entered");
}

int MessageStorage::getMsgServiceId() const {
    return m_id;
}

MessageType MessageStorage::getMsgServiceType() const {
    return m_msg_type;
}

long MessageStorage::addMessagesChangeListener(
            std::shared_ptr<MessagesChangeCallback> callback)
{
    LOGD("Entered");
    return ChangeListenerContainer::getInstance().addMessageChangeListener(callback);
}

long MessageStorage::addConversationsChangeListener(
            std::shared_ptr<ConversationsChangeCallback> callback)
{
    LOGD("Entered");
    return ChangeListenerContainer::getInstance().addConversationChangeListener(callback);
}

long MessageStorage::addFoldersChangeListener(
            std::shared_ptr<FoldersChangeCallback> callback)
{
    LOGD("Entered");
    return ChangeListenerContainer::getInstance().addFolderChangeListener(callback);
}

void MessageStorage::removeChangeListener(JSContextRef context, long watchId)
{
    LOGD("Entered");
    return ChangeListenerContainer::getInstance().removeChangeListener(context, watchId);
}

}//Messaging
}//DeviceAPI
