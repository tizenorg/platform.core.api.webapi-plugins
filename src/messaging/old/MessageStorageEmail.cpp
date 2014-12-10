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

#include "MessageStorageEmail.h"
#include "EmailManager.h"
#include "Message.h"

#include <Logger.h>
#include <PlatformException.h>

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

MessageStorageEmail::MessageStorageEmail(int id):
        MessageStorage(id, MessageType::EMAIL) {
    LOGD("Entered");
}

MessageStorageEmail::~MessageStorageEmail() {
    LOGD("Entered");
}

static gboolean callError(void* data)
{
    LOGD("Entered");
    MessageCallbackUserData* callback =
           static_cast<MessageCallbackUserData*>(data);
    if (!callback) {
       LOGE("Callback is null");
       return FALSE;
    }
    JSContextRef context = callback->getContext();
    JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
            callback->getErrorName(), callback->getErrorMessage());
    callback->callErrorCallback(errobj);

    return FALSE;
}

static gboolean addDraftMessageTask(void* data)
{
    LOGD("Entered");

    MessageCallbackUserData *callback = static_cast<MessageCallbackUserData*>(data);
    EmailManager::getInstance().addDraftMessage(callback);

    return FALSE;
}

void MessageStorageEmail::addDraftMessage(MessageCallbackUserData* callback) {
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
       LOGE("Incorrect message type");
       callback->setError(JSWebAPIErrorFactory::INVALID_VALUES_ERROR,
               "Incorrect message type");
       guint id = g_idle_add(callError, static_cast<void*>(callback));
       if (!id) {
           LOGE("g_idle_add failed");
           delete callback;
           callback = NULL;
       }
       return;
    }

    callback->setAccountId(m_id);

    guint id = g_idle_add(addDraftMessageTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean removeMessagesTask(void* data)
{
    LOGD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    EmailManager::getInstance().removeMessages(callback);

    return FALSE;
}

void MessageStorageEmail::removeMessages(MessagesCallbackUserData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(removeMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean updateMessagesTask(void* data)
{
    LOGD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    EmailManager::getInstance().updateMessages(callback);

    return FALSE;
}

void MessageStorageEmail::updateMessages(MessagesCallbackUserData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);
    guint id = g_idle_add(updateMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean findMessagesTask(void* data)
{
    LOGD("Entered");

    FindMsgCallbackUserData *callback = static_cast<FindMsgCallbackUserData*>(data);
    EmailManager::getInstance().findMessages(callback);

    return FALSE;
}

void MessageStorageEmail::findMessages(FindMsgCallbackUserData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    callback->setAccountId(m_id);
    callback->setMessageServiceType(m_msg_type);
    guint id = g_idle_add(findMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean findConversationsTask(void* data)
{
    LOGD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    EmailManager::getInstance().findConversations(callback);

    return FALSE;
}

void MessageStorageEmail::findConversations(ConversationCallbackData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    callback->setAccountId(m_id);
    callback->setMessageServiceType(m_msg_type);
    guint id = g_idle_add(findConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}
static gboolean removeConversationsTask(void* data)
{
    LOGD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    EmailManager::getInstance().removeConversations(callback);

    return FALSE;
}

void MessageStorageEmail::removeConversations(ConversationCallbackData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);
    guint id = g_idle_add(removeConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean findFoldersTask(void* data)
{
    LOGD("Entered");

    FoldersCallbackData *callback = static_cast<FoldersCallbackData*>(data);
    EmailManager::getInstance().findFolders(callback);

    return FALSE;
}

void MessageStorageEmail::findFolders(FoldersCallbackData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    guint id = g_idle_add(findFoldersTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

}//Messaging
}//DeviceAPI
