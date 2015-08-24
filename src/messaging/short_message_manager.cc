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
 
#include <msg-service/msg.h>
#include <msg_transport.h>
#include <msg_storage.h>
#include <unordered_set>

#include "common/platform_exception.h"
#include "common/logger.h"

#include "messaging_util.h"
#include "messaging_instance.h"
#include "message_service.h"
#include "message_sms.h"
//#include "MessageMMS.h"
//#include "JSMessageConversation.h"
#include "messaging_database_manager.h"

#include "short_message_manager.h"

using common::ErrorCode;
using common::PlatformResult;

namespace extension {
namespace messaging {

ShortMsgManager& ShortMsgManager::getInstance()
{
    LoggerD("Entered");

    static ShortMsgManager instance;
    return instance;
}

static gboolean sendMessageCompleteCB(void* data)
{
    LoggerD("Entered callback:%p", data);

    MessageRecipientsCallbackData* callback =
            static_cast<MessageRecipientsCallbackData*>(data);
    if (!callback) {
        LoggerE("Callback is null");
        return false;
    }

    if (callback->isError()) {
      callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_FAILED);
    }
    else {
        std::shared_ptr<Message> message = callback->getMessage();

        LoggerD("Calling success callback with: %d recipients", message->getTO().size());

        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        std::vector<picojson::value> recipients;
        auto addToRecipients = [&recipients](std::string& e)->void {
            recipients.push_back(picojson::value(e));
        };

        auto toVect = callback->getMessage()->getTO();
        std::for_each(toVect.begin(), toVect.end(), addToRecipients);

        picojson::object data;
        data[JSON_DATA_RECIPIENTS] = picojson::value(recipients);
        data[JSON_DATA_MESSAGE] = MessagingUtil::messageToJson(message);
        obj[JSON_DATA] = picojson::value(data);

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
        callback->getMessage()->setMessageStatus(MessageStatus::STATUS_SENT);
    }

    delete callback;
    callback = NULL;

    return false;
}

static gboolean addDraftMessageCompleteCB(void *data)
{
    LoggerD("Enter");
    auto callback = static_cast<MessageCallbackUserData *>(data);
    if (!callback) {
        LoggerE("Callback is null");
        return FALSE;
    }

    if (callback->isError()) {
        LoggerD("Calling error callback");

        callback->getQueue().resolve(
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

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
    }

    delete callback;
    callback = NULL;

    return FALSE;
}


PlatformResult ShortMsgManager::addDraftMessagePlatform(std::shared_ptr<Message> message)
{
    LoggerD("Add new message(%p)", message.get());

    // Save platform msg to get ID
    msg_struct_t platform_msg = nullptr;
    PlatformResult ret = Message::convertPlatformShortMessageToStruct(message.get(),
                                                                      m_msg_handle, &platform_msg);
    if (ret.IsError()) {
        LoggerD("Convert Platform Short Message to Struct failed (%s)", ret.message().c_str());
        return ret;
    }

    if (NULL == platform_msg) {
        LoggerE("Failed to prepare platform message");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot prepare platform message");
    }

    msg_struct_t send_opt = msg_create_struct(MSG_STRUCT_SENDOPT);
    msg_set_bool_value(send_opt, MSG_SEND_OPT_SETTING_BOOL, false);
    const int msg_id = msg_add_message(m_msg_handle, platform_msg, send_opt);
    if (msg_id < MSG_SUCCESS) {
        LoggerE("Message(%p): Failed to add draft, error: %d", message.get(), msg_id);
        msg_release_struct(&send_opt);
        msg_release_struct(&platform_msg);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot add message to draft");
    }

    LoggerD("Message(%p): New message ID: %d", message.get(), msg_id);
    msg_set_int_value(platform_msg, MSG_MESSAGE_ID_INT, msg_id);
    message->setId(msg_id);
    message->setMessageStatus(MessageStatus::STATUS_DRAFT);

    msg_struct_t msg_conv = msg_create_struct(MSG_STRUCT_CONV_INFO);
    msg_error_t err = msg_get_conversation(m_msg_handle, msg_id, msg_conv);
    if (MSG_SUCCESS == err) {
        int conversationId = 0;
        msg_get_int_value(msg_conv, MSG_CONV_MSG_THREAD_ID_INT, &conversationId);
        message->setConversationId(conversationId);
    } else {
        LoggerE("Message(%p): Failed to get conv", message.get());
    }
    Message* msgInfo = nullptr;
    ret = Message::convertPlatformShortMessageToObject(
        platform_msg, &msgInfo);
    if (ret.IsError()) {
        LoggerD("Convert Platform Short Message to Object failed (%s)", ret.message().c_str());
        return ret;
    }
    const int folderId = msgInfo->getFolderId();
    message->setFolderId(folderId);
    const time_t timestamp = msgInfo->getTimestamp();
    message->setTimeStamp(timestamp);

    const std::string from = msgInfo->getFrom();
    LoggerD("From: %s", from.c_str());
    message->setFrom(from);

    const bool isRead = msgInfo->getIsRead();
    message->setIsRead(isRead);

    const int inResponseTo = msgInfo->getInResponseTo();
    message->setInResponseTo(inResponseTo);

    if (msg_release_struct(&platform_msg) != MSG_SUCCESS) {
        LoggerW("Platform message is already destroyed");
    }
    if (msg_release_struct(&msg_conv) != MSG_SUCCESS) {
        LoggerW("Platform message is already destroyed");
    }
    if (msg_release_struct(&send_opt) != MSG_SUCCESS) {
        LoggerW("Platform message is already destroyed");
    }
    delete msgInfo;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ShortMsgManager::SendMessagePlatform(MessageRecipientsCallbackData* callback)
{
  LoggerD("Entered");
  std::lock_guard<std::mutex> lock(m_mutex);

  PlatformResult platform_result(ErrorCode::NO_ERROR);
  int msg_id;
  Message* msgInfo = nullptr;
  msg_struct_t platform_msg = nullptr;
  msg_struct_t send_opt = nullptr;
  msg_struct_t msg_conv = nullptr;
  msg_struct_t req = nullptr;

  std::shared_ptr<Message> message = callback->getMessage();
  MessageStatus msg_status = message->getMessageStatus();
  int ret = MSG_ERR_UNKNOWN;

  // if it is draft message just send it
  // in other case create new platform message
  // add it to draft and finally send it
  if (!( message->is_id_set() && MessageStatus::STATUS_DRAFT == msg_status)) {
    LoggerD("Add message to draft");
    platform_result = addDraftMessagePlatform(message);

  }
  if(platform_result.IsSuccess()) {
    msg_id = message->getId();
    LoggerD("Message ID: %d", msg_id);

    platform_msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
    send_opt = msg_create_struct(MSG_STRUCT_SENDOPT);
    msg_conv = msg_create_struct(MSG_STRUCT_CONV_INFO);
    ret = msg_get_message(m_msg_handle, msg_id, platform_msg, send_opt);
    if (MSG_SUCCESS != ret) {
      LoggerE("Failed to get platform message structure: %d", ret);
      platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get platform Message structure");
    } else {
      // Send message
      message->setMessageStatus(MessageStatus::STATUS_SENDING);
      req = msg_create_struct(MSG_STRUCT_REQUEST_INFO);
      msg_set_struct_handle(req, MSG_REQUEST_MESSAGE_HND, platform_msg);

      int req_id = -1;
      ret = msg_get_int_value(req, MSG_REQUEST_REQUESTID_INT, &req_id);
      if (MSG_SUCCESS != ret) {
        LoggerE("Failed to get send request ID: %d", ret);
        platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get send request ID");
      } else {
        if (MessageType::MMS == message->getType()) {
          LoggerD("Send MMS message");
          ret = msg_mms_send_message(m_msg_handle, req);
        }
        else if (MessageType::SMS == message->getType()) {
          LoggerD("Send SMS message");
          ret = msg_sms_send_message(m_msg_handle, req);
        }
        else {
          LoggerE("Invalid message type: %d", message->getType());
          platform_result = PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Invalid message type");
        }

        if (platform_result) {
          if (ret != MSG_SUCCESS) {
            LoggerE("Failed to send message: %d", ret);
            platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to send message");
          } else {
            ret = msg_get_int_value(req, MSG_REQUEST_REQUESTID_INT, &req_id);
            if (ret != MSG_SUCCESS) {
              LoggerE("Failed to get message request ID: %d", ret);
              platform_result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get send request");
            }
            if (platform_result.IsSuccess()) {
              LoggerD("req_id: %d", req_id);

              platform_result = Message::convertPlatformShortMessageToObject(platform_msg, &msgInfo);
              if (platform_result.IsSuccess()) {

                int conversationId;
                ret = msg_get_conversation(m_msg_handle, msg_id, msg_conv);
                if (MSG_SUCCESS != ret) {
                  LoggerE("Failed to get conv");
                }
                msg_get_int_value(msg_conv, MSG_CONV_MSG_THREAD_ID_INT,
                                  &conversationId);
                message->setConversationId(conversationId);

                int folderId = msgInfo->getFolderId();
                message->setFolderId(folderId);

                time_t timestamp = msgInfo->getTimestamp();
                message->setTimeStamp(timestamp);

                std::string from = msgInfo->getFrom();
                LoggerD("From:%s", from.c_str());
                message->setFrom(from);

                bool isRead = msgInfo->getIsRead();
                message->setIsRead(isRead);

                int inResponseTo = msgInfo->getInResponseTo();
                message->setInResponseTo(inResponseTo);

                m_sendRequests[req_id] = callback;
                LoggerD("Send MSG_SUCCESS");
              }
            }
          }
        }
      }
    }
  }

  if (msg_release_struct(&req) != MSG_SUCCESS) {
    LoggerW("Request structure is already destroyed");
  }
  if (msg_release_struct(&platform_msg) != MSG_SUCCESS) {
    LoggerW("Platform message is already destroyed");
  }
  if (msg_release_struct(&send_opt) != MSG_SUCCESS) {
    LoggerW("Platform message is already destroyed");
  }
  if (msg_release_struct(&msg_conv) != MSG_SUCCESS) {
    LoggerW("Platform message is already destroyed");
  }

  delete msgInfo;

  return platform_result;
}

PlatformResult ShortMsgManager::sendMessage(MessageRecipientsCallbackData* callback)
{
  LoggerD("Entered");

  if (!callback){
    LoggerE("Callback is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Callback is null");
  }

  PlatformResult platform_result(ErrorCode::NO_ERROR);

  platform_result = SendMessagePlatform(callback);

  if (!platform_result) {
    LoggerE("Message send failed");

    callback->setError(platform_result);

    if (!g_idle_add(sendMessageCompleteCB, static_cast<void*>(callback))) {
      LoggerE("g_idle addition failed");
      delete callback;
      callback = NULL;
    }
  }
  return platform_result;
}

void ShortMsgManager::sendStatusCallback(msg_struct_t sent_status)
{
    LoggerD("Entered");
    int reqId = 0;
    int status = MSG_NETWORK_NOT_SEND;

    msg_get_int_value(sent_status, MSG_SENT_STATUS_REQUESTID_INT, &reqId);
    LoggerD("Send msg %d", reqId);
    msg_get_int_value(sent_status, MSG_SENT_STATUS_NETWORK_STATUS_INT, &status);
    LoggerD("Send msg status: %d", status);

    if(MSG_NETWORK_SEND_SUCCESS != status
        && MSG_NETWORK_SEND_FAIL != status
        && MSG_NETWORK_SEND_TIMEOUT != status)
    {
        LoggerD("Not final status, return");
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    SendReqMap::iterator it = m_sendRequests.find(reqId);
    if (it != m_sendRequests.end()) {
        LoggerD("Matching request found");

        MessageRecipientsCallbackData* callback = it->second;
        m_sendRequests.erase(it);

        if (MSG_NETWORK_SEND_FAIL == status
                || MSG_NETWORK_SEND_TIMEOUT == status) {
            LoggerE("req_id:%d : Failed sending Message(%p) with msg_id:%d msg status is: %s",
                reqId,
                callback->getMessage().get(),
                callback->getMessage()->getId(),
                (MSG_NETWORK_SEND_FAIL == status ? "FAIL" : "TIMEOUT"));

            common::UnknownException e("Send message failed");
            callback->setError(e.name(), e.message());
        }

        if (!g_idle_add(sendMessageCompleteCB, static_cast<void*>(callback))) {
            LoggerE("g_idle addition failed");
            delete callback;
            callback = NULL;
        }
    }
    else {
        LoggerE("No matching request found");
    }

    return;
}

static void sent_status_cb(msg_handle_t handle,
        msg_struct_t sent_status,
        void *data)
{
    LoggerD("Entered");
    ShortMsgManager::getInstance().sendStatusCallback(sent_status);

    return;
}

PlatformResult ShortMsgManager::callProperEventMessages(EventMessages* event,
        msg_storage_change_type_t storageChangeType)
{
    LoggerD("Entered event.items.size()=%d event.removed_conversations.size()=%d"
            " sChangeType:%d", event->items.size(),
            event->removed_conversations.size(), storageChangeType);

    EventConversations* eventConv = new EventConversations();
    eventConv->service_id = event->service_id;
    eventConv->service_type = event->service_type;

    if(MSG_STORAGE_CHANGE_DELETE == storageChangeType) {
        eventConv->items = event->removed_conversations;
    } else {
        PlatformResult ret = ShortMsgManager::getConversationsForMessages(
                event->items, storageChangeType, &(eventConv->items));
        if (ret.IsError()) {
          LoggerD("Error while getting conversations for message");
          delete event;
          delete eventConv;
          return ret;
        }
    }

    switch (storageChangeType) {
        case MSG_STORAGE_CHANGE_INSERT: {
            ChangeListenerContainer::getInstance().callMessageAdded(event);
            if (!eventConv->items.empty()) {

                ConversationPtrVector added_conv;
                ConversationPtrVector updated_conv;

                for(ConversationPtrVector::iterator it = eventConv->items.begin();
                        it != eventConv->items.end(); ++it) {
                    ConversationPtr cur_conv = *it;
                    const bool new_conv = (cur_conv->getMessageCount() <= 1);
                    if(new_conv) {
                        added_conv.push_back(cur_conv);
                    } else {
                        updated_conv.push_back(cur_conv);
                    }

                    LoggerD("%s conversation with id:%d last_msg_id:d",
                            (new_conv ? "ADDED" : "UPDATED"),
                            cur_conv->getConversationId(), cur_conv->getLastMessageId());
                }

                LoggerD("num conversations:all=%d added=%d update=%d", eventConv->items.size(),
                        added_conv.size(), updated_conv.size());

                if(false == added_conv.empty()) {
                    LoggerD("%d new conversations, calling onConversationAdded",
                            added_conv.size());
                    eventConv->items = added_conv;
                    ChangeListenerContainer::getInstance().callConversationAdded(
                            eventConv);
                }

                if(false == updated_conv.empty()) {
                    LoggerD("%d updated conversation, calling onConversationUpdated",
                            updated_conv.size());
                    eventConv->items = updated_conv;
                    ChangeListenerContainer::getInstance().callConversationUpdated(
                            eventConv);
                }

            }
        } break;
        case MSG_STORAGE_CHANGE_DELETE: {
            ChangeListenerContainer::getInstance().callMessageRemoved(event);

            if(false == eventConv->items.empty()) {
                LoggerD("At least one conversation will be deleted, "
                     "triggering also onConversationRemoved");
                ChangeListenerContainer::getInstance().callConversationRemoved(eventConv);
            }
        } break;
        case MSG_STORAGE_CHANGE_UPDATE: {
            ChangeListenerContainer::getInstance().callMessageUpdated(event);
            ChangeListenerContainer::getInstance().callConversationUpdated(eventConv);
        } break;
        default:
            LoggerW("Unknown storageChangeType: %d", storageChangeType);
    }
    delete event;
    delete eventConv;
    return PlatformResult(ErrorCode::NO_ERROR);
}

void ShortMsgManager::storage_change_cb(msg_handle_t handle,
        msg_storage_change_type_t storageChangeType,
        msg_id_list_s *pMsgIdList,
        void* data)
{
    LoggerD("Entered handle:%p sChangeType:%d numMsgs:%d", handle, storageChangeType,
            pMsgIdList->nCount);

    if (MSG_STORAGE_CHANGE_CONTACT == storageChangeType) {
        LoggerD("storageChangeType is MSG_STORAGE_CHANGE_CONTACT, ignoring");
        return;
    }

    if (pMsgIdList->nCount < 1) {
        LoggerW("no messages in callback list");
        return;
    }

    LoggerD("Messages count %d", pMsgIdList->nCount);

    /*
     * There is possibility that in one callback from msg service will come
     * SMS and MMS messages in the same list. ChangeListenerContainer requires
     * that messages in event have common service_id and service_type. So we
     * create here 2 events: one for SMS and one for MMS. If one of events
     * has empty message list, we won't trigger it.
     */
    EventMessages* eventSMS = NULL;
    EventMessages* eventMMS = NULL;

    // if allocation below fails than exception is thrown - no NULL check
    eventSMS = new EventMessages();
    eventSMS->service_type = MessageType::SMS;
    eventSMS->service_id = SMS_ACCOUNT_ID;
    eventMMS = new EventMessages();
    eventMMS->service_type = MessageType::MMS;
    eventMMS->service_id = MMS_ACCOUNT_ID;

    if (MSG_STORAGE_CHANGE_DELETE == storageChangeType) {

        ShortMsgManager& msg_manager = ShortMsgManager::getInstance();
        std::lock_guard<std::mutex> lock(msg_manager.m_mutex);

        std::map<int, MessagePtr>* rem_msgs[2] = {  // Recently removed messages
                &msg_manager.m_sms_removed_messages,
                &msg_manager.m_mms_removed_messages };
        std::map<int, int>* rem_convs[2] = { // Recently removed conversations
                &msg_manager.m_sms_removed_msg_id_conv_id_map,
                &msg_manager.m_mms_removed_msg_id_conv_id_map };
        EventMessages* dest_event[2] = { // SMS/MMS EventMessage to be propagated
                eventSMS,
                eventMMS };
        std::map<int, ConversationPtr>* conv_map[2] = { //Map conversationId - object
                &msg_manager.m_sms_removed_conv_id_object_map,
                &msg_manager.m_mms_removed_conv_id_object_map };

        for(int event_i = 0; event_i < 2; ++event_i) {

            std::map<int, MessagePtr>& cur_rem_msgs = *(rem_msgs[event_i]);
            std::map<int, int>& cur_rem_convs = *(rem_convs[event_i]);
            EventMessages* cur_dest_event = dest_event[event_i];
            std::map<int, ConversationPtr>& cur_conv_map = *(conv_map[event_i]);
            std::unordered_set<int> conv_rem_now;

            for (int i = 0; i < pMsgIdList->nCount; ++i) {
                const msg_message_id_t& msg_id = pMsgIdList->msgIdList[i];
                LoggerD("pMsgIdList[%d] = %d", i, msg_id);

                std::map<int, MessagePtr> ::iterator it = cur_rem_msgs.find(msg_id);
                if(it != cur_rem_msgs.end()) {
                    LoggerD("[%d] is %s, Pushing message with id:%d subject:%s", i,
                            (0 == i) ? "SMS" : "MMS",
                            it->second->getId(),
                            it->second->getSubject().c_str());
                    cur_dest_event->items.push_back(it->second);
                    cur_rem_msgs.erase(it);
                }

                std::map<int, int>::iterator cit = cur_rem_convs.find(msg_id);
                if(cit != cur_rem_convs.end()) {
                    conv_rem_now.insert(cit->second);
                    cur_rem_convs.erase(cit);
                }
            }

            for (auto it = conv_rem_now.begin(); it != conv_rem_now.end(); it++) {
                const int cur_rem_conv_id = *it;

                //---------------------------------------------------------------------
                // Check if we have removed last message from conversation
                //
                bool found = false;
                for(auto it2 = cur_rem_convs.begin();
                        it2 != cur_rem_convs.end();
                        it2++) {
                    if( cur_rem_conv_id == it2->second) {
                        found = true;
                        break;
                    }
                }

                if(false == found) {
                    //We have removed last message from conversation

                    std::map<int, ConversationPtr>::iterator conv_it =
                        cur_conv_map.find(cur_rem_conv_id);
                    if(conv_it != cur_conv_map.end()) {
                        LoggerD("Pushing removed %s MessageConversation(%p) with id:%d",
                                (0 == event_i) ? "SMS" : "MMS",
                                conv_it->second.get(), cur_rem_conv_id);

                        cur_dest_event->removed_conversations.push_back(
                            conv_it->second);
                        cur_conv_map.erase(conv_it);
                    } else {
                        LoggerW("Couldn't find ConversationPtr object with id:%d",
                                cur_rem_conv_id);
                    }
                }
            }
        }

    } else {
        PlatformResult ret(ErrorCode::NO_ERROR);
        for (int i = 0; i < pMsgIdList->nCount; ++i) {

            msg_struct_t msg;
            ret = ShortMsgManager::getInstance().getMessage(pMsgIdList->msgIdList[i], &msg);
            if (ret.IsError() || NULL == msg) {
                LoggerE("Failed to load short message");
                delete eventSMS;
                eventSMS = NULL;
                delete eventMMS;
                eventMMS = NULL;
                return;
            }
            std::shared_ptr<Message> message;
            Message* message_ptr = nullptr;
            ret = Message::convertPlatformShortMessageToObject(msg, &message_ptr);
            if (ret.IsError()) {
                LoggerE("Failed to load short message");
                msg_release_struct(&msg);
                delete eventSMS;
                eventSMS = NULL;
                delete eventMMS;
                eventMMS = NULL;
                return;
            }
            message.reset(message_ptr);
            msg_release_struct(&msg);
            switch (message->getType()) {
                case MessageType::SMS:
                    eventSMS->items.push_back(message);
                    break;
                case MessageType::MMS:
                    eventMMS->items.push_back(message);
                    break;
                default:
                    LoggerE("Unsupported message type");
                    delete eventSMS;
                    eventSMS = NULL;
                    delete eventMMS;
                    eventMMS = NULL;
                    return;
            }
        }
    }

    if (!eventSMS->items.empty() || !eventSMS->removed_conversations.empty()) {
        PlatformResult ret = ShortMsgManager::callProperEventMessages(eventSMS, storageChangeType);
        //PlatformResult could be ignored here. eventSMS is deleted in callProperEventMessages()
    } else {
        LoggerD("No SMS messages, not triggering eventSMS");
        delete eventSMS;
        eventSMS = NULL;
    }
    if (!eventMMS->items.empty() || !eventMMS->removed_conversations.empty()) {
        PlatformResult ret = ShortMsgManager::callProperEventMessages(eventMMS, storageChangeType);
        //PlatformResult could be ignored here. eventMMS is deleted in callProperEventMessages()
    } else {
        LoggerD("No MMS messages, not triggering eventMMS");
        delete eventMMS;
        eventMMS = NULL;
    }
}

void ShortMsgManager::registerStatusCallback(msg_handle_t msg_handle)
{
    LoggerD("Entered");
    m_msg_handle = msg_handle;
    // set message sent status callback
    if (MSG_SUCCESS != msg_reg_sent_status_callback(m_msg_handle,
            &sent_status_cb, NULL)) {
        LoggerE("sent status callback register error!!!");
    }
    if (MSG_SUCCESS != msg_reg_storage_change_callback(m_msg_handle,
            &storage_change_cb, NULL)) {
        LoggerE("storage change callback register error!");
    }
}

void ShortMsgManager::addDraftMessage(MessageCallbackUserData* callback)
{
    LoggerD("Enter");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      std::shared_ptr<Message> message = callback->getMessage();

      PlatformResult ret = addDraftMessagePlatform(message);
      if (ret.IsError()) {
        LoggerE("%d (%s)", ret.error_code(), ret.message().c_str());
        callback->setError(ret);
      }
    }

    // Complete task
    if (!g_idle_add(addDraftMessageCompleteCB, static_cast<void *>(callback))) {
        LoggerE("g_idle addition failed");
        delete callback;
        callback = NULL;
    }
}

void ShortMsgManager::removeMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    std::vector<std::shared_ptr<Message>> messages;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        messages = callback->getMessages();
        MessageType type = callback->getMessageServiceType();
        for(auto it = messages.begin() ; it != messages.end(); ++it) {
            if((*it)->getType() != type) {
                LoggerE("Invalid message type: %d", (*it)->getType());
                callback->SetError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Error while deleting message"));
                break;
            }
        }

        if (!callback->isError()) {
            for (auto it = messages.begin() ; it != messages.end(); ++it) {
                const int id = (*it)->getId();

                //Store message object
                LoggerD("Storing removed message (id:%d) in m_removed_messages", id);
                switch((*it)->getType()) {

                    case SMS:  m_sms_removed_messages[id] = (*it); break;
                    case MMS:  m_mms_removed_messages[id] = (*it); break;
                    default:
                        LoggerD("Unknown message type: %d", (*it)->getType());
                        break;
                }

               int error = msg_delete_message(m_msg_handle, id);
                if (MSG_SUCCESS != error) {
                    LoggerE("Error while deleting message");
                    callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while deleting message"));
                    break;
                }
            }
        }
    }

    if (callback->isError()) {
        LoggerD("Calling error callback");
        callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
    } else {
        LoggerD("Calling success callback");

        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
    }

    delete callback;
    callback = NULL;
}

void ShortMsgManager::updateMessages(MessagesCallbackUserData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    LoggerD("messages to update: %d", callback->getMessages().size());

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
        MessageType type = callback->getMessageServiceType();
        for (auto it = messages.begin() ; it != messages.end(); ++it) {
            if ((*it)->getType() != type) {
                LoggerE("Invalid message type");
                callback->SetError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Error while updating message"));
                break;
            }
        }
        if (!callback->isError()) {
            for (auto it = messages.begin() ; it != messages.end(); ++it) {
                LoggerD("updating Message(%p) msg_id:%d", (*it).get(), (*it)->getId());

                msg_struct_t platform_msg = nullptr;
                PlatformResult ret = Message::convertPlatformShortMessageToStruct(it->get(), m_msg_handle, &platform_msg);
                if (ret.IsError()) {
                    LoggerE("%s", ret.message().c_str());
                    callback->SetError(ret);
                    break;
                }
                if (NULL == platform_msg) {
                    LoggerE("Failed to prepare platform message");
                    callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot prepare platform message"));
                    break;
                }
                msg_struct_t sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
                int error = msg_update_message(m_msg_handle, platform_msg, sendOpt);
                msg_release_struct(&platform_msg);
                msg_release_struct(&sendOpt);
                if (error != MSG_SUCCESS) {
                    LoggerE("Failed to update message %d", (*it)->getId());
                    callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while updating message"));
                    break;
                }
            }
        }
    }

    if (callback->isError()) {
        LoggerD("Calling error callback");

        callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
    } else {
        LoggerD("Calling success callback");

        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();

        auto messages = callback->getMessages();
        picojson::array array;
        auto each = [&array] (std::shared_ptr<Message> m)->void {
            array.push_back(MessagingUtil::messageToJson(m));
        };

        for_each(messages.begin(), messages.end(), each);

        obj[JSON_DATA] = picojson::value(array);

        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
    }

    delete callback;
    callback = NULL;
}

PlatformResult ShortMsgManager::getMessage(int msg_id, msg_struct_t* out_msg)
{
    LoggerD("Entered");
    msg_struct_t sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
    msg_struct_t msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);

    int error = msg_get_message(m_msg_handle, msg_id, msg, sendOpt);
    if (MSG_SUCCESS != error) {
        LoggerE("Couldn't retrieve message from service, msgId: %d, error:%d", msg_id, error);
        msg_release_struct(&sendOpt);
        msg_release_struct(&msg);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Couldn't retrieve message from service");
    }
    msg_release_struct(&sendOpt);
    *out_msg = msg;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ShortMsgManager::getConversationsForMessages(
        MessagePtrVector messages,
        msg_storage_change_type_t storageChangeType, ConversationPtrVector* result)
{
    LoggerD("Entered messages.size()=%d storageChangeType=%d", messages.size(),
            storageChangeType);

    std::unordered_set<int> unique_conv_ids;
    ConversationPtrVector convs;
    for (auto it = messages.begin(); it != messages.end(); ++it) {

        MessagePtr msg = (*it);
        const int conv_id = msg->getConversationId();
        const int count = unique_conv_ids.count(conv_id);
        LoggerD("Message(%p) msg_id:%d conversationId: %d count:%d", msg.get(),
                msg->getId(), conv_id, count);

        if (0 == count) {
            //conversation isn't loaded yet
            unique_conv_ids.insert(conv_id);
            ConversationPtr conv;
            PlatformResult ret = MessageConversation::convertMsgConversationToObject(
                    conv_id, ShortMsgManager::getInstance().m_msg_handle, &conv);
            if (ret.IsError()) {
                LoggerD("Convert msg conversation to object failed (%s)", ret.message().c_str());
                return ret;
            }
            LoggerD("Pushed conv=%p", conv.get());
            convs.push_back(conv);
        }
    }
    *result = convs;
    return PlatformResult(ErrorCode::NO_ERROR);
}

void ShortMsgManager::findMessages(FindMsgCallbackUserData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<int> messagesIds;
        PlatformResult ret = MessagingDatabaseManager::getInstance().findShortMessages(callback, &messagesIds);
        if (ret.IsError()) {
            LoggerE("Failed to find short message: %s (%d)", ret.message().c_str(), ret.error_code());
            callback->SetError(ret);
        }

        if (!callback->isError()) {
            int msgListCount = messagesIds.size();
            LoggerD("Found %d messages", msgListCount);

            msg_struct_t msg;
            msg_struct_t send_opt;
            msg_error_t err;
            for (int i = 0; i < msgListCount; i++) {
                msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
                send_opt = msg_create_struct(MSG_STRUCT_SENDOPT);
                std::unique_ptr<msg_struct_t, int (*)(msg_struct_t*)> msg_ptr(&msg, msg_release_struct);
                std::unique_ptr<msg_struct_t, int (*)(msg_struct_t*)> send_opt_ptr(&send_opt, msg_release_struct);

                err = msg_get_message(m_msg_handle, messagesIds.at(i), msg, send_opt);

                if (MSG_SUCCESS != err) {
                    LoggerE("Failed to get platform message structure: %d", err);
                    callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get platform Message structure"));
                    break;
                }

                Message* message = nullptr;
                PlatformResult ret = Message::convertPlatformShortMessageToObject(msg, &message);
                if (ret.IsError()) {
                    if (ErrorCode::INVALID_VALUES_ERR == ret.error_code()) {
                      LoggerW("Ignore messages with not supported/unrecognized type");
                      continue;
                    }
                    LoggerE("Cannot get platform Message structure");
                    callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get platform Message structure"));
                    break;
                }
                if (!callback->isError()) {
                    callback->addMessage(std::shared_ptr<Message>{message});
                    LoggerD("Created message with id %d:", messagesIds[i]);
                }
            }
        }
    }

    if (callback->isError()) {
        LoggerD("Calling error callback");
        callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
    } else {
        LoggerD("Calling success callback with %d messages:",
                callback->getMessages().size());

        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();

        std::vector<picojson::value> response;
        auto messages = callback->getMessages();
        std::for_each(messages.begin(), messages.end(), [&response](MessagePtr &message){
            response.push_back(MessagingUtil::messageToJson(message));
        });

        obj[JSON_DATA] = picojson::value(response);
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
    }

    delete callback;
    callback = NULL;
}

void ShortMsgManager::findConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<int> conversationsIds;
        PlatformResult ret = MessagingDatabaseManager::getInstance().
            findShortMessageConversations(callback, &conversationsIds);
        if (ret.IsError()) {
            LoggerE("Cannot get platform Message structure");
            callback->SetError(ret);
        }

        if (!callback->isError()) {
            int convListCount = conversationsIds.size();
            LoggerD("Found %d conversations", convListCount);

            for (int i = 0; i < convListCount; i++) {
                std::shared_ptr<MessageConversation> conversation;
                PlatformResult ret = MessageConversation::convertMsgConversationToObject(
                                conversationsIds.at(i), m_msg_handle, &conversation);
                if (ret.IsSuccess()) {
                  callback->addConversation(conversation);
                } else {
                  callback->SetError(ret);
                }
            }
        }
    }

    if (callback->isError()) {
        LoggerD("Calling error callback");
        callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
    } else {
        LoggerD("Calling success callback");
        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();

        std::vector<picojson::value> response;
        auto conversations = callback->getConversations();
        std::for_each(conversations.begin(), conversations.end(),
                [&response](std::shared_ptr<MessageConversation> &conversation) {
                    response.push_back(MessagingUtil::conversationToJson(conversation));
                }
        );
        obj[JSON_DATA] = picojson::value(response);
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
    }

    delete callback;
    callback = NULL;
}

void ShortMsgManager::removeConversations(ConversationCallbackData* callback)
{
    LoggerD("Entered");

    if (!callback){
        LoggerE("Callback is null");
        return;
    }

    int error = MSG_SUCCESS;
    msg_handle_t handle = NULL;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ConversationPtrVector conversations = callback->getConversations();
        const MessageType type = callback->getMessageServiceType();

        std::map<int, int>* msg_id_conv_id_map = NULL;
        std::map<int, ConversationPtr>* conv_id_object_map = NULL;

        error = msg_open_msg_handle(&handle);
        if (MSG_SUCCESS != error) {
            LoggerE("Open message handle error: %d", error);
            callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Error while creatng message handle"));
        }

        if (!callback->isError()) {
          for(auto it = conversations.begin() ; it != conversations.end(); ++it) {
              if((*it)->getType() != type) {
                  LoggerE("Invalid message type");
                  callback->SetError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR,
                                     "Error while deleting message conversation"));
                  break;
              }
          }
        }

        if (!callback->isError()) {
            if(MessageType::SMS == type) {
                msg_id_conv_id_map = &m_sms_removed_msg_id_conv_id_map;
                conv_id_object_map = &m_sms_removed_conv_id_object_map;
            } else if(MessageType::MMS == type) {
                msg_id_conv_id_map = &m_mms_removed_msg_id_conv_id_map;
                conv_id_object_map = &m_mms_removed_conv_id_object_map;
            } else {
                LoggerE("Invalid message type:%d for ShortMsgManager!", type);
                callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid message type for ShortMsgManager!"));
            }
        }

        if (!callback->isError()) {
            int conv_index = 0;
            for (auto it = conversations.begin() ; it != conversations.end();
                        ++it, ++conv_index) {

                ConversationPtr conv = (*it);
                msg_thread_id_t conv_id = conv->getConversationId();

                LoggerD("[%d] MessageConversation(%p) conv_id:%d", conv_index, conv.get(),
                        conv_id);

                msg_struct_list_s conv_view_list;
                error = msg_get_conversation_view_list(handle, (msg_thread_id_t)conv_id,
                        &conv_view_list);
                if (MSG_SUCCESS == error) {
                    for(int msg_index = 0; msg_index < conv_view_list.nCount; ++msg_index)
                    {
                        int cur_msg_id = 0;
                        error = msg_get_int_value(conv_view_list.msg_struct_info[msg_index],
                                MSG_CONV_MSG_ID_INT, &cur_msg_id);

                        if(MSG_SUCCESS == error && cur_msg_id > 0) {
                            (*msg_id_conv_id_map)[cur_msg_id] = conv_id;
                            (*conv_id_object_map)[conv_id] = conv;

                            LoggerD("[%d] message[%d] msg_id:%d,"
                                    "saved MessageConversation(%p) with conv_id:%d",
                                    conv_index, msg_index, cur_msg_id, conv.get(), conv_id);
                        } else {
                            LoggerE("[%d] Couldn't get msg_id, error: %d!", error);
                        }
                    }
                } else {
                    LoggerE("[%d] Couldn' get conversation view list for conv_id:%d error: %d",
                            conv_index, conv_id, error);
                }

                msg_release_list_struct(&conv_view_list);

                error = msg_delete_thread_message_list(handle, (msg_thread_id_t) conv_id,
                        FALSE);
                if (MSG_SUCCESS != error) {
                    LoggerE("Error while deleting message conversation");
                    callback->SetError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                                                      "Error while deleting message conversation"));
                    break;
                }
            }
        }
    }

    if (!callback->isError()) {
        error = msg_close_msg_handle(&handle);
        if (MSG_SUCCESS != error) {
            LoggerW("Cannot close message handle: %d", error);
        }
    }

    if (callback->isError()) {
        LoggerD("Calling error callback");
        callback->getQueue().resolve(
                callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<double>(),
                callback->getJson()->serialize()
        );
    } else {
        LoggerD("Calling success callback");

        auto json = callback->getJson();
        picojson::object& obj = json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

        callback->getQueue().resolve(
                obj.at(JSON_CALLBACK_ID).get<double>(),
                json->serialize()
        );
    }

    delete callback;
    callback = NULL;
}

ShortMsgManager::ShortMsgManager() : m_msg_handle(NULL)
{
    LoggerD("Entered");
}

ShortMsgManager::~ShortMsgManager()
{
    LoggerD("Entered");
    LoggerD("m_sms_removed_messages.size() = %d",
            m_sms_removed_messages.size());
    LoggerD("m_mms_removed_messages.size() = %d",
            m_mms_removed_messages.size());
    LoggerD("m_sms_removed_msg_id_conv_id_map.size() = %d",
            m_sms_removed_msg_id_conv_id_map.size());
    LoggerD("m_sms_removed_conv_id_object_map.size() = %d",
            m_sms_removed_conv_id_object_map.size());
    LoggerD("m_mms_removed_msg_id_conv_id_map.size() = %d",
            m_mms_removed_msg_id_conv_id_map.size());
    LoggerD("m_mms_removed_conv_id_object_map.size() = %d",
            m_mms_removed_conv_id_object_map.size());
}

} // messaging
} // extension
