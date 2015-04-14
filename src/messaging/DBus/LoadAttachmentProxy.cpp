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

/**
 * @file        LoadAttachmentProxy.cpp
 */

#include "LoadAttachmentProxy.h"

#include "common/logger.h"
#include "common/platform_result.h"

#include <cstring>
#include <email-types.h>
#include "../message_service.h"
#include "../message.h"
#include "../message_body.h"
#include "../email_manager.h"

#include <email-api.h>

namespace extension {
namespace messaging {
namespace DBus {

using namespace common;

/**
  * This method perform very specified task (see warning comment) so it should not be
  * visible outside LoadAttachmentProxy class.
  */
PlatformResult updateAttachmentDataWithEmailGetAttachmentData(
        std::shared_ptr<MessageAttachment> attachment)
{
    struct ScopedEmailAttachmentData {
        ScopedEmailAttachmentData() : data(NULL) { }
        ~ScopedEmailAttachmentData() {
            if(data) {
                email_free_attachment_data(&data, 1);
            }
        }
        email_attachment_data_t* operator->() { return data; }
        email_attachment_data_t* data;
    } attachment_data_holder;

    LoggerD("attachmentId = %d", attachment->getId());

    /*
     * WARNING: email_get_attachment_data seems to be getting NOT COMPLETE
     *          email_attachment_data_t object, observed that:
     *              mail_id is 0
     *              account_id is 0
     *              mailbox_id is 0
     *          Thus currently only attachment_path and attachment_mime_type is used!
     *
     *          To get COMPLETE data please use: Message::convertEmailToMessageAttachment
     *          mtehod which fetches all attachments from specified email.
     */
    int err = email_get_attachment_data(attachment->getId(), &attachment_data_holder.data);
    if (EMAIL_ERROR_NONE != err ||
        NULL == attachment_data_holder.data) {
        LoggerE("Couldn't get attachment data for attachmentId:%d", attachment->getId());
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Couldn't get attachment.");
    }

    LoggerD("attachment name : %s", attachment_data_holder->attachment_name);

    if(attachment_data_holder->attachment_mime_type) {
        attachment->setMimeType(attachment_data_holder->attachment_mime_type);
    }

    bool isSaved = false;
    if (attachment_data_holder->attachment_path) {
        LoggerD("set attachment path: %s", attachment_data_holder->attachment_path);
        attachment->setFilePath(attachment_data_holder->attachment_path);

        LoggerD("save_status: %d", attachment_data_holder->save_status);
        LoggerD("attachment_size : %d", attachment_data_holder->attachment_size);
    }
    isSaved = attachment_data_holder->save_status;
    attachment->setIsSaved(isSaved);
    return PlatformResult(ErrorCode::NO_ERROR);
}

LoadAttachmentProxy::LoadAttachmentProxy(const std::string& path,
        const std::string& iface) :
        EmailSignalProxy(path, iface)
{
}

LoadAttachmentProxy::~LoadAttachmentProxy()
{
}

PlatformResult LoadAttachmentProxy::create(const std::string& path,
                                           const std::string& iface,
                                           LoadAttachmentProxyPtr* load_attachment_proxy) {
    load_attachment_proxy->reset(new LoadAttachmentProxy(path, iface));
    if ((*load_attachment_proxy)->isNotProxyGot()) {
        LoggerE("Could not get load attachment proxy");
        load_attachment_proxy->reset();
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not get load attachment proxy");
    } else {
        return PlatformResult(ErrorCode::NO_ERROR);
    }
}

void LoadAttachmentProxy::addCallback(MessageAttachmentCallbackData* callbackOwned)
{
    if(callbackOwned->getMessageAttachment()) {
        LoggerD("Registered callback for attachment_id: %d mail_id:%d op_handle:%d nth:%d",
            callbackOwned->getMessageAttachment()->getId(),
            callbackOwned->getMessageAttachment()->getMessageId(),
            callbackOwned->getOperationHandle(),
            callbackOwned->getNth());
    }

    m_callback_set.insert(callbackOwned);
}

void LoadAttachmentProxy::removeCallback(MessageAttachmentCallbackData* callback)
{
    if(callback->getMessageAttachment()) {
        LoggerD("Removed callback for attachment_id: %d mail_id:%d op_handle:%d nth:%d",
            callback->getMessageAttachment()->getId(),
            callback->getMessageAttachment()->getMessageId(),
            callback->getOperationHandle(),
            callback->getNth());
    }

    m_callback_set.erase(callback);
}

MessageAttachmentCallbackData* LoadAttachmentProxy::findCallback(const int nth,
        const int mail_id)
{
    CallbackSet::iterator it = m_callback_set.begin();
    for (; it != m_callback_set.end(); ++it) {
        MessageAttachmentCallbackData* callback = *it;
        if (nth == callback->getNth() &&
            mail_id == callback->getMessageAttachment()->getMessageId()) {
            return callback;
        }
    }

    LoggerW("Could not find callback with nth: %d and mail_id: %d", nth, mail_id);
    return NULL;
}

void LoadAttachmentProxy::handleEmailSignal(const int status,
            const int mail_id,
            const std::string& source,
            const int op_handle,
            const int error_code)
{
    if(NOTI_DOWNLOAD_ATTACH_FINISH != status &&
            NOTI_DOWNLOAD_ATTACH_FAIL != status) {
        return;
    }

    LoggerD("received email signal with:\n  status: %d\n  mail_id: %d\n  "
            "source: %s\n  op_handle(nth): %d\n  error_code: %d",
            status, mail_id, source.c_str(), op_handle, error_code);

    MessageAttachmentCallbackData* callback = NULL;

    //It seems that D-Bus signal op_handle is equal to nth in:
    // int email_download_attachment(int mail_id, int nth, int *handle)
    // and not handle returned from above call!!
    const int nth = op_handle;

    // From old implementation it looks that op_handle(nth) is is equal to
    // index (1 based) of attachment inside email thus it is not globally unique!
    // Therfore we need to test if mail_id match.
    // For details see old implementation MailSync.cp line 461

    callback = findCallback(nth, mail_id);
    if(!callback) {
        //We should not log not found pair since it could be requested by
        //different application.
        return;
    }

    LoggerD("Found callback for pair mailId:%d nth:%d", mail_id, nth);

    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    if(NOTI_DOWNLOAD_ATTACH_FINISH == status) {
        LoggerD("Message attachment downloaded!");

        std::shared_ptr<MessageAttachment> att = callback->getMessageAttachment();
        ret = updateAttachmentDataWithEmailGetAttachmentData(att);
        if (!ret.IsError()) {
            LoggerD("Updated Message attachment object");

            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            picojson::object args;
            args[JSON_DATA_MESSAGE_ATTACHMENT] = MessagingUtil::messageAttachmentToJson(
                    callback->getMessageAttachment());
            obj[JSON_DATA] = picojson::value(args);

            callback->getQueue().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } else if(NOTI_DOWNLOAD_ATTACH_FAIL) {
        LoggerD("Load message attachment failed!");
        ret = PlatformResult(ErrorCode::UNKNOWN_ERR, "Load message attachment failed!");
    }
    if (ret.IsError()) {
        LoggerE("Exception in signal callback");
        callback->setError(ret);
        callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
    }

    if(callback) {
        removeCallback(callback);
        delete callback;
    }
}

} //namespace DBus
} //namespace messaging
} //namespace extension
