
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

void MessageServiceEmail::sendMessage()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageServiceEmail::loadMessageBody()
{
    LoggerD("Entered");
    //TODO add implementation
}

void MessageServiceEmail::loadMessageAttachment()
{
    LoggerD("Entered");
    //TODO add implementation
}

static gboolean syncTask(void* data)
{
    LOGD("Entered");

    try {
        EmailManager::getInstance().sync(data);

//    } catch(const Common::BasePlatformException& exception) {
//        LOGE("Unhandled exception: %s (%s)!", (exception.getName()).c_str(),
//             (exception.getMessage()).c_str());
    } catch(...) {
        LOGE("Unhandled exception!");
    }

    return FALSE;
}

long MessageServiceEmail::sync(SyncCallbackData *callback)
{
    LoggerD("Entered");
    if (!callback) {
        LOGE("Callback is null");
        //TODO throw Common::UnknownException("Callback is null");
    }

    long op_id = EmailManager::getInstance().getUniqueOpId();
    callback->setOpId(op_id);
    //callback->setAccountId(m_id);

    guint id = g_idle_add(syncTask, static_cast<void*>(callback));
    if (!id) {
        LOGE("g_idle_add failed");
        delete callback;
        //TODO throw Common::UnknownException("Could not add task");
    }
    return op_id;
}

long MessageServiceEmail::syncFolder()
{
    LoggerD("Entered");
    //TODO add implementation
    return 0;
}

void MessageServiceEmail::stopSync()
{
    LoggerD("Entered");
    //TODO add implementation
}

} // messaging
} // extension

