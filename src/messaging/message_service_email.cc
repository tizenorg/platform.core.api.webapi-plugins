
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "message_service_email.h"
#include "email_manager.h"

#include "common/logger.h"

namespace extension {
namespace messaging {

MessageServiceEmail::MessageServiceEmail(int id, std::string name)
        : MessageService(id,
                MessageType::EMAIL,
                name)
{
    LoggerD("Entered");
}

MessageServiceEmail::~MessageServiceEmail()
{
    LoggerD("Entered");
}

static gboolean sendMessageTask(void* data)
{
    LoggerD("Entered");

    try {
        EmailManager::getInstance().sendMessage(
                static_cast<MessageRecipientsCallbackData*>(data));

    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
             (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }

    return FALSE;
}

void MessageServiceEmail::sendMessage(MessageRecipientsCallbackData *callback)
{
    LoggerD("Entered");

    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {

        LoggerE("Incorrect message type");
        throw common::TypeMismatchException("Incorrect message type");
    }

    callback->setAccountId(m_id);

    guint id = g_idle_add(sendMessageTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add fails");
        delete callback;
        throw common::UnknownException("Could not add task");
    }
}

static gboolean loadMessageBodyTask(void* data)
{
    LoggerD("Entered");
    try {
        EmailManager::getInstance().loadMessageBody(static_cast<MessageBodyCallbackData*>(data));

    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
             (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }

    return FALSE;
}

void MessageServiceEmail::loadMessageBody(MessageBodyCallbackData* callback)
{
    LoggerD("Entered");
    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    guint id = g_idle_add(loadMessageBodyTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        throw common::UnknownException("Could not add task");
    }
}

static gboolean loadMessageAttachmentTask(void* data)
{
    LoggerD("Entered");

    try {
        MessageAttachmentCallbackData *callback =
                static_cast<MessageAttachmentCallbackData *>(data);
        if (!callback) {
            LoggerE("Callback is null");
            throw common::UnknownException("Callback is null");
        }

        std::shared_ptr<MessageAttachment> att =  callback->getMessageAttachment();

        // if the attachment is already saved, then it doesn't need to load again.
        if (att->isFilePathSet() && att->isSaved()){
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            picojson::object args;
            args[JSON_DATA_MESSAGE_ATTACHMENT] = MessagingUtil::messageAttachmentToJson(
                    callback->getMessageAttachment());
            obj[JSON_DATA] = picojson::value(args);

            MessagingInstance::getInstance().PostMessage(json->serialize().c_str());
            delete callback;
            callback = NULL;
            return FALSE;
        }

        EmailManager::getInstance().loadMessageAttachment(
                static_cast<MessageAttachmentCallbackData*>(data));
    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
                (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }
    return FALSE;
}

void MessageServiceEmail::loadMessageAttachment(MessageAttachmentCallbackData *callback)
{
    LoggerD("Entered");
    guint id = g_idle_add(loadMessageAttachmentTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        throw common::UnknownException("Could not add task");
    }
}

static gboolean syncTask(void* data)
{
    LoggerD("Entered");

    try {
        EmailManager::getInstance().sync(data);

    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
             (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }

    return FALSE;
}

long MessageServiceEmail::sync(SyncCallbackData *callback)
{
    LoggerD("Entered");
    if (!callback) {
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    long op_id = EmailManager::getInstance().getUniqueOpId();
    callback->setOpId(op_id);

    guint id = g_idle_add(syncTask, static_cast<void*>(callback));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete callback;
        throw common::UnknownException("Could not add task");
    }
    return op_id;
}

static gboolean syncFolderTask(void* data)
{
    LoggerD("Entered");

    try {
        EmailManager::getInstance().syncFolder(
                static_cast<SyncFolderCallbackData*>(data));

    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
             (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }

    return FALSE;
}

long MessageServiceEmail::syncFolder(SyncFolderCallbackData *callback)
{
    LoggerD("Entered");
    if(!callback){
        LoggerE("Callback is null");
        throw common::UnknownException("Callback is null");
    }

    if(!callback->getMessageFolder()) {
        LoggerE("Message folder is null");
        throw common::TypeMismatchException("Message folder is null");
    }

    long op_id = EmailManager::getInstance().getUniqueOpId();
    callback->setOpId(op_id);

    guint id = g_idle_add(syncFolderTask, callback);
    if (!id) {
        LoggerE("g_idle_add fails");
        delete callback;
    }

    return op_id;
}

static gboolean stopSyncTask(void* data)
{
    LoggerD("Entered");

    try {
        if (!data) {
            LoggerE("opId is null");
            return FALSE;
        }

        const long op_id = *(static_cast<long*>(data));
        delete static_cast<long*>(data);
        data = NULL;
        EmailManager::getInstance().stopSync(op_id);

    } catch(const common::PlatformException& exception) {
        LoggerE("Unhandled exception: %s (%s)!", (exception.name()).c_str(),
             (exception.message()).c_str());
    } catch(...) {
        LoggerE("Unhandled exception!");
    }

    return FALSE;
}

void MessageServiceEmail::stopSync(long op_id)
{
    LoggerD("Entered");
    long* data = new long(op_id);
    guint id = g_idle_add(stopSyncTask, static_cast<void*>(data));
    if (!id) {
        LoggerE("g_idle_add failed");
        delete data;
        data = NULL;
        throw common::UnknownException("Could not add task");
    }
}

} // messaging
} // extension

