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

/**
 * @file: EmailManager.h
 */

#ifndef __TIZEN_EMAIL_MANAGER_H__
#define __TIZEN_EMAIL_MANAGER_H__

#include <glib.h>
#include <mutex>
#include <string>
#include <map>
#include <vector>

#include "email-api-network.h"
#include "email-api-account.h"
#include "email-api-mail.h"
#include "email-api-mailbox.h"

#include "common/callback_user_data.h"
#include "common/GDBus/connection.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"

#include "messaging_util.h"
#include "message_service.h"

#include "DBus/SyncProxy.h"
#include "DBus/LoadBodyProxy.h"
#include "DBus/LoadAttachmentProxy.h"
#include "DBus/MessageProxy.h"
#include "DBus/SendProxy.h"

namespace extension {
namespace messaging {

//class Message;
//class MessageCallbackUserData;
class FindMsgCallbackUserData;
//class SyncFolderCallbackData;
class MessageBodyCallbackData;

class EmailManager {
public:
    static EmailManager& getInstance();
    static common::PlatformResult InitializeEmailService();

    void addDraftMessage(MessageCallbackUserData* callback);
    void removeMessages(MessagesCallbackUserData* callback);
    void updateMessages(MessagesCallbackUserData* callback);
    void findMessages(FindMsgCallbackUserData* callback);
    void findConversations(ConversationCallbackData* callback);
    void findFolders(FoldersCallbackData* callback);
    void removeConversations(ConversationCallbackData* callback);

    common::PlatformResult sendMessage(MessageRecipientsCallbackData* callback);
    void sendStatusCallback(int mail_id, email_noti_on_network_event status,
            int error_code);
    void removeStatusCallback(const std::vector<int> &ids,
            email_noti_on_storage_event status);

    void loadMessageBody(MessageBodyCallbackData* callback);
    common::PlatformResult loadMessageAttachment(MessageAttachmentCallbackData* callback);

    void sync(void* data);
    void syncFolder(SyncFolderCallbackData* callback);
    void stopSync(long op_id);

    void RemoveSyncCallback(long op_id);
    void RemoveCallbacksByQueue(const PostQueue& q);

//    void registerStatusCallback(msg_handle_t msg_handle);

    /**
     * Use freeMessage() to release returned email_mail_data_t object.
     */
    static email_mail_data_t* loadMessage(int msg_id);
    static void freeMessage(email_mail_data_t*);

    long getUniqueOpId();

private:
    EmailManager();
    EmailManager(const EmailManager &);
    void operator=(const EmailManager &);
    virtual ~EmailManager();
    common::PlatformResult addDraftMessagePlatform(int account_id,
        std::shared_ptr<Message> message);
    common::PlatformResult addOutboxMessagePlatform(int account_id,
        std::shared_ptr<Message> message);
    common::PlatformResult addMessagePlatform(int account_id, std::shared_ptr<Message> message,
        email_mailbox_type_e mailbox_type);
    common::PlatformResult UpdateMessagesPlatform(MessagesCallbackUserData* callback);
    common::PlatformResult RemoveMessagesPlatform(
        MessagesCallbackUserData* callback);
    common::PlatformResult FindMessagesPlatform(FindMsgCallbackUserData* callback);
    common::PlatformResult FindConversationsPlatform(ConversationCallbackData* callback);
    common::PlatformResult FindFoldersPlatform(FoldersCallbackData* callback);
    common::PlatformResult RemoveConversationsPlatform(ConversationCallbackData* callback);

    typedef std::map<int, MessageRecipientsCallbackData*> SendReqMap;
    typedef SendReqMap::iterator SendReqMapIterator;
    SendReqMapIterator getSendRequest(int mail_id);
    SendReqMap m_sendRequests;
    struct DeleteReq {
        MessagesCallbackUserData* callback;
        int messagesDeleted;
    };
    typedef std::vector<DeleteReq> DeleteReqVector;
    /**
     * Find first request containing at least one message id
     * @param ids
     * @return
     */
    DeleteReqVector::iterator getDeleteRequest(const std::vector<int> &ids);
    DeleteReqVector m_deleteRequests;

    int m_slot_size;

    DBus::SyncProxyPtr m_proxy_sync;
    DBus::LoadBodyProxyPtr m_proxy_load_body;
    DBus::LoadAttachmentProxyPtr m_proxy_load_attachment;
    DBus::MessageProxyPtr m_proxy_messageStorage;
    DBus::SendProxyPtr m_proxy_send;

    std::mutex m_mutex;
    bool m_is_initialized;
};

} // Messaging
} // DeviceAPI
#endif // __TIZEN_EMAIL_MANAGER_H__
