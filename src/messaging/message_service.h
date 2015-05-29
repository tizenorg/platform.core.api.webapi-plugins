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
 
#ifndef MESSAGING_MESSAGE_SERVICE_H_
#define MESSAGING_MESSAGE_SERVICE_H_

#include <ITapiNetwork.h>
#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/callback_user_data.h"
#include "common/platform_result.h"

#include "messaging_util.h"
#include "message_storage.h"
#include "message.h"

namespace extension {
namespace messaging {

enum MessageServiceAccountId
{
    UNKNOWN_ACCOUNT_ID = 0,
    SMS_ACCOUNT_ID = 101,
    MMS_ACCOUNT_ID = 102
};

class MessageRecipientsCallbackData : public common::CallbackUserData {
public:
    MessageRecipientsCallbackData(PostQueue& queue);
    virtual ~MessageRecipientsCallbackData();

    void setMessage(std::shared_ptr<Message> message);
    std::shared_ptr<Message> getMessage() const;

    void setMessageRecipients(const std::vector<std::string>& msgRecipients);
    const std::vector<std::string>& getMessageRecipients() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    void setError(const common::PlatformResult& error);
    bool isError() const;

    void setAccountId(int account_id);
    int getAccountId() const;

    bool setSimIndex(int sim_index);
    TelNetworkDefaultDataSubs_t getSimIndex() const;
    void setDefaultSimIndex(TelNetworkDefaultDataSubs_t sim_index);
    TelNetworkDefaultDataSubs_t getDefaultSimIndex() const;
    bool isSetSimIndex() const;

    PostQueue& getQueue() { return queue_;};

private:
    std::shared_ptr<Message> m_message;
    bool m_is_error;
    std::vector<std::string> m_msg_recipients;
    int m_account_id;
    TelNetworkDefaultDataSubs_t m_sim_index;
    TelNetworkDefaultDataSubs_t m_default_sim_index;
    PostQueue& queue_;
};

class BaseMessageServiceCallbackData : public common::CallbackUserData {
public:
    BaseMessageServiceCallbackData();
    virtual ~BaseMessageServiceCallbackData();

    void setError(const std::string& err_name,
            const std::string& err_message);
    void setError(const common::PlatformResult& error);
    bool isError() const;

    /**
     * This handle is returned from various native API functions:
     *   int email_sync_header(..., int *handle);
     *   int email_download_body(..., int *handle);
     *   int email_download_attachment(..., int *handle);
     *
     * It is used to stop and identify request.
     */
    void setOperationHandle(const int op_handle);
    int getOperationHandle() const;
    void setCallbackId(const double callback_id);
    double getCallbackId() const;

protected:
    bool m_is_error;

    int m_op_handle;
    double m_callback_id;
};

class MessageBodyCallbackData : public BaseMessageServiceCallbackData {
public:
    MessageBodyCallbackData(PostQueue& queue);
    virtual ~MessageBodyCallbackData();

    void setMessage(std::shared_ptr<Message> message);
    std::shared_ptr<Message> getMessage() const;

    PostQueue& getQueue() { return queue_;};
private:
    std::shared_ptr<Message> m_message;
    PostQueue& queue_;
};

class MessageAttachmentCallbackData : public BaseMessageServiceCallbackData {
public:
    MessageAttachmentCallbackData(PostQueue& queue);
    virtual ~MessageAttachmentCallbackData();

    void setMessageAttachment(std::shared_ptr<MessageAttachment> messageAttachment);
    std::shared_ptr<MessageAttachment> getMessageAttachment() const;

    /**
     * nth is used in native api call:
     * int email_download_attachment(int mail_id, int nth, int *handle);
     *
     * nth is equal to attachment index+1 (starts from 1 not 0) see
     * email-api-network.h for details.
     * */
    void setNth(const int nth);
    int getNth() const;

    PostQueue& getQueue() { return queue_;};
private:
    std::shared_ptr<MessageAttachment> m_message_attachment;
    int m_nth;
    PostQueue& queue_;
};

class SyncCallbackData : public BaseMessageServiceCallbackData {
public:
    SyncCallbackData(PostQueue& queue);
    virtual ~SyncCallbackData();

    void setLimit(const unsigned long limit);
    bool isLimit() const;
    unsigned long getLimit() const;

    void setOpId(long op_id);
    long getOpId();
    void setAccountId(int account_id);
    int getAccountId() const;

    PostQueue& getQueue() { return queue_;};
protected:
    bool m_is_limit;
    unsigned long m_limit;

    long m_op_id;
    int m_account_id;
    PostQueue& queue_;
};

class SyncFolderCallbackData : public SyncCallbackData {
public:
    SyncFolderCallbackData(PostQueue& queue);
    virtual ~SyncFolderCallbackData();

    void setMessageFolder(std::shared_ptr<MessageFolder> message_folder);
    std::shared_ptr<MessageFolder> getMessageFolder() const;

private:
    std::shared_ptr<MessageFolder> m_message_folder;
};

class MessageService
{
public:
    virtual ~MessageService();

    virtual int getMsgServiceId() const;
    virtual std::string getMsgServiceIdStr() const;
    virtual MessageType getMsgServiceType() const;
    std::string getMsgServiceTypeString() const;
    virtual std::string getMsgServiceName() const;

    virtual MessageStoragePtr getMsgStorage() const;

    virtual common::PlatformResult sendMessage(MessageRecipientsCallbackData *callback);
    virtual common::PlatformResult loadMessageBody(MessageBodyCallbackData* callback);
    virtual common::PlatformResult loadMessageAttachment(MessageAttachmentCallbackData* callback);
    virtual common::PlatformResult sync(SyncCallbackData *callback, long* operation_id);
    virtual common::PlatformResult syncFolder(SyncFolderCallbackData *callback, long* operation_id);
    virtual common::PlatformResult stopSync(long op_id);

    picojson::object toPicoJS() const;

protected:
    /**
     * We have child classes MessageServiceEmail and MessageServiceShortMsg which
     * should provide specialized implementation.
     */
    MessageService(int id,
            MessageType msgType,
            const std::string& name);

    int m_id;
    MessageType m_msg_type;
    std::string m_name;
    MessageStoragePtr m_storage;
};

} // messaging
} // extension
#endif // MESSAGING_MESSAGE_SERVICE_EMAIL_H_
