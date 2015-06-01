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
 
#include "common/logger.h"
#include "common/platform_exception.h"

#include "messaging_util.h"
#include "message_sms.h"
#include "short_message_manager.h"
#include "message_storage_short_msg.h"
#include "messaging_instance.h"

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
        return;
    }

    guint id = g_idle_add(addDraftMessageTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        return;
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
        return;
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(removeMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        return;
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
        return;
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(updateMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        return;
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
        return;
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findMessagesTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        return;
    }
}

static gboolean findConversationsTask(void* data) {
    LoggerD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    ShortMsgManager::getInstance().findConversations(callback);

    return false;
}

void MessageStorageShortMsg::findConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    callback->setMessageServiceType(m_msg_type);

    guint id = g_idle_add(findConversationsTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        return;
    }
}

static gboolean removeConversationsTask(void* data) {
    LoggerD("Entered");

    ConversationCallbackData *callback = static_cast<ConversationCallbackData*>(data);
    ShortMsgManager::getInstance().removeConversations(callback);

    return false;
}

void MessageStorageShortMsg::removeConversations(ConversationCallbackData* callback)
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
        return;
    }
}

static gboolean findFoldersCB(void* data)
{
    LoggerD("Entered");

    FoldersCallbackData *callback = static_cast<FoldersCallbackData*>(data);

    auto json = callback->getJson();
    picojson::object& obj = json->get<picojson::object>();

    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      picojson::array array;
      auto each = [&array](std::shared_ptr<MessageFolder> folder)->void {
        array.push_back(MessagingUtil::folderToJson(folder));
      };

      auto folders = callback->getFolders();
      for_each(folders.begin(), folders.end(), each);

      obj[JSON_DATA] = picojson::value(array);
      obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);


     callback->getQueue().resolve(
          obj.at(JSON_CALLBACK_ID).get<double>(),
          json->serialize()
      );
    } else {
      LoggerE("json is incorrect - missing required member");
    }
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

    std::string content_type = getMsgServiceTypeString();
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
