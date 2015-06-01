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
 
#include "message_service_email.h"
#include "email_manager.h"

#include "common/logger.h"

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {

MessageServiceEmail::MessageServiceEmail(int id, std::string name)
        : MessageService(id,
                MessageType::EMAIL,
                name)
{
    LoggerD("Entered");
}

MessageServiceEmail::~MessageServiceEmail() {
  LoggerD("Entered");

  for (auto id : registered_callbacks_) {
    // this may internally fail, because we don't have information about
    // callbacks which already have fired
    EmailManager::getInstance().RemoveSyncCallback(id);
  }
}

static gboolean sendMessageTask(void* data)
{
  LoggerD("Entered");

  auto ret = EmailManager::getInstance().sendMessage(static_cast<MessageRecipientsCallbackData*>(data));

  if (!ret) {
    LoggerE("Error: %d - %s", ret.error_code(), ret.message().c_str());
  }

  return FALSE;
}

PlatformResult MessageServiceEmail::sendMessage(MessageRecipientsCallbackData *callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
    }

    callback->setAccountId(m_id);

    guint id = g_idle_add(sendMessageTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add fails");
        delete callback;
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean loadMessageBodyTask(void* data)
{
    LoggerD("Entered");

    EmailManager::getInstance().loadMessageBody(static_cast<MessageBodyCallbackData*>(data));

    return FALSE;
}

PlatformResult MessageServiceEmail::loadMessageBody(MessageBodyCallbackData* callback)
{
    LoggerD("Entered");
    if (!callback) {
        LoggerE("Callback is null");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
    }

    guint id = g_idle_add(loadMessageBodyTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean loadMessageAttachmentTask(void* data)
{
  LoggerD("Entered");

  auto callback = static_cast<MessageAttachmentCallbackData*>(data);

  if (callback) {
    auto att = callback->getMessageAttachment();

    // if the attachment is already saved, then it doesn't need to load again.
    if (att->isFilePathSet() && att->isSaved()) {
      auto json = callback->getJson();
      picojson::object& obj = json->get<picojson::object>();
      obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

      if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
        picojson::object args;
        args[JSON_DATA_MESSAGE_ATTACHMENT] = MessagingUtil::messageAttachmentToJson(att);
        obj[JSON_DATA] = picojson::value(args);

        callback->getQueue().resolve(
            obj.at(JSON_CALLBACK_ID).get<double>(),
            json->serialize()
        );
      } else {
        LoggerE("json is incorrect - missing required member");
      }
      delete callback;
      callback = nullptr;
    } else {
      const auto ret = EmailManager::getInstance().loadMessageAttachment(callback);

      if (!ret) {
        LoggerE("Error: %d - %s", ret.error_code(), ret.message().c_str());
      }
    }
  } else {
    LoggerE("Callback is null");
  }

  return FALSE;
}

PlatformResult MessageServiceEmail::loadMessageAttachment(MessageAttachmentCallbackData *callback)
{
  LoggerD("Entered");
  guint id = g_idle_add(loadMessageAttachmentTask, static_cast<void*>(callback));
  if (!id) {
    LoggerE("g_idle_add failed");
    delete callback;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean syncTask(void* data)
{
  LoggerD("Entered");

  EmailManager::getInstance().sync(data);

  return FALSE;
}

PlatformResult MessageServiceEmail::sync(SyncCallbackData *callback, long* operation_id)
{
  LoggerD("Entered");

  if (!callback) {
    LoggerE("Callback is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
  }

  long op_id = EmailManager::getInstance().getUniqueOpId();
  callback->setOpId(op_id);

  guint id = g_idle_add(syncTask, static_cast<void*>(callback));
  if (!id) {
    LoggerE("g_idle_add failed");
    delete callback;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
  }
  *operation_id = op_id;
  registered_callbacks_.insert(op_id);
  return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean syncFolderTask(void* data)
{
    LoggerD("Entered");

    EmailManager::getInstance().syncFolder(static_cast<SyncFolderCallbackData*>(data));

    return FALSE;
}

PlatformResult MessageServiceEmail::syncFolder(SyncFolderCallbackData *callback, long* operation_id)
{
  LoggerD("Entered");
  if (!callback) {
    LoggerE("Callback is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
  }

  if (!callback->getMessageFolder()) {
    LoggerE("Message folder is null");
    delete callback;
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Message folder is null");
  }

  long op_id = EmailManager::getInstance().getUniqueOpId();
  callback->setOpId(op_id);

  guint id = g_idle_add(syncFolderTask, callback);
  if (!id) {
    LoggerE("g_idle_add fails");
    delete callback;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
  }
  *operation_id = op_id;
  registered_callbacks_.insert(op_id);
  return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean stopSyncTask(void* data)
{
  LoggerD("Entered");

  if (!data) {
    LoggerE("opId is null");
    return FALSE;
  }

  const long op_id = *(static_cast<long*>(data));
  delete static_cast<long*>(data);
  data = NULL;
  EmailManager::getInstance().stopSync(op_id);

  return FALSE;
}

PlatformResult MessageServiceEmail::stopSync(long op_id)
{
  LoggerD("Entered");

  registered_callbacks_.erase(op_id);
  long* data = new long(op_id);
  guint id = g_idle_add(stopSyncTask, static_cast<void*>(data));

  if (!id) {
    LoggerE("g_idle_add failed");
    delete data;
    data = NULL;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add task");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

} // messaging
} // extension

