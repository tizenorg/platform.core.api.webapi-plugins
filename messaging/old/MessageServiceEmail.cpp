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

#include <JSWebAPIError.h>
#include <JSUtil.h>
#include <Logger.h>
#include <PlatformException.h>

#include "MessageServiceEmail.h"
#include "EmailManager.h"

namespace DeviceAPI {
namespace Messaging {

MessageServiceEmail::MessageServiceEmail(int id, std::string name)
        : MessageService(id,
                MessageType::EMAIL,
                name)
{
    LOGD("Entered");
}

MessageServiceEmail::~MessageServiceEmail()
{
    LOGD("Entered");
}

static gboolean sendMessageTask(void* data)
{
    LOGD("Entered");

    try {
        EmailManager::getInstance().sendMessage(
                static_cast<MessageRecipientsCallbackData*>(data));

    } catch(const Common::BasePlatformException& exception) {
        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
             (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }

    return FALSE;
}

void MessageServiceEmail::sendMessage(MessageRecipientsCallbackData *callback)
{
    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    if (m_msg_type != callback->getMessage()->getType()) {

        LOGE("Incorrect message type");
        throw Common::TypeMismatchException("Incorrect message type");
    }

    callback->setAccountId(m_id);

    guint id = g_idle_add(sendMessageTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add fails");
        delete callback;
        throw Common::UnknownException("Could not add task");
    }
}

static gboolean loadMessageBodyTask(void* data)
{
    LOGD("Entered");

    try {
        EmailManager::getInstance().loadMessageBody(
                static_cast<MessageBodyCallbackData*>(data));

    } catch(const Common::BasePlatformException& exception) {
        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
             (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }

    return FALSE;
}

void MessageServiceEmail::loadMessageBody(MessageBodyCallbackData *callback)
{
    LOGD("Entered");
    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    guint id = g_idle_add(loadMessageBodyTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        throw Common::UnknownException("Could not add task");
    }
}

static gboolean loadMessageAttachmentTask(void* data)
{
    LOGD("Entered");

    try {
        MessageAttachmentCallbackData *callback =
                static_cast<MessageAttachmentCallbackData *>(data);
        if (!callback) {
            LOGE("Callback is null");
            throw Common::UnknownException("Callback is null");
        }

        std::shared_ptr<MessageAttachment> att =  callback->getMessageAttachment();

        // if the attachment is already saved, then it doesn't need to load again.
        if (att->isFilePathSet() && att->isSaved()){
            JSContextRef context = callback->getContext();
            JSObjectRef jsMessageAtt = JSMessageAttachment::makeJSObject(context, att);
            callback->callSuccessCallback(jsMessageAtt);

            delete callback;
            callback = NULL;
            return FALSE;
        }

        EmailManager::getInstance().loadMessageAttachment(
                static_cast<MessageAttachmentCallbackData*>(data));
    } catch(const Common::BasePlatformException& exception) {
        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
                (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }
    return FALSE;
}

void MessageServiceEmail::loadMessageAttachment(MessageAttachmentCallbackData *callback)
{
    LOGD("Entered");
    guint id = g_idle_add(loadMessageAttachmentTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        throw Common::UnknownException("Could not add task");
    }
}

static gboolean syncTask(void* data)
{
    LOGD("Entered");

    try {
        EmailManager::getInstance().sync(data);

    } catch(const Common::BasePlatformException& exception) {
        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
             (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }

    return FALSE;
}

long MessageServiceEmail::sync(SyncCallbackData *callback)
{
    LOGD("Entered");
    if (!callback) {
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    long op_id = EmailManager::getInstance().getUniqueOpId();
    callback->setOpId(op_id);
    callback->setAccountId(m_id);

    guint id = g_idle_add(syncTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        throw Common::UnknownException("Could not add task");
    }
    return op_id;
}

static gboolean syncFolderTask(void* data)
{
    LOGD("Entered");

    try {
        EmailManager::getInstance().syncFolder(
                static_cast<SyncFolderCallbackData*>(data));

    } catch(const Common::BasePlatformException& exception) {
        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
             (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }

    return FALSE;
}

long MessageServiceEmail::syncFolder(SyncFolderCallbackData *callback)
{
    LOGD("Entered");
    if(!callback){
        LOGE("Callback is null");
        throw Common::UnknownException("Callback is null");
    }

    if(!callback->getMessageFolder()) {
        LOGE("Message folder is null");
        throw Common::TypeMismatchException("Message folder is null");
    }

    long op_id = EmailManager::getInstance().getUniqueOpId();
    callback->setOpId(op_id);
    callback->setAccountId(m_id);

    guint id = g_idle_add(syncFolderTask, callback);
    if (!id) {
        LOGE("g_idle_add fails");
        delete callback;
    }

    return op_id;
}

static gboolean stopSyncTask(void* data)
{
    LOGD("Entered");

    try {
        if (!data) {
            LOGE("opId is null");
            return FALSE;
        }

        const long op_id = *(static_cast<long*>(data));
        delete static_cast<long*>(data);
        data = NULL;
        EmailManager::getInstance().stopSync(op_id);

    } catch(const Common::BasePlatformException& exception) {
        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
             (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }

    return FALSE;
}

void MessageServiceEmail::stopSync(long data)
{
    LOGD("Entered");
    long* op_id = new long();
    *op_id = data;
    guint id = g_idle_add(stopSyncTask, static_cast<void*>(op_id));
    if (!id) {
        LOGE("g_idle_add failed");
        delete op_id;
        op_id = NULL;
        throw Common::UnknownException("Could not add task");
    }
}


} // Messaging
} // DeviceAPI

