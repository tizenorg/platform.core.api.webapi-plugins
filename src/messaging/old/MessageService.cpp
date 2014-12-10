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

#include "MessageService.h"
#include "MessageStorageShortMsg.h"
#include "MessageStorageEmail.h"

using namespace std;
using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

//#################### MessageRecipientsCallbackData ####################

MessageRecipientsCallbackData::MessageRecipientsCallbackData(JSContextRef globalCtx):
        CallbackUserData(globalCtx),
        m_is_error(false),
        m_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN),
        m_default_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN)
{
    LOGD("Entered");
}

MessageRecipientsCallbackData::~MessageRecipientsCallbackData()
{
    LOGD("Entered");
}

void MessageRecipientsCallbackData::setMessage(std::shared_ptr<Message> message)
{
    m_message = message;
}

std::shared_ptr<Message> MessageRecipientsCallbackData::getMessage() const
{
    return m_message;
}

void MessageRecipientsCallbackData::setMessageRecipients(
        const std::vector<std::string>& msgRecipients)
{
    m_msg_recipients = msgRecipients;
}

const std::vector<std::string>& MessageRecipientsCallbackData::getMessageRecipients() const
{
    return m_msg_recipients;
}

void MessageRecipientsCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
        if (m_message) {
            m_err_message += " for: ";
            // platform issue: we cannot get error per recipient
            // so all recipients are added to error message
            std::vector<std::string> recp_list = m_message->getTO();
            unsigned int count = recp_list.size();
            for (unsigned int i = 0; i < count; ++i) {
                m_err_message += recp_list.at(i) + ", ";
            }
            recp_list = m_message->getCC();
            count = recp_list.size();
            for (unsigned int i = 0; i < count; ++i) {
                m_err_message += recp_list.at(i) + ", ";
            }
            recp_list = m_message->getBCC();
            count = recp_list.size();
            for (unsigned int i = 0; i < count; ++i) {
                m_err_message += recp_list.at(i) + ", ";
            }

        }
    }
}

bool MessageRecipientsCallbackData::isError() const
{
    return m_is_error;
}

std::string MessageRecipientsCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string MessageRecipientsCallbackData::getErrorMessage() const
{
    return m_err_message;
}

void MessageRecipientsCallbackData::setAccountId(int account_id){
    m_account_id = account_id;
}

int MessageRecipientsCallbackData::getAccountId() const
{
    return m_account_id;
}

void MessageRecipientsCallbackData::setSimIndex(
    TelNetworkDefaultDataSubs_t sim_index)
{
    m_sim_index = sim_index;
}

TelNetworkDefaultDataSubs_t MessageRecipientsCallbackData::getSimIndex() const
{
    return m_sim_index;
}

bool MessageRecipientsCallbackData::isSetSimIndex() const
{
    return m_sim_index != TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN;
}

void MessageRecipientsCallbackData::setDefaultSimIndex(
    TelNetworkDefaultDataSubs_t sim_index)
{
    m_default_sim_index = sim_index;
}

TelNetworkDefaultDataSubs_t MessageRecipientsCallbackData::getDefaultSimIndex() const
{
    return m_default_sim_index;
}

//#################### BaseMessageServiceCallbackData ####################

BaseMessageServiceCallbackData::BaseMessageServiceCallbackData(JSContextRef globalCtx):
        CallbackUserData(globalCtx),
        m_is_error(false),
        m_op_handle(-1)
{
    LOGD("Entered");
}

BaseMessageServiceCallbackData::~BaseMessageServiceCallbackData()
{
    LOGD("Entered");
}

void BaseMessageServiceCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

bool BaseMessageServiceCallbackData::isError() const
{
    return m_is_error;
}

std::string BaseMessageServiceCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string BaseMessageServiceCallbackData::getErrorMessage() const
{
    return m_err_message;
}

void BaseMessageServiceCallbackData::setOperationHandle(const int op_handle)
{
    m_op_handle = op_handle;
}

int BaseMessageServiceCallbackData::getOperationHandle() const
{
    return m_op_handle;
}

//#################### MessageBodyCallbackData ####################

MessageBodyCallbackData::MessageBodyCallbackData(JSContextRef globalCtx):
        BaseMessageServiceCallbackData(globalCtx)
{
    LOGD("Entered");
}

MessageBodyCallbackData::~MessageBodyCallbackData()
{
    LOGD("Entered");
}

void MessageBodyCallbackData::setMessage(std::shared_ptr<Message> message)
{
    m_message = message;
}

std::shared_ptr<Message> MessageBodyCallbackData::getMessage() const
{
    return m_message;
}

//#################### MessageAttachmentCallbackData ####################

MessageAttachmentCallbackData::MessageAttachmentCallbackData(JSContextRef globalCtx):
        BaseMessageServiceCallbackData(globalCtx),
        m_nth(0)
{
    LOGD("Entered");
}


MessageAttachmentCallbackData::~MessageAttachmentCallbackData()
{
    LOGD("Entered");
}

void MessageAttachmentCallbackData::setMessageAttachment(
        std::shared_ptr<MessageAttachment> messageAttachment)
{
    m_message_attachment = messageAttachment;
}

std::shared_ptr<MessageAttachment> MessageAttachmentCallbackData::
    getMessageAttachment() const
{
    return m_message_attachment;
}

void MessageAttachmentCallbackData::setNth(const int nth)
{
    m_nth = nth;
}

int MessageAttachmentCallbackData::getNth() const
{
    return m_nth;
}

//#################### SyncCallbackData ####################

SyncCallbackData::SyncCallbackData(JSContextRef globalCtx):
        BaseMessageServiceCallbackData(globalCtx),
        m_is_limit(false),
        m_limit(0),
        m_account_id(-1)
{
    LOGD("Entered");
}

SyncCallbackData::~SyncCallbackData()
{
    LOGD("Entered");
}

void SyncCallbackData::setLimit(const unsigned long limit)
{
    m_is_limit = true;
    m_limit = limit;
}

bool SyncCallbackData::isLimit() const
{
    return m_is_limit;
}

unsigned long SyncCallbackData::getLimit() const
{
    return m_limit;
}

void SyncCallbackData::setOpId(long op_id)
{
    m_op_id = op_id;
}

long SyncCallbackData::getOpId()
{
    return m_op_id;
}

void SyncCallbackData::setAccountId(int account_id)
{
    m_account_id = account_id;
}

int SyncCallbackData::getAccountId() const
{
    return m_account_id;
}

//#################### SyncFolderCallbackData ####################

SyncFolderCallbackData::SyncFolderCallbackData(JSContextRef globalCtx):
        SyncCallbackData(globalCtx)
{
    LOGD("Entered");
}

SyncFolderCallbackData::~SyncFolderCallbackData()
{
    LOGD("Entered");
}

void SyncFolderCallbackData::setMessageFolder(
        std::shared_ptr<MessageFolder> message_folder)
{
    m_message_folder = message_folder;
}

std::shared_ptr<MessageFolder> SyncFolderCallbackData::getMessageFolder() const
{
    return m_message_folder;
}

//#################### MessageService ####################

MessageService::MessageService(int id,
                    MessageType msgType,
                    string name):
        SecurityAccessor(),
        m_id(id),
        m_msg_type(msgType),
        m_name(name)
{
    LOGD("Entered");
    switch (msgType) {
        case MessageType::SMS:
        case MessageType::MMS:
            m_storage.reset(new MessageStorageShortMsg(id, msgType));
            break;
        case MessageType::EMAIL:
            m_storage.reset(new MessageStorageEmail(id));
            break;
        default:
            LOGE("Undefined message type");
            throw InvalidValuesException("Undefined message type");
    }
}

MessageService::~MessageService()
{
    LOGD("Entered");
}

int MessageService::getMsgServiceId() const
{
    return m_id;
}

string MessageService::getMsgServiceIdStr() const
{
    return to_string(m_id);
}

MessageType MessageService::getMsgServiceType() const
{
    return m_msg_type;
}

std::string MessageService::getMsgServiceName() const
{
    return m_name;
}

std::shared_ptr<MessageStorage> MessageService::getMsgStorage() const
{
    return m_storage;
}

void MessageService::sendMessage(MessageRecipientsCallbackData *callback)
{
    // this method should be overwritten be specific services
    LOGE("Cannot send message");
    throw NotSupportedException("Cannot send message");
}

void MessageService::loadMessageBody(MessageBodyCallbackData *callback)
{
    // this method should be overwritten by specific services
    LOGE("Cannot load message body");
    throw NotSupportedException("Cannot load message body");
}

void MessageService::loadMessageAttachment(MessageAttachmentCallbackData *callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LOGE("Cannot load message attachment");
    throw NotSupportedException("Cannot load message attachment");
}

long MessageService::sync(SyncCallbackData *callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LOGE("Cannot sync with external server");
    throw NotSupportedException("Cannot sync with external server");
}

long MessageService::syncFolder(SyncFolderCallbackData *callback)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LOGE("Cannot sync folder with external server");
    throw NotSupportedException("Cannot sync folder with external server");
}

void MessageService::stopSync(long op_id)
{
    // this method should be overwritten by email service
    // for MMS and SMS this function is not supported
    LOGE("Cannot stop sync with external server");
    throw NotSupportedException("Cannot stop sync with external server");
}

} // Messaging
} // DeviceAPI

