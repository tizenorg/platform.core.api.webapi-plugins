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
#include "JSMessageFolder.h"
#include <JSUtil.h>
#include "MessageStorageShortMsg.h"
#include "ShortMsgManager.h"
#include <Logger.h>
#include <PlatformException.h>

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

MessageStorageShortMsg::MessageStorageShortMsg(int id, MessageType msgType):
        MessageStorage(id, msgType) {
    LOGD("Entered");
}

MessageStorageShortMsg::~MessageStorageShortMsg() {
    LOGD("Entered");
}

static gboolean addDraftMessageTask(void* data) {
    LOGD("Entered");

    MessageCallbackUserData *callback = static_cast<MessageCallbackUserData*>(data);
    ShortMsgManager::getInstance().addDraftMessage(callback);

    return false;
}

void MessageStorageShortMsg::addDraftMessage(MessageCallbackUserData* callback) {
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
        LOGE("Incorrect message type");
        throw TypeMismatchException("Incorrect message type");
    }

    guint id = g_idle_add(addDraftMessageTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        throw UnknownException("g_idle_add failed");
    }
}

static gboolean removeMessagesTask(void* data) {
    LOGD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    ShortMsgManager::getInstance().removeMessages(callback);

    return false;
}

void MessageStorageShortMsg::removeMessages(MessagesCallbackUserData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(removeMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        throw UnknownException("g_idle_add failed");
    }
}

static gboolean updateMessagesTask(void* data) {
    LOGD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    ShortMsgManager::getInstance().updateMessages(callback);

    return false;
}

void MessageStorageShortMsg::updateMessages(MessagesCallbackUserData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(updateMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        throw UnknownException("g_idle_add failed");
    }
}

static gboolean findMessagesTask(void* data) {
    LOGD("Entered");

    FindMsgCallbackUserData *callback = static_cast<FindMsgCallbackUserData*>(data);
    ShortMsgManager::getInstance().findMessages(callback);

    return false;
}

void MessageStorageShortMsg::findMessages(FindMsgCallbackUserData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        throw UnknownException("g_idle_add failed");
    }
}

static gboolean findConversationsTask(void* data) {
    LOGD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    ShortMsgManager::getInstance().findConversations(callback);

    return false;
}

void MessageStorageShortMsg::findConversations(ConversationCallbackData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        throw UnknownException("g_idle_add failed");
    }
}

static gboolean removeConversationsTask(void* data) {
    LOGD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    ShortMsgManager::getInstance().removeConversations(callback);

    return false;
}

void MessageStorageShortMsg::removeConversations(ConversationCallbackData* callback)
{
    LOGD("Entered");

    if (!callback) {
        LOGE("Callback is null");
        throw UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(removeConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        throw UnknownException("g_idle_add failed");
    }
}

static gboolean findFoldersCB(void* data)
{
    LOGD("Entered");

    FoldersCallbackData *callback = static_cast<FoldersCallbackData*>(data);

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return FALSE;
    }

    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<FolderPtr,
                JSMessageFolder>(context, callback->getFolders());

    callback->callSuccessCallback(js_obj);

    delete callback;
    callback = NULL;

    return FALSE;
}


void MessageStorageShortMsg::findFolders(FoldersCallbackData* callback)
{
    LOGD("Entered");
    if (!callback){
        LOGE("Callback is null");
        return;
    }

    std::string content_type = MessagingUtil::messageTypeToString(m_msg_type);
    std::string empty = "";
    std::shared_ptr<MessageFolder> folder;

    /* For SMS and MMS, folderId can be one of these values:
     *
     *  INBOX = 1,
     *  OUTBOX = 2,
     *  DRAFTS = 3,
     *  SENTBOX = 4
     */

    for(int i = 1;i < 5;i++)
    {
        folder = std::make_shared<MessageFolder>(
                std::to_string(i),
                empty,
                std::to_string(m_id),
                content_type,
                MessagingUtil::messageFolderTypeToString((MessageFolderType)i),
                empty,
                (MessageFolderType)i,
                false);

        callback->addFolder(folder);
    }

    guint id = g_idle_add(findFoldersCB, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

}//Messaging
}//DeviceAPI
