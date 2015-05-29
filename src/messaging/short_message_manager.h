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
 
#ifndef __MESSAGING_SEND_SHORT_MSG_MANAGER_H__
#define __MESSAGING_SEND_SHORT_MSG_MANAGER_H__

#include <glib.h>
#include <mutex>
#include <map>

#include <msg_storage_types.h>
#include <msg_types.h>

//#include <CallbackUserData.h>

#include "common/platform_result.h"
#include "change_listener_container.h"
#include "messaging_util.h"
#include "message_service.h"
#include "message_sms.h"
#include "message_conversation.h"

namespace extension {
namespace messaging {

class FindMsgCallbackUserData;

class ShortMsgManager {
public:
    static ShortMsgManager& getInstance();

    common::PlatformResult sendMessage(MessageRecipientsCallbackData* callback);
    void sendStatusCallback(msg_struct_t sent_status);

    void addDraftMessage(MessageCallbackUserData* callback);
    void findMessages(FindMsgCallbackUserData* callback);
    void findConversations(ConversationCallbackData* callback);
    void removeConversations(ConversationCallbackData* callback);

    void registerStatusCallback(msg_handle_t msg_handle);

    void removeMessages(MessagesCallbackUserData* callback);
    void updateMessages(MessagesCallbackUserData* callback);
    common::PlatformResult getMessage(int msg_id, msg_struct_t* out_msg);
private:
    ShortMsgManager();
    ShortMsgManager(const ShortMsgManager &);
    void operator=(const ShortMsgManager &);
    virtual ~ShortMsgManager();

    /**
     * Listener for msg storage changes. Calls callbacks from ChangeListenerContainer.
     * @param handle
     * @param storageChangeType
     * @param pMsgIdList
     * @param data
     */
    static void storage_change_cb(msg_handle_t handle,
        msg_storage_change_type_t storageChangeType,
        msg_id_list_s *pMsgIdList,
        void* data);

    common::PlatformResult addDraftMessagePlatform(std::shared_ptr<Message> message);
    common::PlatformResult SendMessagePlatform(MessageRecipientsCallbackData* callback);
    /**
     * Returns unique list of conversations for given vector of messages.
     * storageChangeType is needed to filter conversations returned:
     * - for MSG_STORAGE_CHANGE_UPDATE all conversations are fetched
     * - for MSG_STORAGE_CHANGE_INSERT only conversations with 1 message are returned
     * - for MSG_STORAGE_CHANGE_DELETE  only conversations with 1 message are returned
     * @param messages
     * @param storageChangeType
     * @return
     */
    static common::PlatformResult getConversationsForMessages(
            MessagePtrVector messages,
            msg_storage_change_type_t storageChangeType, ConversationPtrVector* result);
    static common::PlatformResult callProperEventMessages(EventMessages* event,
            msg_storage_change_type_t storageChangeType);
    typedef std::map<msg_request_id_t, MessageRecipientsCallbackData*> SendReqMap;
    SendReqMap m_sendRequests;
    msg_handle_t m_msg_handle;

    /**
     * Map MessageId - Message object of recently removed SMS messages
     */
    std::map<int, MessagePtr> m_sms_removed_messages;

    /**
     * Map MessageId - Message object of recently removed MMS messages
     */
    std::map<int, MessagePtr> m_mms_removed_messages;



    /**
     * Map MessageId - ConversationId for SMS messages (only from removed conversation)
     */
    std::map<int, int> m_sms_removed_msg_id_conv_id_map;

    /**
     * Map ConversationId - ConversationPtr object (only removed) for SMS
     */
    std::map<int, ConversationPtr> m_sms_removed_conv_id_object_map;

    /**
     * Map MessageId - ConversationId for MMS messages (only from removed conversation)
     */
    std::map<int, int> m_mms_removed_msg_id_conv_id_map;

    /**
     * Map ConversationId - ConversationPtr object (only removed) for MMS
     */
    std::map<int, ConversationPtr> m_mms_removed_conv_id_object_map;

    std::mutex m_mutex;
};

} // messaging
} // extension
#endif // __MESSAGING_SEND_SHORT_MSG_MANAGER_H__
