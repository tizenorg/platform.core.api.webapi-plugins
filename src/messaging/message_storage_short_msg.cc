// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"
#include "message_sms.h"
#include "short_message_manager.h"
#include "message_storage_short_msg.h"

namespace extension {
namespace messaging {

MessageStorageShortMsg::MessageStorageShortMsg(int id, MessageType msgType):
        MessageStorage(id, msgType) {
    LoggerD("Entered");
}

MessageStorageShortMsg::~MessageStorageShortMsg() {
    LoggerD("Entered");
}

static gboolean addDraftMessageTask(void* data) {
    LoggerD("Entered");

    MessageCallbackUserData *callback = static_cast<MessageCallbackUserData*>(data);
    ShortMsgManager::getInstance().addDraftMessage(callback);

    return false;
}

void MessageStorageShortMsg::addDraftMessage(MessageCallbackUserData* callback) {
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {
        LoggerE("Incorrect message type");
        throw common::TypeMismatchException("Incorrect message type");
    }

    guint id = g_idle_add(addDraftMessageTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        throw common::UnknownException("g_idle_add failed");
    }
}

static gboolean removeMessagesTask(void* data) {
    LoggerD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    ShortMsgManager::getInstance().removeMessages(callback);

    return false;
}

void MessageStorageShortMsg::removeMessages(MessagesCallbackUserData* callback)
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
        throw common::UnknownException("g_idle_add failed");
    }
}

static gboolean updateMessagesTask(void* data) {
    LoggerD("Entered");

    MessagesCallbackUserData *callback = static_cast<MessagesCallbackUserData*>(data);
    ShortMsgManager::getInstance().updateMessages(callback);

    return false;
}

void MessageStorageShortMsg::updateMessages(MessagesCallbackUserData* callback)
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
        throw common::UnknownException("g_idle_add failed");
    }
}

static gboolean findMessagesTask(void* data) {
    LoggerD("Entered");

    FindMsgCallbackUserData *callback = static_cast<FindMsgCallbackUserData*>(data);
    ShortMsgManager::getInstance().findMessages(callback);

    return false;
}

void MessageStorageShortMsg::findMessages(FindMsgCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        throw common::UnknownException("g_idle_add failed");
    }
}

static gboolean findConversationsTask(void* data) {
    LoggerD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    // TODO
    //ShortMsgManager::getInstance().findConversations(callback);

    return false;
}

void MessageStorageShortMsg::findConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        throw common::UnknownException("g_idle_add failed");
    }
}

static gboolean removeConversationsTask(void* data) {
    LoggerD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    // TODO
    //ShortMsgManager::getInstance().removeConversations(callback);

    return false;
}

void MessageStorageShortMsg::removeConversations(ConversationCallbackData* callback)
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
        throw common::UnknownException("g_idle_add failed");
    }
}

static gboolean findFoldersCB(void* data)
{
    LoggerD("Entered");

    FoldersCallbackData *callback = static_cast<FoldersCallbackData*>(data);

    // TODO create json array of folders
    //JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<FolderPtr,
                //JSMessageFolder>(context, callback->getFolders());

    // TODO post success
    //callback->callSuccessCallback(js_obj);

    delete callback;
    callback = NULL;

    return FALSE;
}


void MessageStorageShortMsg::findFolders(FoldersCallbackData* callback)
{
    LoggerD("Entered");
    if (!callback){
        LoggerE("Callback is null");
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
        LoggerE("g_idle_add failed");
        delete callback;
        callback = NULL;
    }
}

} // messaging
} // extension
