// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <msg.h>
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

    try {
        if (callback->isError()) {
            PostQueue::getInstance().resolve(
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

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
            callback->getMessage()->setMessageStatus(MessageStatus::STATUS_SENT);
        }
    }
    catch (const common::PlatformException& err) {
        LoggerE("Error while calling sendMessage callback: %s (%s)",
                (err.name()).c_str(),(err.message()).c_str());
    }
    catch (...) {
        LoggerE("Unknown error when calling sendMessage callback.");
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

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");

            PostQueue::getInstance().resolve(
                    callback->getJson()->get<picojson::object>().at(JSON_CALLBACK_ID).get<long>(),
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
    } catch (const common::PlatformException& err) {
        LoggerE("Error while calling addDraftMessage callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling addDraftMessage callback.");
    }

    delete callback;
    callback = NULL;

    return FALSE;
}


void ShortMsgManager::addDraftMessagePlatform(std::shared_ptr<Message> message)
{
    LoggerD("Add new message(%p)", message.get());

    // Save platform msg to get ID
    msg_struct_t platform_msg
            = Message::convertPlatformShortMessageToStruct(message.get(), m_msg_handle);
    if (NULL == platform_msg) {
        LoggerE("Failed to prepare platform message");
        throw common::UnknownException("Cannot prepare platform message");
    }

    msg_struct_t send_opt = msg_create_struct(MSG_STRUCT_SENDOPT);
    msg_set_bool_value(send_opt, MSG_SEND_OPT_SETTING_BOOL, false);
    const int msg_id = msg_add_message(m_msg_handle, platform_msg, send_opt);
    if (msg_id < MSG_SUCCESS) {
        LoggerE("Message(%p): Failed to add draft, error: %d", message.get(), msg_id);
        msg_release_struct(&send_opt);
        msg_release_struct(&platform_msg);
        throw common::UnknownException("Cannot add message to draft");
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

    Message* msgInfo = Message::convertPlatformShortMessageToObject(
            platform_msg);

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
}

void ShortMsgManager::sendMessage(MessageRecipientsCallbackData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    int msg_id;
    Message* msgInfo = NULL;
    msg_struct_t platform_msg = NULL;
    msg_struct_t send_opt = NULL;
    msg_struct_t msg_conv = NULL;
    msg_struct_t req = NULL;

    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::shared_ptr<Message> message = callback->getMessage();
        MessageStatus msg_status = message->getMessageStatus();
        int ret = MSG_ERR_UNKNOWN;

        // if it is draft message just send it
        // in other case create new platform message
        // add it to draft and finally send it
        if (!( message->is_id_set() && MessageStatus::STATUS_DRAFT == msg_status)) {
            LoggerD("Add message to draft");
            addDraftMessagePlatform(message);
        }

        msg_id = message->getId();
        LoggerD("Message ID: %d", msg_id);

        platform_msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
        send_opt = msg_create_struct(MSG_STRUCT_SENDOPT);
        msg_conv = msg_create_struct(MSG_STRUCT_CONV_INFO);
        ret = msg_get_message(m_msg_handle, msg_id, platform_msg, send_opt);
        if (MSG_SUCCESS != ret) {
            LoggerE("Failed to get platform message structure: %d", ret);
            throw common::UnknownException("Cannot get platform Message structure");
        }

        // Send message
        message->setMessageStatus(MessageStatus::STATUS_SENDING);
        req = msg_create_struct(MSG_STRUCT_REQUEST_INFO);
        msg_set_struct_handle(req, MSG_REQUEST_MESSAGE_HND, platform_msg);

        int req_id = -1;
        ret = msg_get_int_value(req, MSG_REQUEST_REQUESTID_INT, &req_id);
        if (MSG_SUCCESS != ret) {
            LoggerE("Failed to get send request ID: %d", ret);
            throw common::UnknownException("Failed to get send request ID");
        }

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
            throw common::TypeMismatchException("Invalid message type");
        }

        if (ret != MSG_SUCCESS) {
            LoggerE("Failed to send message: %d", ret);
            throw common::UnknownException("Failed to send message");
        }

        ret = msg_get_int_value(req, MSG_REQUEST_REQUESTID_INT, &req_id);
        if (ret != MSG_SUCCESS) {
            LoggerE("Failed to get message request ID: %d", ret);
            throw common::UnknownException("Failed to get send request");
        }
        LoggerD("req_id: %d", req_id);

        msgInfo = Message::convertPlatformShortMessageToObject(platform_msg);

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
    catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
        if (!g_idle_add(sendMessageCompleteCB, static_cast<void*>(callback))) {
            LoggerE("g_idle addition failed");
            delete callback;
            callback = NULL;
        }
    }
    catch (...) {
        LoggerE("Message send failed");
        common::UnknownException e("Message send failed");
        callback->setError(e.name(), e.message());
        if (!g_idle_add(sendMessageCompleteCB, static_cast<void*>(callback))) {
            LoggerE("g_idle addition failed");
            delete callback;
            callback = NULL;
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

    return;
}

void ShortMsgManager::sendStatusCallback(msg_struct_t sent_status)
{
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

void ShortMsgManager::callProperEventMessages(EventMessages* event,
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
        eventConv->items = ShortMsgManager::getConversationsForMessages(
                event->items, storageChangeType);
    }

    switch (storageChangeType) {
        case MSG_STORAGE_CHANGE_INSERT: {
            ChangeListenerContainer::getInstance().callMessageAdded(event);
            if (!eventConv->items.empty()) {

                ConversationPtrVector added_conv;
                ConversationPtrVector updated_conv;

                for(ConversationPtrVector::iterator it = eventConv->items.begin();
                        it != eventConv->items.end(); it++) {
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
    try {
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
            for (int i = 0; i < pMsgIdList->nCount; ++i) {

                msg_struct_t msg = ShortMsgManager::getInstance().getMessage(
                        pMsgIdList->msgIdList[i]);
                if (NULL == msg) {
                    LoggerE("Failed to load short message");
                    delete eventSMS;
                    eventSMS = NULL;
                    delete eventMMS;
                    eventMMS = NULL;
                    throw common::UnknownException("Failed to load short message");
                }
                std::shared_ptr<Message> message(
                        Message::convertPlatformShortMessageToObject(msg));
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
                        throw common::UnknownException("Unsupported message type");
                }
            }
        }

        if (!eventSMS->items.empty() || !eventSMS->removed_conversations.empty()) {
            ShortMsgManager::callProperEventMessages(eventSMS, storageChangeType);
        } else {
            LoggerD("No SMS messages, not triggering eventSMS");
            delete eventSMS;
            eventSMS = NULL;
        }
        if (!eventMMS->items.empty() || !eventMMS->removed_conversations.empty()) {
            ShortMsgManager::callProperEventMessages(eventMMS, storageChangeType);
        } else {
            LoggerD("No MMS messages, not triggering eventMMS");
            delete eventMMS;
            eventMMS = NULL;
        }

    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        delete eventSMS;
        delete eventMMS;
    } catch (...) {
        LoggerE("Failed to call callback");
        delete eventSMS;
        delete eventMMS;
    }
}

void ShortMsgManager::registerStatusCallback(msg_handle_t msg_handle)
{
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
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::shared_ptr<Message> message = callback->getMessage();

        addDraftMessagePlatform(message);

    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Message add draft failed");
        common::UnknownException e("Message add draft failed");
        callback->setError(e.name(), e.message());
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

    int error;
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
        MessageType type = callback->getMessageServiceType();
        for(auto it = messages.begin() ; it != messages.end(); ++it) {
            if((*it)->getType() != type) {
                LoggerE("Invalid message type: %d", (*it)->getType());
                throw common::TypeMismatchException("Error while deleting message");
            }
        }
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

            error = msg_delete_message(m_msg_handle, id);
            if (MSG_SUCCESS != error) {
                LoggerE("Error while deleting message");
                throw common::UnknownException("Error while deleting message");
            }
        }
    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Messages remove failed");
        common::UnknownException e("Messages remove failed");
        callback->setError(e.name(), e.message());
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
    } catch (const common::PlatformException& err) {
        LoggerE("Error while calling removeShortMsg callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling removeShortMsg callback.");
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
    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::shared_ptr<Message>> messages = callback->getMessages();
        MessageType type = callback->getMessageServiceType();
        for (auto it = messages.begin() ; it != messages.end(); ++it) {
            if ((*it)->getType() != type) {
                LoggerE("Invalid message type");
                throw common::TypeMismatchException("Error while updating message");
            }
        }
        for (auto it = messages.begin() ; it != messages.end(); ++it) {

            LoggerD("updating Message(%p) msg_id:%d", (*it).get(), (*it)->getId());

            msg_struct_t platform_msg
                    = Message::convertPlatformShortMessageToStruct(it->get(), m_msg_handle);
            if (NULL == platform_msg) {
                LoggerE("Failed to prepare platform message");
                throw common::UnknownException("Cannot prepare platform message");
            }
            msg_struct_t sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
            int error = msg_update_message(m_msg_handle, platform_msg, sendOpt);
            msg_release_struct(&platform_msg);
            msg_release_struct(&sendOpt);
            if (error != MSG_SUCCESS) {
                LoggerE("Failed to update message %d", (*it)->getId());
                throw common::UnknownException("Error while updating message");
            }
        }
    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Messages update failed");
        common::UnknownException e("Messages update failed");
        callback->setError(e.name(), e.message());
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

            auto messages = callback->getMessages();
            picojson::array array;
            auto each = [&array] (std::shared_ptr<Message> m)->void {
                array.push_back(MessagingUtil::messageToJson(m));
            };

            for_each(messages.begin(), messages.end(), each);

            obj[JSON_DATA] = picojson::value(array);

            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const common::PlatformException& err) {
        LoggerE("Error while calling updateShortMsg callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling updateShortMsg callback.");
    }

    delete callback;
    callback = NULL;
}

msg_struct_t ShortMsgManager::getMessage(int msg_id)
{
    msg_struct_t sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
    msg_struct_t msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
    int error = msg_get_message(m_msg_handle, msg_id, msg, sendOpt);
    if (MSG_SUCCESS != error) {
        LoggerE("Couldn't retrieve message from service, msgId: %d, error:%d", msg_id, error);
        throw common::UnknownException("Couldn't retrieve message from service");
    }
    msg_release_struct(&sendOpt);
    return msg;
}

ConversationPtrVector ShortMsgManager::getConversationsForMessages(
        MessagePtrVector messages,
        msg_storage_change_type_t storageChangeType)
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
            ConversationPtr conv = MessageConversation::convertMsgConversationToObject(
                    conv_id, ShortMsgManager::getInstance().m_msg_handle);

            LoggerD("Pushed conv=%p", conv.get());
            convs.push_back(conv);
        }
    }
    return convs;
}

void ShortMsgManager::findMessages(FindMsgCallbackUserData* callback)
{
    LoggerD("Entered");

    if(!callback){
        LoggerE("Callback is null");
        return;
    }

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<int> messagesIds =
                MessagingDatabaseManager::getInstance().findShortMessages(callback);
        int msgListCount = messagesIds.size();
        LoggerD("Found %d messages", msgListCount);

        msg_struct_t msg;
        msg_struct_t sendOpt;
        msg_error_t err;
        for (int i = 0; i < msgListCount; i++) {
            msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);
            sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
            err = msg_get_message(m_msg_handle, messagesIds.at(i), msg, sendOpt);

            if (MSG_SUCCESS != err) {
                LoggerE("Failed to get platform message structure: %d", err);
                throw common::UnknownException("Cannot get platform Message structure");
            }

            try {
                std::shared_ptr<Message> message(
                    Message::convertPlatformShortMessageToObject(msg));
                callback->addMessage(message);

                LoggerD("Created message with id %d:", messagesIds[i]);
            }
            catch(const common::InvalidValuesException& exception) {
                //Ignore messages with not supported/unrecognized type
            }

            msg_release_struct(&sendOpt);
            msg_release_struct(&msg);
        }

    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Message add draft failed");
        common::UnknownException e("Message add draft failed");
        callback->setError(e.name(), e.message());
    }

    try {
        if (callback->isError()) {
            LoggerD("Calling error callback");
            PostQueue::getInstance().resolve(
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

            PostQueue::getInstance().resolve(
                    obj.at(JSON_CALLBACK_ID).get<double>(),
                    json->serialize()
            );
        }
    } catch (const common::PlatformException& err) {
        LoggerE("Error while calling findMessages callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Failed to call findMessages callback.");
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

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<int> conversationsIds =
                MessagingDatabaseManager::getInstance().findShortMessageConversations(callback);
        int convListCount = conversationsIds.size();
        LoggerD("Found %d conversations", convListCount);

        for (int i = 0; i < convListCount; i++) {
            std::shared_ptr<MessageConversation> conversation =
                    MessageConversation::convertMsgConversationToObject(
                            conversationsIds.at(i), m_msg_handle);

            callback->addConversation(conversation);
        }
    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Message add draft failed");
        common::UnknownException e("Message add draft failed");
        callback->setError(e.name(), e.message());
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

            std::vector<picojson::value> response;
            auto conversations = callback->getConversations();
            std::for_each(conversations.begin(), conversations.end(),
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
    } catch (const common::PlatformException& err) {
        LoggerE("Error while calling findConversations callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Failed to call findConversations callback.");
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

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        ConversationPtrVector conversations = callback->getConversations();
        const MessageType type = callback->getMessageServiceType();

        error = msg_open_msg_handle(&handle);
        if (MSG_SUCCESS != error) {
            LoggerE("Open message handle error: %d", error);
            throw common::UnknownException("Error while creatng message handle");
        }

        for(auto it = conversations.begin() ; it != conversations.end(); ++it) {
            if((*it)->getType() != type) {
                LoggerE("Invalid message type");
                throw common::TypeMismatchException("Error while deleting message conversation");
            }
        }

        std::map<int, int>* msg_id_conv_id_map = NULL;
        std::map<int, ConversationPtr>* conv_id_object_map = NULL;
        if(MessageType::SMS == type) {
            msg_id_conv_id_map = &m_sms_removed_msg_id_conv_id_map;
            conv_id_object_map = &m_sms_removed_conv_id_object_map;
        } else if(MessageType::MMS == type) {
            msg_id_conv_id_map = &m_mms_removed_msg_id_conv_id_map;
            conv_id_object_map = &m_mms_removed_conv_id_object_map;
        } else {
            LoggerE("Invalid message type:%d for ShortMsgManager!", type);
            throw common::UnknownException("Invalid message type for ShortMsgManager!");
        }

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
                throw common::UnknownException("Error while deleting message conversation");
            }

        }

    } catch (const common::PlatformException& err) {
        LoggerE("%s (%s)", (err.name()).c_str(), (err.message()).c_str());
        callback->setError(err.name(), err.message());
    } catch (...) {
        LoggerE("Messages remove failed");
        common::UnknownException e("Messages remove failed");
        callback->setError(e.name(), e.message());
    }

    error = msg_close_msg_handle(&handle);
    if (MSG_SUCCESS != error) {
        LoggerW("Cannot close message handle: %d", error);
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
    } catch (const common::PlatformException& err) {
        LoggerE("Error while calling removeConversations callback: %s (%s)",
                (err.name()).c_str(), (err.message()).c_str());
    } catch (...) {
        LoggerE("Unknown error when calling removeConversations callback.");
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
