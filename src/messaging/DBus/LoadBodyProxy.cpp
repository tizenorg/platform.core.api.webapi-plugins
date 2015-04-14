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
 * @file        LoadBodyProxy.cpp
 */

#include "LoadBodyProxy.h"

//#include <Logger.h>
//#include <PlatformException.h>
//#include <cstring>
#include <email-types.h>

//#include "MessageService.h"
//#include "Message.h"
//#include "MessageBody.h"
//#include "EmailManager.h"
//#include "JSMessage.h"
//#include <JSWebAPIErrorFactory.h>

#include "common/logger.h"
#include <cstring>
#include "common/platform_result.h"

#include "../message.h"
#include "../message_body.h"
#include "../email_manager.h"

namespace extension {
namespace messaging {
namespace DBus {

using namespace common;

LoadBodyProxy::LoadBodyProxy(const std::string& path,
        const std::string& iface) :
        EmailSignalProxy(path, iface)
{
}

LoadBodyProxy::~LoadBodyProxy()
{
}

PlatformResult LoadBodyProxy::create(const std::string& path,
                                     const std::string& iface,
                                     LoadBodyProxyPtr* load_body_proxy) {
    load_body_proxy->reset(new LoadBodyProxy(path, iface));
    if ((*load_body_proxy)->isNotProxyGot()) {
        LoggerE("Could not get load body proxy");
        load_body_proxy->reset();
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not get load body proxy");
    } else {
        return PlatformResult(ErrorCode::NO_ERROR);
    }
}

void LoadBodyProxy::addCallback(MessageBodyCallbackData* callbackOwned)
{
    m_callback_set.insert(callbackOwned);
}

void LoadBodyProxy::removeCallback(MessageBodyCallbackData* callback)
{
    m_callback_set.erase(callback);
}

MessageBodyCallbackData* LoadBodyProxy::findCallbackByOpHandle(const int op_handle)
{
    CallbackSet::iterator it = m_callback_set.begin();
    for (; it != m_callback_set.end(); ++it) {

        MessageBodyCallbackData* callback = *it;
        if (op_handle == callback->getOperationHandle()) {
            return callback;
        }
    }

    LoggerW("Could not find callback with op_handle: %d", op_handle);
    return NULL;
}

void LoadBodyProxy::handleEmailSignal(const int status,
        const int mail_id,
        const std::string& source,
        const int op_handle,
        const int error_code)
{
    switch(status) {
        //We should handle this signal since it is DOWNLOAD_BODY_*
        case NOTI_DOWNLOAD_BODY_START:
        case NOTI_DOWNLOAD_BODY_FINISH:
        case NOTI_DOWNLOAD_BODY_FAIL: {
        } break;

        // This values have not been explicitly handled in old implementation
        //  NOTI_DOWNLOAD_BODY_CANCEL
        //  NOTI_DOWNLOAD_MULTIPART_BODY
        //
        // 1. I assume that NOTI_DOWNLOAD_MULTIPART_BODY is called several times
        // before final NOTI_DOWNLOAD_BODY_FINISH is called, thus we should not
        // remove nor delete callback.
        //
        // 2. I assume that NOTI_DOWNLOAD_BODY_CANCEL is called before
        // NOTI_DOWNLOAD_BODY_FAIL so we should do the same as in point 1.
        case NOTI_DOWNLOAD_BODY_CANCEL:
        case NOTI_DOWNLOAD_MULTIPART_BODY:
        default: {
            // This signal is not related with load message body or there is nothing
            // to do so we can return now.
            return;
        } break;
    }

    LoggerD("received email signal with:\n  status: %d\n  mail_id: %d\n  "
        "source: %s\n  op_handle: %d\n  error_code: %d",
        status, mail_id, source.c_str(), op_handle, error_code);

    if(NOTI_DOWNLOAD_BODY_START == status) {
        LoggerD("Download message body started ...");
        // There is nothing more to do so we can return now.
        return;
    }

    MessageBodyCallbackData* callback = NULL;

    callback = findCallbackByOpHandle(op_handle);
    if (!callback) {
        LoggerE("Callback is null");
    } else {
        PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
        if( (NOTI_DOWNLOAD_BODY_FINISH == status) ||
            (NOTI_DOWNLOAD_BODY_FAIL == status &&
                     EMAIL_ERROR_MAIL_IS_ALREADY_DOWNLOADED == error_code)) {

            // Old implementation is not verifying whether message update failed,
            // it just calls success callback.
            if(callback->getMessage()) {
                email_mail_data_t* mail_data = EmailManager::loadMessage(
                         callback->getMessage()->getId());
                if (mail_data) {
                    ret = callback->getMessage()->updateEmailMessage(*mail_data);
                    if (!ret.IsError()) {
                        EmailManager::freeMessage(mail_data);
                        mail_data = NULL;
                    }
                }

                //TODO: this should be reviewed when attachments and
                //      loadAttachments have been completed.
                //TODO: see old implementation lines 608-635 in MailSync.cpp
                //
                // This is original Messaging implementation:
                //
                // std::vector<IAttachmentPtr> attachments = mail->getAttachments();
                // std::vector<IAttachmentPtr> inlineAttachments = mail->getInlineAttachments();
                //
                // for (unsigned int idx = 0; idx < attachments.size() ; idx++ )
                // {
                //   LoggerD("set Attachment ID = " << attachments[idx]->getAttachmentID());
                //   attachments[idx]->setMessage(event->m_message);
                //
                // }
                // for (unsigned int idx = 0; idx < inlineAttachments.size() ; idx++ )
                // {
                //   LoggerD("set inline Attachment ID = " << inlineAttachments[idx]->getAttachmentID());
                //   inlineAttachments[idx]->setMessage(event->m_message);
                // }
                //
            }

            if (!ret.IsError()) {
                LoggerD("Calling success callback");

                auto json = callback->getJson();
                picojson::object& obj = json->get<picojson::object>();
                obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

                picojson::object args;
                args[JSON_DATA_MESSAGE_BODY] = MessagingUtil::messageBodyToJson(
                        callback->getMessage()->getBody());
                obj[JSON_DATA] = picojson::value(args);

                callback->getQueue().resolve(
                        obj.at(JSON_CALLBACK_ID).get<double>(),
                        json->serialize()
                );
            }
        } else if(NOTI_DOWNLOAD_BODY_FAIL == status) {
            LoggerD("Load message body failed!");
            ret = PlatformResult(ErrorCode::UNKNOWN_ERR, "Load message body failed!");
        }

        if (ret.IsError()) {
            callback->setError(ret);
            callback->getQueue().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        }

        removeCallback(callback);
        delete callback;
    }
}

} //namespace DBus
} //namespace messaging
} //namespace extension
