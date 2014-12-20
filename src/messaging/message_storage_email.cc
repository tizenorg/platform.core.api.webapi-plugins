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
