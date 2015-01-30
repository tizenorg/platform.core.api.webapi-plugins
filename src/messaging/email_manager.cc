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
 * @file: EmailManager.cpp
 */

//#include <JSWebAPIErrorFactory.h>
//#include <JSWebAPIError.h>
//#include <JSUtil.h>
#include "common/logger.h"
#include <memory>
#include "common/platform_exception.h"
#include <sstream>
//#include <GlobalContextManager.h>

#include "MsgCommon/AbstractFilter.h"

#include <email-api-network.h>
#include <email-api-account.h>
#include <email-api-mail.h>
#include <email-api-mailbox.h>

#include "email_manager.h"
#include "messaging_util.h"
#include "message_service.h"
#include "message.h"
#include "message_conversation.h"
//#include "MessageCallbackUserData.h"
//#include "MessagesCallbackUserData.h"
#include "find_msg_callback_user_data.h"
#include "conversation_callback_data.h"
#include "message_email.h"
#include "messaging_database_manager.h"

//#include "JSMessage.h"
//#include "JSMessageConversation.h"
//#include "JSMessageFolder.h"

#include <email-api.h>
#include <vconf.h>

//#include "DBus/SyncProxy.h"
#include "DBus/LoadBodyProxy.h"
//#include "DBus/LoadAttachmentProxy.h"

#include <sstream>
#include "MsgCommon/FilterIterator.h"

using namespace common;
using namespace extension::tizen;

namespace extension {
namespace messaging {

namespace {
const int ACCOUNT_ID_NOT_INITIALIZED = -1;
const std::string FIND_FOLDERS_ATTRIBUTE_ACCOUNTID_NAME  = "serviceId";
} //anonymous namespace

EmailManager& EmailManager::getInstance()
{
    LoggerD("Entered");

    static EmailManager instance;
    return instance;
}

EmailManager::EmailManager()
{
    LoggerD("Entered");
    getUniqueOpId();
    const int non_err = EMAIL_ERROR_NONE;

    if(non_err != email_service_begin()){
        LoggerE("Email service failed to begin");
        throw UnknownException("Email service failed to begin");
    }
    if(non_err != email_open_db()){
        LoggerE("Email DB failed to open");
        throw UnknownException("Email DB failed to open");
    }

    int slot_size = -1;
    vconf_get_int("db/private/email-service/slot_size", &(slot_size));
    if (slot_size > 0) {
        m_slot_size = slot_size;
    }

    m_proxy_sync = std::make_shared<DBus::SyncProxy>(
                                      DBus::Proxy::DBUS_PATH_NETWORK_STATUS,
                                      DBus::Proxy::DBUS_IFACE_NETWORK_STATUS);
    if (!m_proxy_sync) {
        LoggerE("Sync proxy is null");
        throw UnknownException("Sync proxy is null");
    }
    m_proxy_sync->signalSubscribe();

    m_proxy_load_body = std::make_shared<DBus::LoadBodyProxy>(
                                        DBus::Proxy::DBUS_PATH_NETWORK_STATUS,
                                        DBus::Proxy::DBUS_IFACE_NETWORK_STATUS);
    if (!m_proxy_load_body) {
        LoggerE("Load body proxy is null");
        throw UnknownException("Load body proxy is null");
    }
    m_proxy_load_body->signalSubscribe();

//    m_proxy_load_attachment = std::make_shared<DBus::LoadAttachmentProxy>(
//                                        DBus::Proxy::DBUS_PATH_NETWORK_STATUS,
//                                        DBus::Proxy::DBUS_IFACE_NETWORK_STATUS);
//    if (!m_proxy_load_attachment) {
//        LoggerE("Load attachment proxy is null");
//        throw Common::UnknownException("Load attachment proxy is null");
//    }
//    m_proxy_load_attachment->signalSubscribe();

    m_proxy_messageStorage = std::make_shared<DBus::MessageProxy>();
    if (!m_proxy_messageStorage) {
        LoggerE("Message proxy is null");
        throw UnknownException("Message proxy is null");
    }
    m_proxy_messageStorage->signalSubscribe();

    m_proxy_send = std::make_shared<DBus::SendProxy>();
    if (!m_proxy_send) {
        LoggerE("Send proxy is null");
        throw UnknownException("Send proxy is null");
    }
    m_proxy_send->signalSubscribe();
}

EmailManager::~EmailManager()
{
    LoggerD("Entered");
}

void EmailManager::addDraftMessagePlatform(int account_id,
    std::shared_ptr<Message> message)
{
    LoggerD("Entered");
    addMessagePlatform(account_id, message, EMAIL_MAILBOX_TYPE_DRAFT);
}

void EmailManager::addOutboxMessagePlatform(int account_id,
    std::shared_ptr<Message> message)
{
    addMessagePlatform(account_id, message, EMAIL_MAILBOX_TYPE_OUTBOX);
}

void EmailManager::addMessagePlatform(int account_id,
    std::shared_ptr<Message> message, email_mailbox_type_e mailbox_type)
{
    LoggerD("Entered");
    email_mail_data_t* mail_data = NULL;
    email_mail_data_t* mail_data_final = NULL;
    int err = EMAIL_ERROR_NONE;

    mail_data = Message::convertPlatformEmail(message);

    mail_data->account_id = account_id;

    //Adding "from" email address
    email_account_t* account = NULL;
    err = email_get_account(account_id, EMAIL_ACC_GET_OPT_FULL_DATA, &account);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("email_get_account failed. [%d]\n",err);
        err = email_free_mail_data(&mail_data,1);
        if(EMAIL_ERROR_NONE != err) {
            LoggerE("Failed to free mail data memory");
        }
        throw UnknownException("Cannot retrieve email account information");
    }
    LoggerE("FROM %s", account->user_email_address);
    std::stringstream ss;
    ss << "<" << account->user_email_address << ">";
    std::string address_from;
    ss >> address_from;
    mail_data->full_address_from = strdup(address_from.c_str());
    LoggerE("FROM %s", mail_data->full_address_from);
    err = email_free_account(&account,1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to free account data memory");
    }
    //Setting mailbox id
    email_mailbox_t *mailbox_data = NULL;
    err = email_get_mailbox_by_mailbox_type(account_id, mailbox_type,
            &mailbox_data);
    if(EMAIL_ERROR_NONE != err) {
        LoggerD("email_get_mailbox_by_mailbox_type failed. [%d]\n",err);
        err = email_free_mail_data(&mail_data,1);
        if(EMAIL_ERROR_NONE != err) {
            LoggerE("Failed to free mail data memory");
        }
        throw UnknownException("Cannot retrieve draft mailbox");
    }
    else {
        LoggerD("email_get_mailbox_by_mailbox_type success.\n");
        mail_data->mailbox_id = mailbox_data->mailbox_id;
        mail_data->mailbox_type = mailbox_data->mailbox_type;
    }

    mail_data->report_status = EMAIL_MAIL_REPORT_NONE;
    mail_data->save_status = EMAIL_MAIL_STATUS_SAVED;
    mail_data->flags_draft_field = 1;

    //adding email without attachments
    err = email_add_mail(mail_data, NULL, 0, NULL, 0);
    if(EMAIL_ERROR_NONE != err) {
        LoggerD("email_add_mail failed. [%d]\n",err);
        err = email_free_mail_data(&mail_data,1);
        if(EMAIL_ERROR_NONE != err) {
            LoggerE("Failed to free mail data memory");
        }
        err = email_free_mailbox(&mailbox_data, 1);
        if (EMAIL_ERROR_NONE != err) {
            LoggerE("Failed to destroy mailbox");
        }
        throw UnknownException("Couldn't add message to draft mailbox");
    }
    else {
        LoggerD("email_add_mail success.\n");
    }

    LoggerD("saved mail without attachments id = [%d]\n", mail_data->mail_id);

    message->setId(mail_data->mail_id);
    message->setMessageStatus(MessageStatus::STATUS_DRAFT);

    if (message->getHasAttachment()){
        Message::addEmailAttachments(message);
    }

    err = email_get_mail_data(message->getId(), &mail_data_final);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to retrieve added mail data");
        throw UnknownException("Couldn't retrieve added mail data");
    }

    message->updateEmailMessage(*mail_data_final);

    err = email_free_mail_data(&mail_data_final,1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to free mail data final memory");
    }

    err = email_free_mail_data(&mail_data,1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to free mail data memory");
    }

    err = email_free_mailbox(&mailbox_data, 1);
    if (EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to destroy mailbox");
    }
}

static gboolean addDraftMessageCompleteCB(void *data)
{
    MessageCallbackUserData* callback =
        static_cast<MessageCallbackUserData *>(data);
    if (!callback) {
        LoggerE("Callback is null");
        return false;
    }

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
            callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
        } else {
            LoggerD("Calling success callback");
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            picojson::object args;
            args[JSON_DATA_MESSAGE] = MessagingUtil::messageToJson(callback->getMessage());
            obj[JSON_DATA] = picojson::value(args);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
    } catch (...) {
        LoggerE("Message add draft failed");
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
    }

    delete callback;
    callback = NULL;

    return false;
}

void EmailManager::addDraftMessage(MessageCallbackUserData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::shared_ptr<Message> message = callback->getMessage();
        addDraftMessagePlatform(callback->getAccountId(), message);
    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Message add draft failed");
        UnknownException err("Message add draft failed");
        callback->setError(err.name(), err.message());
    }

    //Complete task
    if (!g_idle_add(addDraftMessageCompleteCB, static_cast<void *>(callback))) {
        LoggerE("g_idle addition failed");
        delete callback;
        callback = NULL;
    }
}

//**** sending email ****
static gboolean sendEmailCompleteCB(void* data)
{
    LoggerD("Entered");

    MessageRecipientsCallbackData* callback =
            static_cast<MessageRecipientsCallbackData*>(data);
    if (!callback) {
        LoggerE("Callback is null");
        return false;
    }

    try {
        if (callback->isError()) {
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
            callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
        }
        else {
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            std::vector<picojson::value> recipients;
            auto addToRecipients = [&recipients](std::string& e)->void {
                recipients.push_back(picojson::value(e));
            };

            auto toVect = callback->getMessage()->getTO();
            std::for_each(toVect.begin(), toVect.end(), addToRecipients);

            auto ccVect = callback->getMessage()->getCC();
            std::for_each(ccVect.begin(), ccVect.end(), addToRecipients);

            auto bccVect = callback->getMessage()->getBCC();
            std::for_each(bccVect.begin(), bccVect.end(), addToRecipients);

            picojson::object data;
            data[JSON_DATA_RECIPIENTS] = picojson::value(recipients);
            data[JSON_DATA_MESSAGE] = MessagingUtil::messageToJson(callback->getMessage());
            obj[JSON_DATA] = picojson::value(data);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
            callback->getMessage()->setMessageStatus(MessageStatus::STATUS_SENT);
        }
    }
    catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
    }
    catch (...) {
        LoggerE("Unknown error when calling send message callback");
    }

    delete callback;
    callback = NULL;

    return false;
}

void EmailManager::sendMessage(MessageRecipientsCallbackData* callback)
{
    LoggerD("Entered");
    int err = EMAIL_ERROR_NONE;
    email_mail_data_t *mail_data = NULL;

    try{
        if(!callback){
            LoggerE("Callback is null");
            throw common::UnknownException("Callback is null");
        }

        std::shared_ptr<Message> message = callback->getMessage();
        if(!message) {
            LoggerE("Message is null");
            throw common::UnknownException("Message is null");
        }

        if(!(message->is_id_set())) {
            addOutboxMessagePlatform(callback->getAccountId(),message);
        }

        err = email_get_mail_data(message->getId(),&mail_data);
        if(EMAIL_ERROR_NONE != err) {
            LoggerE("email_get_mail_data failed. [%d]\n",err);
            throw common::UnknownException("Failed to get platform email structure");
        }

        LoggerD("email_get_mail_data success.\n");

        //Sending EMAIL
        mail_data->save_status = EMAIL_MAIL_STATUS_SENDING;

        int req_id = 0;
        err = email_send_mail(mail_data->mail_id, &req_id);
        if (EMAIL_ERROR_NONE != err) {
            LoggerE("Failed to send message %d", err);
            throw common::UnknownException("Failed to send message");
        }
        LoggerD("req_id: %d", req_id);
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_SENDING);
        m_sendRequests[req_id] = callback;

    } catch (const common::PlatformException& ex) {
         LoggerE("%s (%s)", (ex.name()).c_str(), (ex.message()).c_str());
         callback->setError(ex.name(), ex.message());
         if (!g_idle_add(sendEmailCompleteCB, static_cast<void*>(callback))) {
             LoggerE("g_idle addition failed");
             delete callback;
             callback = NULL;
         }
    }catch (...) {
        LoggerE("Message send failed");
        common::UnknownException ex("Message send failed");
        callback->setError(ex.name(), ex.message());
        if (!g_idle_add(sendEmailCompleteCB, static_cast<void*>(callback))) {
            LoggerE("g_idle addition failed");
            delete callback;
            callback = NULL;
        }
    }

    err = email_free_mail_data(&mail_data,1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to free mail data memory");
    }

    return;
}

void EmailManager::sendStatusCallback(int mail_id,
        email_noti_on_network_event status,
        int error_code)
{
    LoggerD("Enter");

    std::lock_guard<std::mutex> lock(m_mutex);
    //find first request for this mail_id
    SendReqMapIterator it = getSendRequest(mail_id);
    if (it != m_sendRequests.end()) {
        LoggerD("Found request");
        MessageRecipientsCallbackData* callback = it->second;
        m_sendRequests.erase(it);

        if (NOTI_SEND_FAIL == status) {
            LoggerD("Failed to send message, set proper error");
            switch (error_code) {
                case EMAIL_ERROR_NO_SIM_INSERTED:
                case EMAIL_ERROR_SOCKET_FAILURE:
                case EMAIL_ERROR_CONNECTION_FAILURE:
                case EMAIL_ERROR_CONNECTION_BROKEN:
                case EMAIL_ERROR_NO_SUCH_HOST:
                case EMAIL_ERROR_NETWORK_NOT_AVAILABLE:
                case EMAIL_ERROR_INVALID_STREAM:
                case EMAIL_ERROR_NO_RESPONSE:
                {
                    LoggerE("Network error %d", error_code);
                    common::NetworkException e("Failed to send message");
                    callback->setError(e.name(), e.message());
                    break;
                }
                default:
                    LoggerE("Unknown error %d", error_code);
                    common::UnknownException ex("Failed to send message");
                    callback->setError(ex.name(),ex.message());
            }
        } else if (NOTI_SEND_FINISH == status) {
            LoggerD("Message sent successfully");
        }

        if (!g_idle_add(sendEmailCompleteCB, static_cast<void*>(callback))) {
            LoggerE("g_idle addition failed");
            delete callback;
            callback = NULL;
        }
    } else {
        LoggerW("No matching request found");
    }
}

email_mail_data_t* EmailManager::loadMessage(int msg_id)
{
    email_mail_data_t* mail_data = NULL;
    int err = EMAIL_ERROR_NONE;
    err = email_get_mail_data(msg_id, &mail_data);
    if (EMAIL_ERROR_NONE != err) {
        LoggerE("email_get_mail_data failed. [%d]", err);
    } else {
        LoggerD("email_get_mail_data success.");
    }
    return mail_data;
}

EmailManager::SendReqMapIterator EmailManager::getSendRequest(int mail_id)
{
    for (auto it = m_sendRequests.begin(); it != m_sendRequests.end(); it++) {
        if (it->second->getMessage()->getId() == mail_id) {
            return it;
        }
    }
    return m_sendRequests.end();
}

void EmailManager::freeMessage(email_mail_data_t* mail_data)
{
    if(!mail_data) {
        return;
    }

    int err = email_free_mail_data(&mail_data, 1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Could not free mail data!");
    }
}

void EmailManager::loadMessageBody(MessageBodyCallbackData* callback)
{
    LoggerD("Entered");
    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    if(!callback->getMessage()) {
        LoggerE("Callback's message is null");
        return;
    }

    m_proxy_load_body->addCallback(callback);

    const int mailId = callback->getMessage()->getId();
    int err = EMAIL_ERROR_NONE;

    int op_handle = -1;
    err = email_download_body(mailId, 0, &op_handle);
    if(EMAIL_ERROR_NONE != err){
        LoggerE("Email download body failed, %d", err);
        m_proxy_load_body->removeCallback(callback);
        return;
    }
    callback->setOperationHandle(op_handle);
}

void EmailManager::loadMessageAttachment(MessageAttachmentCallbackData* callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("Callback is null");
        throw common::InvalidValuesException("Callback is null");
    }
    if(!callback->getMessageAttachment()) {
        LoggerE("Callback's message attachment is null");
        throw common::InvalidValuesException("Callback's message attachment is null");
    }

    std::shared_ptr<MessageAttachment> msgAttachment = callback->getMessageAttachment();
    LoggerD("attachmentId:%d mailId:%d", msgAttachment->getId(),
            msgAttachment->getMessageId());

    struct ScopedEmailMailData {
        ScopedEmailMailData() : data(NULL) { }
        ~ScopedEmailMailData() { EmailManager::freeMessage(data); }
        email_mail_data_t* data;
    } mail_data_holder;

    mail_data_holder.data = EmailManager::loadMessage(msgAttachment->getMessageId());
    if(!mail_data_holder.data) {
        std::stringstream err_ss;
        err_ss << "Couldn't get email_mail_data_t for messageId:"
                << msgAttachment->getMessageId();
        LoggerE("%s",err_ss.str().c_str());
        throw common::UnknownException(err_ss.str().c_str());
    }

    AttachmentPtrVector attachments = Message::convertEmailToMessageAttachment(
            *mail_data_holder.data);
    LoggerD("Mail:%d contain:%d attachments", msgAttachment->getMessageId(),
        attachments.size());

    AttachmentPtrVector::iterator it = attachments.begin();
    int attachmentIndex = -1;
    for(int i = 0; it != attachments.end(); ++i, ++it) {
        if((*it)->getId() == msgAttachment->getId()) {
            attachmentIndex = i;
            break;
        }
    }

    if(attachmentIndex < 0) {
        std::stringstream err_ss;
        err_ss << "Attachment with id:" << msgAttachment->getId() << "not found";
        LoggerE("%s",err_ss.str().c_str());
        throw common::UnknownException(err_ss.str().c_str());
    }

    LoggerD("Attachment with id:%d is located at index:%d", msgAttachment->getId(),
            attachmentIndex);

    int op_handle = -1;
    const int nth = attachmentIndex + 1; //in documentation: the minimum number is "1"
    callback->setNth(nth);

    int err = email_download_attachment(msgAttachment->getMessageId(), nth, &op_handle);
    if (EMAIL_ERROR_NONE != err) {
        std::stringstream err_ss;
        err_ss << "Download email attachment failed with error: " << err;
        LoggerE("%s",err_ss.str().c_str());
        throw common::UnknownException(err_ss.str().c_str());
    } else  {
        LoggerD("email_download_attachment returned handle:%d",op_handle);
        callback->setOperationHandle(op_handle);
        m_proxy_load_attachment->addCallback(callback);
    }
}

//#################################### sync: ###################################

void EmailManager::sync(void* data)
{
    LoggerD("Entered");
    SyncCallbackData* callback = static_cast<SyncCallbackData*>(data);
    if(!callback){
        LoggerE("Callback is null");
        return;
    }
    long op_id = callback->getOpId();
    m_proxy_sync->addCallback(op_id, callback);

    int err = EMAIL_ERROR_NONE;
    int limit = callback->getLimit();
    int slot_size = -1;
    int account_id = callback->getAccountId();

    if (limit < 0) {
        slot_size = m_slot_size;
    }
    else {
        slot_size = limit;
    }

    err = email_set_mail_slot_size(0, 0, slot_size);
    if(EMAIL_ERROR_NONE != err){
        LoggerE("Email set slot size failed, %d", err);
        m_proxy_sync->removeCallback(op_id);
        return;
    }

    int op_handle = -1;
    err = email_sync_header(account_id, 0, &op_handle);
    if(EMAIL_ERROR_NONE != err){
        LoggerE("Email sync header failed, %d", err);
        m_proxy_sync->removeCallback(op_id);
    }
    callback->setOperationHandle(op_handle);
}

//#################################### ^sync ###################################

//################################## syncFolder: ###############################

void EmailManager::syncFolder(SyncFolderCallbackData* callback)
{
    LoggerD("Entered");
    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    const long op_id = callback->getOpId();
    m_proxy_sync->addCallback(op_id, callback);

    if(!callback->getMessageFolder())
    {
        LoggerE("Callback's messageFolder is null");
        m_proxy_sync->removeCallback(op_id);
        return;
    }

    int err = EMAIL_ERROR_NONE;

    email_mailbox_t* mailbox = NULL;

    const std::string folder_id_str = callback->getMessageFolder()->getId();
    int folder_id = 0;
    std::istringstream(folder_id_str) >> folder_id;

    err = email_get_mailbox_by_mailbox_id(folder_id, &mailbox);
    if (EMAIL_ERROR_NONE != err || NULL == mailbox) {
        LoggerE("Couldn't get mailbox, error code: %d", err);
        m_proxy_sync->removeCallback(op_id);
        return;
    }

    try {
        const int limit = callback->getLimit();
        int slot_size = -1;

        if (limit < 0) {
            slot_size = m_slot_size;
        }
        else {
            slot_size = limit;
        }

        err = email_set_mail_slot_size(0, 0, slot_size);
        if(EMAIL_ERROR_NONE != err){
            LoggerE("Email set slot size failed, %d", err);
            throw UnknownException("Email set slot size failed");
        }

        int op_handle = -1;
        const int account_id = callback->getAccountId();
        err = email_sync_header(account_id, mailbox->mailbox_id, &op_handle);
        if(EMAIL_ERROR_NONE != err) {
            LoggerE("Email sync header failed, %d", err);
            m_proxy_sync->removeCallback(op_id);
            throw UnknownException("Email sync header failed");
        }
        callback->setOperationHandle(op_handle);
    }
    catch (const PlatformException& e) {
        LoggerE("Exception in syncFolder");
    }

    if (NULL != mailbox)
    {
        err = email_free_mailbox(&mailbox , 1);
        if  (EMAIL_ERROR_NONE !=  err) {
            LoggerD("Failed to email_free_mailbox - err:%d ", err);
        }
        mailbox = NULL;
    }
}

//#################################### ^syncFolder #############################

//################################## stopSync: #################################

void EmailManager::stopSync(long op_id)
{
    LoggerD("Entered");
    SyncCallbackData* callback = NULL;
    try {
        callback = dynamic_cast<SyncCallbackData*>(
                m_proxy_sync->getCallback(op_id));
    }
    catch (const PlatformException& e) {
        LoggerE("Could not get callback");
    }
    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    int err = EMAIL_ERROR_NONE;
    err = email_cancel_job(callback->getAccountId(), callback->getOperationHandle(),
            EMAIL_CANCELED_BY_USER);
    if(EMAIL_ERROR_NONE != err){
        LoggerE("Email cancel job failed, %d", err);
    }

    std::shared_ptr<picojson::value> response = callback->getJson();
    picojson::object& obj = response->get<picojson::object>();
    AbortException error("Sync aborted by user");
    callback->setError(error.name(), error.message());
    PostQueue::getInstance().resolve(
            obj.at(JSON_CALLBACK_ID).get<double>(),
            response->serialize()
    );
    m_proxy_sync->removeCallback(op_id);
}

//################################## ^stopSync #################################

void removeEmailCompleteCB(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");
    if (!callback) {
        LoggerE("Callback is null");
        return;
    }

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        } else {
            LoggerD("Calling success callback");

            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);
            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("Error while calling removeEmail callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling removeEmail callback.");
    }

    delete callback;
    callback = NULL;
}

EmailManager::DeleteReqVector::iterator EmailManager::getDeleteRequest(
        const std::vector<int> &ids)
{
    for (auto idIt = ids.begin(); idIt != ids.end(); ++idIt) {
        for (auto reqIt = m_deleteRequests.begin(); reqIt != m_deleteRequests.end(); ++reqIt) {
            MessagePtrVector msgs = reqIt->callback->getMessages();
            for (auto msgIt = msgs.begin(); msgIt != msgs.end(); ++msgIt) {
                if ((*msgIt)->getId() == *idIt) {
                    return reqIt;
                }
            }
        }
    }
    return m_deleteRequests.end();
}

void EmailManager::removeStatusCallback(const std::vector<int> &ids,
            email_noti_on_storage_event status)
{
    LoggerD("Enter");
    std::lock_guard<std::mutex> lock(m_mutex);
    DeleteReqVector::iterator it = getDeleteRequest(ids);
    if (it != m_deleteRequests.end()) {
        LoggerD("Found request");
        if (NOTI_MAIL_DELETE_FINISH == status) {
            LoggerD("Successfully removed %d mails", ids.size());
            it->messagesDeleted += ids.size();
        }
        MessagesCallbackUserData* callback = it->callback;
        if (NOTI_MAIL_DELETE_FAIL == status) {
            LoggerD("Failed to remove mail");
            UnknownException e("Messages remove failed");
            callback->setError(e.name(), e.message());
        }
        //if one of mails failed, call error callback
        //if all mails are deleted, call success.
        // >= is used in case of duplicated dbus messages
        if (NOTI_MAIL_DELETE_FAIL == status ||
                static_cast<unsigned int>(it->messagesDeleted) >= it->callback->getMessages().size()) {
            LoggerD("Calling callback");
            m_deleteRequests.erase(it);
            m_mutex.unlock();
            removeEmailCompleteCB(callback);
        } else {
            LoggerD("Not all messages are removed, waiting for next callback");
        }
    } else {
        LoggerD("Request not found, ignoring");
    }
}

void EmailManager::removeMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    int error;
    email_mail_data_t *mail = NULL;

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
        MessageType type = callback->getMessageServiceType();
        for(auto it = messages.begin() ; it != messages.end(); ++it) {
            if((*it)->getType() != type) {
                LoggerE("Invalid message type");
                throw TypeMismatchException("Error while deleting email");
            }
        }
        for (auto it = messages.begin() ; it != messages.end(); ++it) {
            error = email_get_mail_data((*it)->getId(), &mail);
            if (EMAIL_ERROR_NONE != error) {
                LoggerE("Couldn't retrieve mail data");
                throw UnknownException("Error while deleting mail");
            }

            //This task (_EMAIL_API_DELETE_MAIL) is for async
            error = email_delete_mail(mail->mailbox_id, &mail->mail_id, 1, 0);
            if (EMAIL_ERROR_NONE != error) {
                email_free_mail_data(&mail, 1);
                LoggerE("Error while deleting mail");
                throw UnknownException("Error while deleting mail");
            }
            email_free_mail_data(&mail, 1);
        }
        //store delete request and wait for dbus response
        DeleteReq request;
        request.callback = callback;
        request.messagesDeleted = 0;
        m_deleteRequests.push_back(request);
    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
        removeEmailCompleteCB(callback);
    } catch (...) {
        LoggerE("Messages remove failed");
        UnknownException e("Messages remove failed");
        callback->setError(e.name(), e.message());
        removeEmailCompleteCB(callback);
    }
}

void EmailManager::updateMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    int error;
    email_mail_data_t *mail = NULL;

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
        MessageType type = callback->getMessageServiceType();
        for (auto it = messages.begin() ; it != messages.end(); ++it) {
            if ((*it)->getType() != type) {
                LoggerE("Invalid message type");
                throw TypeMismatchException("Error while updating message");
            }
        }
        for (auto it = messages.begin() ; it != messages.end(); ++it) {

            mail = Message::convertPlatformEmail((*it));

            if((*it)->getHasAttachment())
            {
                LoggerD("Message has attachments. Workaround need to be used.");
                //Update of mail on server using function email_update_mail() is not possible.
                //Attachment is updated only locally (can't be later loaded from server),
                //so use of workaround is needed:
                //1. add new mail
                //2. delete old mail

                //adding message again after changes
                addDraftMessagePlatform(mail->account_id, (*it));
                LoggerD("mail added - new id = [%d]\n", (*it)->getId());

                //storing old message id
                (*it)->setOldId(mail->mail_id);
                //deleting old mail
                LoggerD("mail deleted = [%d]\n", mail->mail_id);
                error = email_delete_mail(mail->mailbox_id,&mail->mail_id,1,1);
                if (EMAIL_ERROR_NONE != error) {
                    email_free_mail_data(&mail, 1);
                    LoggerE("Error while deleting old mail on update: %d", error);
                    throw UnknownException("Error while deleting old mail on update");
                }
            } else {
                LoggerD("There are no attachments, updating only email data.");
                error = email_update_mail(mail, NULL, 0, NULL, 0);
                if (EMAIL_ERROR_NONE != error) {
                    email_free_mail_data(&mail, 1);
                    LoggerE("Error while updating mail");
                    throw UnknownException("Error while updating mail");
                }
            }

            email_free_mail_data(&mail, 1);
        }
    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Messages update failed");
        UnknownException ex("Messages update failed");
        callback->setError(ex.name(), ex.message());
    }

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        } else {
            LoggerD("Calling success callback");
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            picojson::array array;
            auto messages = callback->getMessages();
            for (int i = 0 ; i < messages.size(); ++i) {
                array.push_back(MessagingUtil::messageToJson(messages[i]));
            }
            obj[JSON_DATA] = picojson::value(array);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("Error while calling updateEmail callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling updateEmail callback.");
    }

    delete callback;
    callback = NULL;
}

void EmailManager::findMessages(FindMsgCallbackUserData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    email_mail_data_t* mailList = NULL;
    int mailListCount = 0;
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::pair<int, email_mail_data_t*> emails =
                MessagingDatabaseManager::getInstance().findEmails(callback);
        mailListCount = emails.first;
        LoggerD("Found %d mails", mailListCount);

        mailList = emails.second;
        email_mail_data_t* nth_email = mailList;

        for (int i = 0; i < mailListCount; ++i) {
            std::shared_ptr<Message> email =
                    Message::convertPlatformEmailToObject(*nth_email);
            callback->addMessage(email);
            nth_email++;
        }
    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Message find failed");
        UnknownException err("Message find failed");
        callback->setError(err.name(), err.message());
    }

    if (mailListCount > 0 && mailList != NULL) {
        if (EMAIL_ERROR_NONE != email_free_mail_data(&mailList, mailListCount)) {
            LoggerW("Failed to free mailList");
        }
    }

    //Complete task
    LoggerD("callback: %p error: %d messages.size() = %d", callback, callback->isError(),
            callback->getMessages().size());

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        } else {
            LoggerD("Calling success callback");
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();

            std::vector<picojson::value> response;
            auto messages = callback->getMessages();
            std::for_each(messages.begin(), messages.end(), [&response](MessagePtr &message){
                response.push_back(MessagingUtil::messageToJson(message));
            });
            obj[JSON_DATA] = picojson::value(response);
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("Error while calling findMessages callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Failed to call findMessages callback.");
    }

    delete callback;
    callback = NULL;
}

void EmailManager::findConversations(ConversationCallbackData* callback)
{
    LoggerE("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    int convListCount = 0;
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<EmailConversationInfo> conversationsInfo =
                MessagingDatabaseManager::getInstance().findEmailConversations(callback);
        convListCount = conversationsInfo.size();
        LoggerD("Found %d conversations", convListCount);

        for (int i = 0; i < convListCount; ++i) {
            std::shared_ptr<MessageConversation> conversation =
                    MessageConversation::convertEmailConversationToObject(conversationsInfo.at(i).id);
            conversation->setUnreadMessages(conversationsInfo.at(i).unreadMessages);
            callback->addConversation(conversation);
        }

    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Conversation find failed");
        UnknownException ex("Conversation find failed");
        callback->setError(ex.name(), ex.message());
    }

    //Complete task
    LoggerD("callback: %p error:%d conversations.size()=%d", callback, callback->isError(),
            callback->getConversations().size());

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        } else {
            LoggerD("Calling success callback");
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();

            std::vector<picojson::value> response;
            auto messages = callback->getConversations();
            std::for_each(messages.begin(), messages.end(),
                    [&response](std::shared_ptr<MessageConversation> &conversation) {
                        response.push_back(MessagingUtil::conversationToJson(conversation));
                    }
            );
            obj[JSON_DATA] = picojson::value(response);
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("Error while calling findConversations callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Failed to call findConversations callback.");
    }

    delete callback;
    callback = NULL;
}

long EmailManager::getUniqueOpId()
{
    // mutex is created only on first call (first call added to constructor
    // to initialize mutex correctly)
    static std::mutex op_id_mutex;
    std::lock_guard<std::mutex> lock(op_id_mutex);
    static long op_id = 0;
    return op_id++;
}

void EmailManager::findFolders(FoldersCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    int ret = EMAIL_ERROR_UNKNOWN;
    int account_id = ACCOUNT_ID_NOT_INITIALIZED;
    email_mailbox_t* mailboxes = NULL;
    email_mailbox_t* nth_mailbox = NULL;
    int mailboxes_count;

    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        tizen::AbstractFilterPtr filter = callback->getFilter();
        if (!filter) {
            LoggerE("Filter not provided");
            throw UnknownException("Filter not provided");
        }

        for(FilterIterator it(filter); false == it.isEnd(); it++) {

            if(FIS_COMPOSITE_START == it.getState()) {
                CompositeFilterPtr cf = castToCompositeFilter((*it));
                if(cf && INTERSECTION != cf->getType()) {
                    LoggerE("[ERROR] >>> invalid Filter type: %d", cf->getType());
                    throw TypeMismatchException("Invalid Filter Type");
                }
            }
            else if(FIS_ATTRIBUTE_FILTER == it.getState()) {
                AttributeFilterPtr attrf = castToAttributeFilter((*it));
                if(attrf) {
                    const std::string attr_name = attrf->getAttributeName();
                    if (FIND_FOLDERS_ATTRIBUTE_ACCOUNTID_NAME == attr_name) {
                        account_id = static_cast<int>(attrf->getMatchValue()->toLong());
                    } else {
                        LoggerE("The attribute name: %s is invalid", attr_name.c_str());
                        throw InvalidValuesException("The attribute name is invalid");
                    }
                }
            }
        }

        LoggerD("Listing folders for account ID: %d", account_id);
        if (account_id > 0) {
            ret = email_get_mailbox_list(account_id,
                    -1,
                    &mailboxes,
                    &mailboxes_count);
            if (EMAIL_ERROR_NONE != ret || !mailboxes) {
                LoggerE("Cannot get folders: %d", ret);
                throw UnknownException("Platform error, cannot get folders");
            }

            if (mailboxes_count <= 0) {
                LoggerD("Empty mailboxes");
            }
            else {
                LoggerD("Founded mailboxes: %d", mailboxes_count);

                nth_mailbox = mailboxes;
                for (int i = 0; i < mailboxes_count; ++i) {
                    std::shared_ptr<MessageFolder> fd;
                    fd = std::make_shared<MessageFolder>(*nth_mailbox);
                    callback->addFolder(fd);
                    nth_mailbox++;
                }
            }
        }
    } catch (const PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Messages update failed");
        UnknownException ex("Messages update failed");
        callback->setError(ex.name(), ex.message());
    }

    if (mailboxes != NULL) {
        if (EMAIL_ERROR_NONE != email_free_mailbox(&mailboxes,
                mailboxes_count)) {
            LoggerW("Free mailboxes failed: %d", ret);
        }
    }

    //Complete task
    LoggerD("callback: %p error:%d folders.size()=%d", callback, callback->isError(),
            callback->getFolders().size());

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        } else {
            LoggerD("Calling success callback");
            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();

            std::vector<picojson::value> response;
            auto folders = callback->getFolders();
            std::for_each(folders.begin(), folders.end(),
                    [&response](std::shared_ptr<MessageFolder> &folder) {
                        response.push_back(MessagingUtil::folderToJson(folder));
                    }
            );
            obj[JSON_DATA] = picojson::value(response);
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("Error while calling findFolders callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling findFolders callback.");
    }

    delete callback;
    callback = NULL;
}

void EmailManager::removeConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    int error;
    try
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<MessageConversation>> conversations =
                callback->getConversations();
        MessageType type = callback->getMessageServiceType();

        int thread_id = 0;
        for(auto it = conversations.begin() ; it != conversations.end(); ++it) {
            if((*it)->getType() != type) {
                LoggerE("Invalid message type");
                throw TypeMismatchException("Error while deleting email conversation");
            }
        }

        for (auto it = conversations.begin() ; it != conversations.end(); ++it) {
            thread_id = (*it)->getConversationId();
            error = email_delete_thread(thread_id, false);
            if (EMAIL_ERROR_NONE != error) {
                LoggerE("Couldn't delete email conversation data");
                throw UnknownException("Error while deleting email conversation");
            }

            // for now, there is no way to recognize deleting email thread job is completed.
            // so use polling to wait the thread is removed.
            email_mail_data_t *thread_info = NULL;
            do {
                usleep(300 * 1000);
                LoggerD("Waiting to delete this email thread...");
                error = email_get_thread_information_by_thread_id(
                    thread_id, &thread_info);

                if (thread_info != NULL) {
                    free(thread_info);
                    thread_info = NULL;
                }
            } while (error != EMAIL_ERROR_MAIL_NOT_FOUND);
        }
    }
    catch (const PlatformException& err)
    {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    }
    catch (...)
    {
        LoggerE("Email conversation remove failed");
        callback->setError("UnknownError", "Email conversation remove failed");
    }

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                    callback->getJson()->serialize()
            );
        } else {
            LoggerD("Calling success callback");

            auto json = callback->getJson();
            picojson::object& obj = json->get<picojson::object>();
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const PlatformException& err) {
        LoggerE("Error while calling removeConversations callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling removeConversations callback.");
    }

    delete callback;
    callback = NULL;
}

} // Messaging
} // DeviceAPI
