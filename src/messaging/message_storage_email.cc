// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
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

static gboolean callError(void* data)
{
    LoggerD("Entered");
    MessageCallbackUserData* callback =
           static_cast<MessageCallbackUserData*>(data);
    if (!callback) {
       LoggerE("Callback is null");
       return FALSE;
    }

    MessagingInstance::getInstance().PostMessage(callback->getJson()->serialize().c_str());

    return FALSE;
}

void MessageStorageEmail::addDraftMessage(MessageCallbackUserData* callback) {
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
       LoggerE("Incorrect message type");
       callback->setError("UnknownError", "Incorrect message type");
       guint id = g_idle_add(callError, static_cast<void*>(callback));
       if (!id) {
           LoggerE("g_idle_add failed");
           delete callback;
           callback = NULL;
       }
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
        throw common::UnknownException("Callback is null");
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
        throw common::UnknownException("Callback is null");
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
        throw common::UnknownException("Callback is null");
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
        throw common::UnknownException("Callback is null");
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
        throw common::UnknownException("Callback is null");
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
        throw common::UnknownException("Callback is null");
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
