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
 
#include "message_storage_email.h"

#include <glib.h>

#include "common/platform_exception.h"

#include "email_manager.h"
#include "message.h"

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

static gboolean addDraftMessageTask(void* data)
{
    LoggerD("Entered");

    MessageCallbackUserData *callback = static_cast<MessageCallbackUserData*>(data);
    EmailManager::getInstance().addDraftMessage(callback);

    return FALSE;
}

void MessageStorageEmail::addDraftMessage(MessageCallbackUserData* callback) {
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setAccountId(m_id);

    guint id = g_idle_add(addDraftMessageTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean removeMessagesTask(void* data)
{
    LoggerD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    EmailManager::getInstance().removeMessages(callback);

    return FALSE;
}

void MessageStorageEmail::removeMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(removeMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean updateMessagesTask(void* data)
{
    LoggerD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    EmailManager::getInstance().updateMessages(callback);

    return FALSE;
}

void MessageStorageEmail::updateMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setMessageServiceType(m_msg_type);
    guint id = g_idle_add(updateMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean findMessagesTask(void* data)
{
    LoggerD("Entered");

    FindMsgCallbackUserData *callback = static_cast<FindMsgCallbackUserData*>(data);
    EmailManager::getInstance().findMessages(callback);

    return FALSE;
}

void MessageStorageEmail::findMessages(FindMsgCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setAccountId(m_id);
    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean findConversationsTask(void* data)
{
    LoggerD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    EmailManager::getInstance().findConversations(callback);

    return FALSE;
}

void MessageStorageEmail::findConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setAccountId(m_id);
    callback->setMessageServiceType(m_msg_type);
    guint id = g_idle_add(findConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean removeConversationsTask(void* data)
{
    LoggerD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    EmailManager::getInstance().removeConversations(callback);

    return FALSE;
}

void MessageStorageEmail::removeConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(removeConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

static gboolean findFoldersTask(void* data)
{
    LoggerD("Entered");

    FoldersCallbackData *callback = static_cast<FoldersCallbackData*>(data);
    EmailManager::getInstance().findFolders(callback);

    return FALSE;
}

void MessageStorageEmail::findFolders(FoldersCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    guint id = g_idle_add(findFoldersTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

} //messaging
} //extension
