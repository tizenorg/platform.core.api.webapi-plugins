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

//#include <JSWebAPIErrorFactory.h>
//#include <JSWebAPIError.h>
//#include <JSUtil.h>
#include "common/logger.h"
#include "common/scope_exit.h"
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

#include "common/scope_exit.h"
#include "messaging/DBus/DBusTypes.h"

using namespace common;
using namespace extension::tizen;

namespace extension {
namespace messaging {

namespace {
const int ACCOUNT_ID_NOT_INITIALIZED = -1;
const std::string FIND_FOLDERS_ATTRIBUTE_ACCOUNTID_NAME  = "serviceId";
} //anonymous namespace

EmailManager::EmailManager() : m_slot_size(-1), m_is_initialized(false)
{
    LoggerD("Entered");
}

EmailManager& EmailManager::getInstance()
{
    LoggerD("Entered");
    static EmailManager instance;
    return instance;
}

#define CHECK_ERROR(ret, message) \
    if (ret.IsError()) { \
        LoggerE(message); \
        return ret; \
    }

PlatformResult EmailManager::InitializeEmailService()
{
    LoggerD("Entered");
    EmailManager& instance = EmailManager::getInstance();

    if (!instance.m_is_initialized) {
        instance.getUniqueOpId();
        
        int ntv_ret = email_service_begin();
        if(ntv_ret != EMAIL_ERROR_NONE){
            return LogAndCreateResult(
                      ErrorCode::UNKNOWN_ERR, "Email service failed to begin",
                      ("email_service_begin error: %d (%s)", ntv_ret, get_error_message(ntv_ret)));
        }

        ntv_ret = email_open_db();
        if(ntv_ret != EMAIL_ERROR_NONE){
            return LogAndCreateResult(
                      ErrorCode::UNKNOWN_ERR, "Email DB failed to open",
                      ("email_open_db error: %d (%s)", ntv_ret, get_error_message(ntv_ret)));
        }

        int slot_size = -1;
        vconf_get_int("db/private/email-service/slot_size", &(slot_size));
        if (slot_size > 0) {
            instance.m_slot_size = slot_size;
        }

        PlatformResult ret = DBus::SyncProxy::create(DBus::kDBusPathNetworkStatus,
                                                     DBus::kDBusIfaceNetworkStatus,
                                                     &instance.m_proxy_sync);
        CHECK_ERROR(ret, "create sync proxy failed");
        if (!instance.m_proxy_sync) {
            LoggerE("Sync proxy is null");
            return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Sync proxy is null");
        }
        instance.m_proxy_sync->signalSubscribe();

        ret = DBus::LoadBodyProxy::create(DBus::kDBusPathNetworkStatus,
                                          DBus::kDBusIfaceNetworkStatus,
                                          &instance.m_proxy_load_body);
        CHECK_ERROR(ret, "create load body proxy failed");
        if (!instance.m_proxy_load_body) {
            return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Load body proxy is null");
        }
        instance.m_proxy_load_body->signalSubscribe();

    //    ret = DBus::LoadAttachmentProxy::create(DBus::Proxy::DBUS_PATH_NETWORK_STATUS,
    //                                            DBus::Proxy::DBUS_IFACE_NETWORK_STATUS,
    //                                            &m_proxy_load_attachment);
    //    CHECK_ERROR(ret, "create load attachment proxy failed");
    //    if (!m_proxy_load_attachment) {
    //        LoggerE("Load attachment proxy is null");
    //        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Load attachment proxy is null");
    //    }
    //    m_proxy_load_attachment->signalSubscribe();

        ret = DBus::MessageProxy::create(&instance.m_proxy_messageStorage);
        CHECK_ERROR(ret, "create message proxy failed");
        if (!instance.m_proxy_messageStorage) {
            return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Message proxy is null");
        }
        instance.m_proxy_messageStorage->signalSubscribe();

        ret = DBus::SendProxy::create(&instance.m_proxy_send);
        CHECK_ERROR(ret, "create send proxy failed");
        if (!instance.m_proxy_send) {
            return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Send proxy is null");
        }
        instance.m_proxy_send->signalSubscribe();

        instance.m_is_initialized = true;
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

EmailManager::~EmailManager()
{
    LoggerD("Entered");
}

PlatformResult EmailManager::addDraftMessagePlatform(int account_id,
    std::shared_ptr<Message> message)
{
    LoggerD("Entered");
    return addMessagePlatform(account_id, message, EMAIL_MAILBOX_TYPE_DRAFT);
}

PlatformResult EmailManager::addOutboxMessagePlatform(int account_id,
    std::shared_ptr<Message> message)
{
  return addMessagePlatform(account_id, message, EMAIL_MAILBOX_TYPE_OUTBOX);
}

PlatformResult EmailManager::addMessagePlatform(int account_id,
    std::shared_ptr<Message> message, email_mailbox_type_e mailbox_type)
{
    LoggerD("Entered");
    email_mail_data_t* mail_data = NULL;
    email_mail_data_t* mail_data_final = NULL;
    int err = EMAIL_ERROR_NONE;

    PlatformResult ret = Message::convertPlatformEmail(message, &mail_data);
    if (ret.IsError()) return ret;

    mail_data->account_id = account_id;

    //Adding "from" email address
    email_account_t* account = NULL;
    err = email_get_account(account_id, GET_FULL_DATA_WITHOUT_PASSWORD, &account);
    if(EMAIL_ERROR_NONE != err) {
        int ntv_ret = email_free_mail_data(&mail_data,1);
        if(EMAIL_ERROR_NONE != ntv_ret) {
            LoggerE("Failed to free mail data memory %d (%s)", ntv_ret, get_error_message(ntv_ret));
        }
        return LogAndCreateResult(
                  ErrorCode::UNKNOWN_ERR, "Cannot retrieve email account information",
                  ("email_get_account error: %d (%s)", err, get_error_message(err)));
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
        int ntv_ret = email_free_mail_data(&mail_data,1);
        if(EMAIL_ERROR_NONE != ntv_ret) {
            LoggerE("Failed to free mail data memory: %d (%s)", ntv_ret, get_error_message(ntv_ret));
        }
        return LogAndCreateResult(
                  ErrorCode::UNKNOWN_ERR, "Cannot retrieve draft mailbox",
                  ("email_get_mailbox_by_mailbox_type error: %d (%s)", err, get_error_message(err)));
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
        int ntv_ret = email_free_mail_data(&mail_data,1);
        if(EMAIL_ERROR_NONE != ntv_ret) {
            LoggerE("Failed to free mail data memory: %d (%s)", ntv_ret, get_error_message(ntv_ret));
        }
        ntv_ret = email_free_mailbox(&mailbox_data, 1);
        if (EMAIL_ERROR_NONE != ntv_ret) {
            LoggerE("Failed to destroy mailbox: %d (%s)", ntv_ret, get_error_message(ntv_ret));
        }
        return LogAndCreateResult(
                  ErrorCode::UNKNOWN_ERR, "Couldn't add message to draft mailbox",
                  ("email_add_mail error: %d (%s)", err, get_error_message(err)));
    }
    else {
        LoggerD("email_add_mail success.\n");
    }

    LoggerD("saved mail without attachments id = [%d]\n", mail_data->mail_id);

    message->setId(mail_data->mail_id);
    message->setMessageStatus(MessageStatus::STATUS_DRAFT);

    if (message->getHasAttachment()){
        ret = Message::addEmailAttachments(message);
        if (ret.IsError()) {
          int ntv_ret = email_free_mail_data(&mail_data,1);
          if(EMAIL_ERROR_NONE != ntv_ret) {
              LoggerE("Failed to free mail data memory: %d (%s)", ntv_ret, get_error_message(ntv_ret));
          }
          return ret;
        }
    }

    err = email_get_mail_data(message->getId(), &mail_data_final);
    if(EMAIL_ERROR_NONE != err) {
      int ntv_ret = email_free_mail_data(&mail_data,1);
      if(EMAIL_ERROR_NONE != ntv_ret) {
          LoggerE("Failed to free mail data memory: %d (%s)", ntv_ret, get_error_message(ntv_ret));
      }
      return LogAndCreateResult(
                ErrorCode::UNKNOWN_ERR, "Couldn't retrieve added mail data",
                ("email_get_mail_data error: %d (%s)", err, get_error_message(err)));
    }
    ret = message->updateEmailMessage(*mail_data_final);
    if (ret.IsError()) {
      int ntv_ret = email_free_mail_data(&mail_data,1);
      if(EMAIL_ERROR_NONE != ntv_ret) {
          LoggerE("Failed to free mail data memory: %d (%s)", ntv_ret, get_error_message(ntv_ret));
      }
      return ret;
    }

    err = email_free_mail_data(&mail_data_final,1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to free mail data final memory: %d (%s)", err, get_error_message(err));
    }

    err = email_free_mail_data(&mail_data,1);
    if(EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to free mail data memory: %d (%s)", err, get_error_message(err));
    }

    err = email_free_mailbox(&mailbox_data, 1);
    if (EMAIL_ERROR_NONE != err) {
        LoggerE("Failed to destroy mailbox: %d (%s)", err, get_error_message(err));
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

static gboolean addDraftMessageCompleteCB(void *data)
{
    LoggerD("Entered");
    MessageCallbackUserData* callback =
        static_cast<MessageCallbackUserData *>(data);
    if (!callback) {
        LoggerE("Callback is null");
        return false;
    }

    if (callback->IsError()) {
        LoggerD("Calling error callback");
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
    } else {
        LoggerD("Calling success callback");

        picojson::object args;
        args[JSON_DATA_MESSAGE] = MessagingUtil::messageToJson(callback->getMessage());
        callback->SetSuccess(picojson::value(args));
    }

    callback->Post();

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
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::shared_ptr<Message> message = callback->getMessage();
    PlatformResult ret = addDraftMessagePlatform(callback->getAccountId(), message);
    if (ret.IsError()) {
      callback->SetError(ret);
    }
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

    if (callback->IsError()) {
        LoggerD("Calling error callback");
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
    }
    else {
        LoggerD("Calling success callback");

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

        callback->SetSuccess(picojson::value(data));
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_SENT);
    }

    callback->Post();

    delete callback;
    callback = NULL;

    return false;
}

PlatformResult EmailManager::sendMessage(MessageRecipientsCallbackData* callback)
{
  LoggerD("Entered");

  if (!callback) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
  }

  int err = EMAIL_ERROR_NONE;
  email_mail_data_t *mail_data = NULL;

  PlatformResult platform_result(ErrorCode::NO_ERROR);

  std::shared_ptr<Message> message = callback->getMessage();

  if (message) {
    if (!(message->is_id_set())) {
      platform_result = addOutboxMessagePlatform(callback->getAccountId(), message);
    }

    if (platform_result) {
      err = email_get_mail_data(message->getId(),&mail_data);
      if (EMAIL_ERROR_NONE != err) {
        platform_result = LogAndCreateResult(
                              ErrorCode::UNKNOWN_ERR, "Failed to get platform email structure",
                              ("email_get_mail_data %d (%s)", err, get_error_message(err)));
      } else {
        LoggerD("email_get_mail_data success.\n");

        // Sending EMAIL
        mail_data->save_status = EMAIL_MAIL_STATUS_SENDING;

        int req_id = 0;
        err = email_send_mail(mail_data->mail_id, &req_id);

        if (EMAIL_ERROR_NONE != err) {
          platform_result = LogAndCreateResult(
                                ErrorCode::UNKNOWN_ERR, "Failed to send message",
                                ("email_send_mail error: %d (%s)", err, get_error_message(err)));
        } else {
          LoggerD("req_id: %d", req_id);
          callback->getMessage()->setMessageStatus(MessageStatus::STATUS_SENDING);
          m_sendRequests[req_id] = callback;
        }
      }
    }
  } else {
    LoggerE("Message is null");
    platform_result = LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Message is null");
  }

  if (!platform_result) {
    LoggerE("Message send failed");

    callback->SetError(platform_result);

    if (!g_idle_add(sendEmailCompleteCB, static_cast<void*>(callback))) {
      LoggerE("g_idle addition failed");
      delete callback;
      callback = NULL;
    }
  }

  if (mail_data) {
    err = email_free_mail_data(&mail_data, 1);

    if (EMAIL_ERROR_NONE != err) {
      LoggerE("Failed to free mail data memory");
    }
  }

  return platform_result;
}

void EmailManager::sendStatusCallback(int mail_id,
        email_noti_on_network_event status,
        int error_code)
{
    LoggerD("Entered");

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
                    callback->SetError(LogAndCreateResult(
                                          ErrorCode::NETWORK_ERR, "Failed to send message",
                                          ("Network error: %d (%s)", error_code, get_error_message(error_code))));
                    break;
                }
                default:
                    callback->SetError(LogAndCreateResult(
                                          ErrorCode::UNKNOWN_ERR, "Failed to send message",
                                          ("Unknown error: %d (%s)", error_code, get_error_message(error_code))));
                    break;
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
    LoggerD("Entered");
    email_mail_data_t* mail_data = NULL;
    int err = email_get_mail_data(msg_id, &mail_data);
    if (EMAIL_ERROR_NONE != err) {
        LoggerE("email_get_mail_data failed. [%d] (%s)", err, get_error_message(err));
    } else {
        LoggerD("email_get_mail_data success.");
    }
    return mail_data;
}

EmailManager::SendReqMapIterator EmailManager::getSendRequest(int mail_id)
{
    LoggerD("Entered");
    for (auto it = m_sendRequests.begin(); it != m_sendRequests.end(); it++) {
        if (it->second->getMessage()->getId() == mail_id) {
            return it;
        }
    }
    return m_sendRequests.end();
}

void EmailManager::freeMessage(email_mail_data_t* mail_data)
{
    LoggerD("Entered");
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

    int op_handle = -1;
    int err = email_download_body(mailId, 0, &op_handle);
    if(EMAIL_ERROR_NONE != err){
        LoggerE("Email download body failed, %d (%s)", err, get_error_message(err));
        m_proxy_load_body->removeCallback(callback);
        return;
    }
    callback->setOperationHandle(op_handle);
}

PlatformResult EmailManager::loadMessageAttachment(MessageAttachmentCallbackData* callback)
{
  LoggerD("Entered");

  if (!callback) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Callback is null");
  }

  if (!callback->getMessageAttachment()) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Callback's message attachment is null");
  }

  std::shared_ptr<MessageAttachment> msgAttachment = callback->getMessageAttachment();
  LoggerD("attachmentId: [%d] mailId: [%d]", msgAttachment->getId(),
          msgAttachment->getMessageId());

  email_mail_data_t* mail_data = EmailManager::loadMessage(msgAttachment->getMessageId());

  SCOPE_EXIT {
    EmailManager::freeMessage(mail_data);
  };

  if (!mail_data) {
    return LogAndCreateResult(
              ErrorCode::UNKNOWN_ERR, "Couldn't load message.",
              ("Couldn't get email_mail_data_t for messageId: %d", msgAttachment->getMessageId()));
  }

  AttachmentPtrVector attachments;
  auto platform_result = Message::convertEmailToMessageAttachment(*mail_data, &attachments);

  if (!platform_result) {
    return platform_result;
  }

  LoggerD("Mail: [%d] contains: [%d] attachments",
          msgAttachment->getMessageId(), attachments.size());

  auto it = attachments.begin();
  int attachmentIndex = -1;
  for (int i = 0; it != attachments.end(); ++i, ++it) {
    if ((*it)->getId() == msgAttachment->getId()) {
      attachmentIndex = i;
      break;
    }
  }

  if (attachmentIndex < 0) {
    return LogAndCreateResult(
              ErrorCode::UNKNOWN_ERR, "Couldn't find attachment.",
              ("Attachment with id: %d not found", msgAttachment->getId()));
  }

  LoggerD("Attachment with id: [%d] is located at index: [%d]",
          msgAttachment->getId(), attachmentIndex);

  int op_handle = -1;
  const int nth = attachmentIndex + 1;  //in documentation: the minimum number is "1"
  callback->setNth(nth);

  int err = email_download_attachment(msgAttachment->getMessageId(), nth, &op_handle);

  if (EMAIL_ERROR_NONE != err) {
    return LogAndCreateResult(
              ErrorCode::UNKNOWN_ERR, "Failed to load attachment.",
              ("Download email attachment failed with error: %d (%s)", err, get_error_message(err)));
  } else {
    LoggerD("email_download_attachment returned handle: [%d]", op_handle);
    callback->setOperationHandle(op_handle);
    m_proxy_load_attachment->addCallback(callback);
    return PlatformResult(ErrorCode::NO_ERROR);
  }
}

//#################################### sync: ###################################

void EmailManager::sync(void* data)
{
  LoggerD("Entered");

  SyncCallbackData* callback = static_cast<SyncCallbackData*>(data);

  if (!callback) {
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
  } else {
    slot_size = limit;
  }

  err = email_set_mail_slot_size(0, 0, slot_size);

  if (EMAIL_ERROR_NONE != err) {
    LoggerE("Email set slot size failed, %d (%s)", err, get_error_message(err));
    m_proxy_sync->removeCallback(op_id);
    return;
  }

  int op_handle = -1;
  err = email_sync_header(account_id, 0, &op_handle);

  if (EMAIL_ERROR_NONE != err) {
    LoggerE("Email sync header failed, %d (%s)", err, get_error_message(err));
    m_proxy_sync->removeCallback(op_id);
  } else {
    callback->setOperationHandle(op_handle);
  }
}

//#################################### ^sync ###################################

//################################## syncFolder: ###############################

void EmailManager::syncFolder(SyncFolderCallbackData* callback)
{
  LoggerD("Entered");

  if (!callback) {
    LoggerE("Callback is null");
    return;
  }

  const long op_id = callback->getOpId();
  m_proxy_sync->addCallback(op_id, callback);

  if (!callback->getMessageFolder()) {
    LoggerE("Callback's messageFolder is null");
    m_proxy_sync->removeCallback(op_id);
    return;
  }

  email_mailbox_t* mailbox = NULL;

  const std::string folder_id_str = callback->getMessageFolder()->getId();
  int folder_id = 0;
  std::istringstream(folder_id_str) >> folder_id;

  int err = email_get_mailbox_by_mailbox_id(folder_id, &mailbox);

  if (EMAIL_ERROR_NONE != err || NULL == mailbox) {
    LoggerE("Couldn't get mailbox, error code: %d (%s)", err, get_error_message(err));
    m_proxy_sync->removeCallback(op_id);
    return;
  }

  const int limit = callback->getLimit();
  int slot_size = -1;

  if (limit < 0) {
    slot_size = m_slot_size;
  } else {
    slot_size = limit;
  }

  err = email_set_mail_slot_size(0, 0, slot_size);
  if (EMAIL_ERROR_NONE != err) {
    LoggerE("Email set slot size failed, %d (%s)", err, get_error_message(err));
  } else {
    int op_handle = -1;
    const int account_id = callback->getAccountId();

    err = email_sync_header(account_id, mailbox->mailbox_id, &op_handle);

    if (EMAIL_ERROR_NONE != err) {
      LoggerE("Email sync header failed, %d (%s)", err, get_error_message(err));
      m_proxy_sync->removeCallback(op_id);
    } else {
      callback->setOperationHandle(op_handle);
    }
  }

  if (NULL != mailbox) {
    err = email_free_mailbox(&mailbox, 1);
    if (EMAIL_ERROR_NONE != err) {
      LoggerE("Failed to email_free_mailbox - err: %d (%s)", err, get_error_message(err));
    }
    mailbox = NULL;
  }
}

//#################################### ^syncFolder #############################

//################################## stopSync: #################################

void EmailManager::stopSync(long op_id)
{
  LoggerD("Entered");

  SyncCallbackData* callback = static_cast<SyncCallbackData*>(m_proxy_sync->getCallback(op_id));

  if (!callback) {
    LoggerE("Callback is null");
    return;
  }

  int err = email_cancel_job(callback->getAccountId(),
                         callback->getOperationHandle(),
                         EMAIL_CANCELED_BY_USER);

  if (EMAIL_ERROR_NONE != err) {
    LoggerE("Email cancel job failed, %d (%s)", err, get_error_message(err));
  }

  callback->SetError(LogAndCreateResult(ErrorCode::ABORT_ERR, "Sync aborted by user"));

  callback->Post();
  m_proxy_sync->removeCallback(op_id);
}

//################################## ^stopSync #################################

void EmailManager::RemoveSyncCallback(long op_id) {
  LoggerD("Entered");
  m_proxy_sync->removeCallback(op_id);
}

void EmailManager::RemoveCallbacksByQueue(const PostQueue& q) {
  LoggerD("Entered");

  for (auto it = m_sendRequests.begin(); it != m_sendRequests.end();) {
    if (it->second->HasQueue(q)) {
      delete it->second;
      m_sendRequests.erase(it++);
    } else {
      ++it;
    }
  }
}

void removeEmailCompleteCB(MessagesCallbackUserData* callback)
{
  LoggerD("Entered");
  if (!callback) {
    LoggerE("Callback is null");
    return;
  }


  if (callback->IsError()) {
    LoggerD("Calling error callback");

  } else {
    LoggerD("Calling success callback");

    callback->SetSuccess();
  }

  callback->Post();

  delete callback;
  callback = NULL;
}

EmailManager::DeleteReqVector::iterator EmailManager::getDeleteRequest(
        const std::vector<int> &ids)
{
    LoggerD("Entered");
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
    LoggerD("Entered");
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
            callback->SetError(LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Messages remove failed"));
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

PlatformResult EmailManager::RemoveMessagesPlatform(MessagesCallbackUserData* callback)
{
  LoggerD("Entered");
  int error;
  email_mail_data_t *mail = NULL;

  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
  MessageType type = callback->getMessageServiceType();
  for(auto it = messages.begin() ; it != messages.end(); ++it) {
    if((*it)->getType() != type) {
      return LogAndCreateResult(
                ErrorCode::TYPE_MISMATCH_ERR, "Error while deleting email",
                ("Invalid message type %d", (*it)->getType()));
    }
  }
  for (auto it = messages.begin() ; it != messages.end(); ++it) {
    error = email_get_mail_data((*it)->getId(), &mail);
    if (EMAIL_ERROR_NONE != error) {
      return LogAndCreateResult(
                ErrorCode::UNKNOWN_ERR, "Error while deleting mail",
                ("Couldn't retrieve mail data: %d (%s)", error, get_error_message(error)));
    }

    //This task (_EMAIL_API_DELETE_MAIL) is for async
    error = email_delete_mail(mail->mailbox_id, &mail->mail_id, 1, 0);
    if (EMAIL_ERROR_NONE != error) {
      email_free_mail_data(&mail, 1);
      return LogAndCreateResult(
                ErrorCode::UNKNOWN_ERR, "Error while deleting mail",
                ("email_delete_mail error: %d (%s)", error, get_error_message(error)));
    }
    email_free_mail_data(&mail, 1);
  }
  //store delete request and wait for dbus response
  DeleteReq request;
  request.callback = callback;
  request.messagesDeleted = 0;
  m_deleteRequests.push_back(request);

  return PlatformResult(ErrorCode::NO_ERROR);
}

void EmailManager::removeMessages(MessagesCallbackUserData* callback)
{
  LoggerD("Entered");

  if (!callback){
    LoggerE("Callback is null");
    return;
  }

  PlatformResult ret = RemoveMessagesPlatform(callback);
  if (ret.IsError()) {
    LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
    callback->SetError(ret);
    removeEmailCompleteCB(callback);
  }
}

PlatformResult EmailManager::UpdateMessagesPlatform(MessagesCallbackUserData* callback) {
  LoggerD("Entered");
  int error;
  email_mail_data_t *mail = NULL;
  SCOPE_EXIT {
    if (mail) {
      email_free_mail_data(&mail, 1);
      mail = NULL;
    }
  };

  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
  MessageType type = callback->getMessageServiceType();
  for (auto it = messages.begin() ; it != messages.end(); ++it) {
    if ((*it)->getType() != type) {
      return LogAndCreateResult(
                ErrorCode::TYPE_MISMATCH_ERR, "Error while updating message",
                ("Invalid message type: %d", (*it)->getType()));
    }
  }
  for (auto it = messages.begin() ; it != messages.end(); ++it) {

    PlatformResult ret = Message::convertPlatformEmail((*it), &mail);
    if (ret.IsError()) return ret;

    if((*it)->getHasAttachment())
    {
      LoggerD("Message has attachments. Workaround need to be used.");
      //Update of mail on server using function email_update_mail() is not possible.
      //Attachment is updated only locally (can't be later loaded from server),
      //so use of workaround is needed:
      //1. add new mail
      //2. delete old mail

      //adding message again after changes
      PlatformResult ret = addDraftMessagePlatform(mail->account_id, (*it));
      if (ret.IsError()) {
        return ret;
      }
      LoggerD("mail added - new id = [%d]\n", (*it)->getId());

      //storing old message id
      (*it)->setOldId(mail->mail_id);
      //deleting old mail
      LoggerD("mail deleted = [%d]\n", mail->mail_id);
      error = email_delete_mail(mail->mailbox_id,&mail->mail_id,1,1);
      if (EMAIL_ERROR_NONE != error) {
        return LogAndCreateResult(
                  ErrorCode::UNKNOWN_ERR, "Error while deleting old mail on update",
                  ("email_delete_mail error: %d (%s)", error, get_error_message(error)));
      }
    } else {
      LoggerD("There are no attachments, updating only email data.");
      error = email_update_mail(mail, NULL, 0, NULL, 0);
      if (EMAIL_ERROR_NONE != error) {
        return LogAndCreateResult(
                  ErrorCode::UNKNOWN_ERR, "Error while updating mail",
                  ("email_update_mail error: %d (%s)", error, get_error_message(error)));
      }
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void EmailManager::updateMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    PlatformResult ret = UpdateMessagesPlatform(callback);
    if (ret.IsError()) {
      LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
      callback->SetError(ret);
    }

    if (callback->IsError()) {
      LoggerD("Calling error callback");
    } else {
      LoggerD("Calling success callback");

      picojson::array array;
      auto messages = callback->getMessages();
      size_t messages_size = messages.size();
      for (size_t i = 0 ; i < messages_size; ++i) {
        array.push_back(MessagingUtil::messageToJson(messages[i]));
      }

      callback->SetSuccess(picojson::value(array));
    }

    callback->Post();

    delete callback;
    callback = NULL;
}

PlatformResult EmailManager::FindMessagesPlatform(FindMsgCallbackUserData* callback)
{
  LoggerD("Entered");
  email_mail_data_t* mailList = NULL;
  int mailListCount = 0;

  SCOPE_EXIT {
    if (mailListCount > 0 && mailList != NULL) {
      if (EMAIL_ERROR_NONE != email_free_mail_data(&mailList, mailListCount)) {
        LoggerW("Failed to free mailList");
      }
    }
  };

  std::lock_guard<std::mutex> lock(m_mutex);
  std::pair<int, email_mail_data_t*> emails;
  PlatformResult ret = MessagingDatabaseManager::getInstance().findEmails(callback, &emails);
  if (ret.IsError()) return ret;
  mailListCount = emails.first;
  LoggerD("Found %d mails", mailListCount);

  mailList = emails.second;
  email_mail_data_t* nth_email = mailList;

  for (int i = 0; i < mailListCount; ++i) {
    std::shared_ptr<Message> email;
    ret = Message::convertPlatformEmailToObject(*nth_email, &email);
    if (ret.IsError()) return ret;
    callback->addMessage(email);
    nth_email++;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void EmailManager::findMessages(FindMsgCallbackUserData* callback)
{
  LoggerD("Entered");

  if(!callback){
    LoggerE("Callback is null");
    return;
  }

  PlatformResult ret = FindMessagesPlatform(callback);
  if (ret.IsError()) {
    LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
    callback->SetError(ret);
  }

  //Complete task
  LoggerD("callback: %p error: %d messages.size() = %d", callback, callback->IsError(),
          callback->getMessages().size());

  if (callback->IsError()) {
    LoggerD("Calling error callback");

  } else {
    LoggerD("Calling success callback");
    std::vector<picojson::value> response;
    auto messages = callback->getMessages();
    std::for_each(messages.begin(), messages.end(), [&response](MessagePtr &message){
      response.push_back(MessagingUtil::messageToJson(message));
    });

    callback->SetSuccess(picojson::value(response));
  }

  callback->Post();

  delete callback;
  callback = NULL;
}

PlatformResult EmailManager::FindConversationsPlatform(ConversationCallbackData* callback)
{
  LoggerD("Entered");
  int convListCount = 0;

  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<EmailConversationInfo> conversationsInfo;
  PlatformResult ret = MessagingDatabaseManager::getInstance().
      findEmailConversations(callback, &conversationsInfo);
  if (ret.IsError()) return ret;

  convListCount = conversationsInfo.size();
  LoggerD("Found %d conversations", convListCount);

  for (int i = 0; i < convListCount; ++i) {
    std::shared_ptr<MessageConversation> conversation;
    PlatformResult ret = MessageConversation::convertEmailConversationToObject(
        conversationsInfo.at(i).id, &conversation);
    conversation->setUnreadMessages(conversationsInfo.at(i).unreadMessages);
    callback->addConversation(conversation);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void EmailManager::findConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    PlatformResult ret = FindConversationsPlatform(callback);
    if (ret.IsError()) {
        LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
        callback->SetError(ret);
    }

    //Complete task
    LoggerD("callback: %p error:%d conversations.size()=%d", callback, callback->IsError(),
            callback->getConversations().size());

    if (callback->IsError()) {
      LoggerD("Calling error callback");
    } else {
      LoggerD("Calling success callback");

      std::vector<picojson::value> response;
      auto messages = callback->getConversations();
      std::for_each(messages.begin(), messages.end(),
                    [&response](std::shared_ptr<MessageConversation> &conversation) {
        response.push_back(MessagingUtil::conversationToJson(conversation));
      }
      );

      callback->SetSuccess(picojson::value(response));
    }

    callback->Post();

    delete callback;
    callback = NULL;
}

long EmailManager::getUniqueOpId()
{
    LoggerD("Entered");
    // mutex is created only on first call (first call added to constructor
    // to initialize mutex correctly)
    static std::mutex op_id_mutex;
    std::lock_guard<std::mutex> lock(op_id_mutex);
    static long op_id = 0;
    return op_id++;
}

PlatformResult EmailManager::FindFoldersPlatform(FoldersCallbackData* callback)
{
  LoggerD("Entered");
  int ret = EMAIL_ERROR_UNKNOWN;
  int account_id = ACCOUNT_ID_NOT_INITIALIZED;
  email_mailbox_t* mailboxes = NULL;
  email_mailbox_t* nth_mailbox = NULL;
  int mailboxes_count;

  SCOPE_EXIT {
    if (mailboxes != NULL) {
      if (EMAIL_ERROR_NONE != email_free_mailbox(&mailboxes,
                                                 mailboxes_count)) {
        LoggerW("Free mailboxes failed: %d", ret);
      }
    }
  };

  std::lock_guard<std::mutex> lock(m_mutex);

  tizen::AbstractFilterPtr filter = callback->getFilter();
  if (!filter) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Filter not provided");
  }

  for(FilterIterator it(filter); false == it.isEnd(); it++) {

    if (FIS_COMPOSITE_START == it.getState()) {
      CompositeFilterPtr cf = castToCompositeFilter((*it));
      if(cf && INTERSECTION != cf->getType()) {
        return LogAndCreateResult(
                  ErrorCode::TYPE_MISMATCH_ERR, "Invalid Filter Type",
                  ("Invalid Filter type: %d", cf->getType()));
      }
    }
    else if (FIS_ATTRIBUTE_FILTER == it.getState()) {
      AttributeFilterPtr attrf = castToAttributeFilter((*it));
      if (attrf) {
        const std::string attr_name = attrf->getAttributeName();
        if (FIND_FOLDERS_ATTRIBUTE_ACCOUNTID_NAME == attr_name) {
          account_id = static_cast<int>(attrf->getMatchValue()->toLong());
        } else {
          return LogAndCreateResult(
                    ErrorCode::INVALID_VALUES_ERR, "The attribute name is invalid",
                    ("The attribute name: %s is invalid", attr_name.c_str()));
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
      return LogAndCreateResult(
                ErrorCode::UNKNOWN_ERR, "Platform error, cannot get folders",
                ("email_get_mailbox_list error: %d (%s)", ret, get_error_message(ret)));
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
  return PlatformResult(ErrorCode::NO_ERROR);
}

void EmailManager::findFolders(FoldersCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    PlatformResult ret = FindFoldersPlatform(callback);
    if (ret.IsError()) {
      LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
      callback->SetError(ret);
    }

    //Complete task
    LoggerD("callback: %p error:%d folders.size()=%d", callback, callback->IsError(),
            callback->getFolders().size());

    if (callback->IsError()) {
      LoggerD("Calling error callback");
    } else {
      LoggerD("Calling success callback");

      std::vector<picojson::value> response;
      auto folders = callback->getFolders();
      std::for_each(folders.begin(), folders.end(),
                    [&response](std::shared_ptr<MessageFolder> &folder) {
        response.push_back(MessagingUtil::folderToJson(folder));
      }
      );

      callback->SetSuccess(picojson::value(response));
    }

    callback->Post();

    delete callback;
    callback = NULL;
}

PlatformResult EmailManager::RemoveConversationsPlatform(ConversationCallbackData* callback)
{
  LoggerD("Entered");
  int error;
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<std::shared_ptr<MessageConversation>> conversations =
      callback->getConversations();
  MessageType type = callback->getMessageServiceType();

  int thread_id = 0;
  for(auto it = conversations.begin() ; it != conversations.end(); ++it) {
    if ((*it)->getType() != type) {
      return LogAndCreateResult(
                ErrorCode::TYPE_MISMATCH_ERR, "Error while deleting email conversation",
                ("Invalid message type %d", (*it)->getType()));
    }
  }

  for (auto it = conversations.begin() ; it != conversations.end(); ++it) {
    thread_id = (*it)->getConversationId();
    error = email_delete_thread(thread_id, false);
    if (EMAIL_ERROR_NONE != error) {
      return LogAndCreateResult(
                ErrorCode::TYPE_MISMATCH_ERR, "Error while deleting email conversation",
                ("Couldn't delete email conversation data %d (%s)", error, get_error_message(error)));
    }

    // for now, there is no way to recognize deleting email thread job is completed.
    // so use polling to wait the thread is removed.
    email_mail_data_t *thread_info = NULL;
    do {
      struct timespec sleep_time = { 0, 300L * 1000L * 1000L };
      nanosleep(&sleep_time, nullptr);
      LoggerD("Waiting to delete this email thread...");
      error = email_get_thread_information_by_thread_id(
          thread_id, &thread_info);

      if (thread_info != NULL) {
        free(thread_info);
        thread_info = NULL;
      }
    } while (error != EMAIL_ERROR_MAIL_NOT_FOUND);
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void EmailManager::removeConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    PlatformResult ret = RemoveConversationsPlatform(callback);
    if (ret.IsError()) {
      LoggerE("%d (%s)", ret.error_code(), (ret.message()).c_str());
      callback->SetError(ret);
    }

    if (callback->IsError()) {
      LoggerD("Calling error callback");
    } else {
      LoggerD("Calling success callback");

      callback->SetSuccess();
    }

    callback->Post();

    delete callback;
    callback = NULL;
}

std::string EmailManager::getMessageStatus(int id) {
  LoggerD("Entered");

  email_mail_data_t *mail = nullptr;
  MessageStatus status = MessageStatus::STATUS_UNDEFINED;

  int ret = email_get_mail_data(id, &mail);
  if (EMAIL_ERROR_NONE != ret || !mail) {
    LoggerD("Failed to get data %d (%s)", ret, get_error_message(ret));
    return "";
  }

  switch(mail->save_status) {
    case EMAIL_MAIL_STATUS_SENT:
      status = MessageStatus::STATUS_SENT;
      break;
    case EMAIL_MAIL_STATUS_SENDING:
      status = MessageStatus::STATUS_SENDING;
      break;
    case EMAIL_MAIL_STATUS_SAVED:
      status = MessageStatus::STATUS_DRAFT;
      break;
    case EMAIL_MAIL_STATUS_SEND_FAILURE:
      status = MessageStatus::STATUS_FAILED;
      break;
    default:
      status = MessageStatus::STATUS_UNDEFINED;
      break;
  }

  ret = email_free_mail_data(&mail, 1);
  if (EMAIL_ERROR_NONE != ret ) {
    LoggerD("Failed to free mail data %d (%s)", ret, get_error_message(ret));
  }

  return MessagingUtil::messageStatusToString(status);
}

} // Messaging
} // DeviceAPI
