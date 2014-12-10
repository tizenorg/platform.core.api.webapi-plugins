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
#include <Logger.h>
#include <PlatformException.h>
#include <cstring>
#include <email-types.h>
#include "MessageService.h"
#include "Message.h"
#include "MessageBody.h"
#include "EmailManager.h"
#include "JSMessageAttachment.h"
#include <email-api.h>
#include <JSWebAPIErrorFactory.h>

namespace DeviceAPI {
namespace Messaging {
namespace DBus {

/**
  * This method perform very specified task (see warning comment) so it should not be
  * visible outside LoadAttachmentProxy class.
  */
void updateAttachmentDataWithEmailGetAttachmentData(
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

    LOGD("attachmentId = %d", attachment->getId());

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
        LOGE("Couldn't get attachment data for attachmentId:%d", attachment->getId());
        throw Common::UnknownException("Couldn't get attachment.");
    }

    LOGD("attachment name : %s", attachment_data_holder->attachment_name);

    if(attachment_data_holder->attachment_mime_type) {
        attachment->setMimeType(attachment_data_holder->attachment_mime_type);
    }

    bool isSaved = false;
    if (attachment_data_holder->attachment_path) {
        LOGD("set attachment path: %s", attachment_data_holder->attachment_path);
        attachment->setFilePath(attachment_data_holder->attachment_path);

        LOGD("save_status: %d", attachment_data_holder->save_status);
        LOGD("attachment_size : %d", attachment_data_holder->attachment_size);
    }
    isSaved = attachment_data_holder->save_status;
    attachment->setIsSaved(isSaved);
}

LoadAttachmentProxy::LoadAttachmentProxy(const std::string& path,
        const std::string& iface) :
        EmailSignalProxy(path, iface)
{
}

LoadAttachmentProxy::~LoadAttachmentProxy()
{
}

void LoadAttachmentProxy::addCallback(MessageAttachmentCallbackData* callbackOwned)
{
    if(callbackOwned->getMessageAttachment()) {
        LOGD("Registered callback for attachment_id: %d mail_id:%d op_handle:%d nth:%d",
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
        LOGD("Removed callback for attachment_id: %d mail_id:%d op_handle:%d nth:%d",
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

    LOGW("Could not find callback with nth: %d and mail_id: %d", nth, mail_id);
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

    LOGD("received email signal with:\n  status: %d\n  mail_id: %d\n  "
            "source: %s\n  op_handle(nth): %d\n  error_code: %d",
            status, mail_id, source.c_str(), op_handle, error_code);

    MessageAttachmentCallbackData* callback = NULL;

    //It seems that D-Bus signal op_handle is equal to nth in:
    // int email_download_attachment(int mail_id, int nth, int *handle)
    // and not handle returned from above call!!
    const int nth = op_handle;

    try {
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

        LOGD("Found callback for pair mailId:%d nth:%d", mail_id, nth);

        if(NOTI_DOWNLOAD_ATTACH_FINISH == status) {
            LOGD("Message attachment downloaded!");

            std::shared_ptr<MessageAttachment> att = callback->getMessageAttachment();
            updateAttachmentDataWithEmailGetAttachmentData(att);
            LOGD("Updated Message attachment object");

            try {
                JSContextRef context = callback->getContext();
                JSObjectRef jsMessageAtt = JSMessageAttachment::makeJSObject(context, att);
                callback->callSuccessCallback(jsMessageAtt);
            } catch (...) {
                LOGW("Couldn't create JSMessageAttachment object!");
                throw Common::UnknownException(
                        "Couldn't create JSMessageAttachment object!");
            }

        } else if(NOTI_DOWNLOAD_ATTACH_FAIL) {
            LOGD("Load message attachment failed!");
            JSObjectRef errobj = Common::JSWebAPIErrorFactory::makeErrorObject(
                    callback->getContext(),
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(errobj);
        }
    } catch (const Common::BasePlatformException& e) {
        LOGE("Exception in signal callback");
        JSObjectRef errobj = Common::JSWebAPIErrorFactory::makeErrorObject(
                callback->getContext(), e);
        callback->callErrorCallback(errobj);
    } catch (...) {
        LOGE("Exception in signal callback");
        JSObjectRef errobj = Common::JSWebAPIErrorFactory::makeErrorObject(
                callback->getContext(),
                Common::JSWebAPIErrorFactory::UNKNOWN_ERROR,
                "Handling signal callback failed");
        callback->callErrorCallback(errobj);
    }

    if(callback) {
        removeCallback(callback);
        delete callback;
    }
}

} //namespace DBus
} //namespace Messaging
} //namespace DeviceAPI
