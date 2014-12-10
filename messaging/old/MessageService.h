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

#ifndef __TIZEN_MESSAGE_SERVICE_H__
#define __TIZEN_MESSAGE_SERVICE_H__

#include <msg.h>
#include <msg_transport.h>
#include <msg_storage.h>
#include <memory>
#include <ITapiNetwork.h>

#include <CallbackUserData.h>
#include <PlatformException.h>
#include <Security.h>

#include "MessagingUtil.h"
#include "Message.h"
#include "MessageStorage.h"
#include "MessageFolder.h"

#include "DBus/Connection.h"

namespace DeviceAPI {
namespace Messaging {

enum MessageServiceAccountId
{
    UNKNOWN_ACCOUNT_ID = 0,
    SMS_ACCOUNT_ID = 101,
    MMS_ACCOUNT_ID = 102
};

class MessageRecipientsCallbackData : public Common::CallbackUserData {
public:
    MessageRecipientsCallbackData(JSContextRef globalCtx);
    virtual ~MessageRecipientsCallbackData();

    void setMessage(std::shared_ptr<Message> message);
    std::shared_ptr<Message> getMessage() const;

    void setMessageRecipients(const std::vector<std::string>& msgRecipients);
    const std::vector<std::string>& getMessageRecipients() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void setAccountId(int account_id);
    int getAccountId() const;

    void setSimIndex(TelNetworkDefaultDataSubs_t sim_index);
    TelNetworkDefaultDataSubs_t getSimIndex() const;
    void setDefaultSimIndex(TelNetworkDefaultDataSubs_t sim_index);
    TelNetworkDefaultDataSubs_t getDefaultSimIndex() const;
    bool isSetSimIndex() const;

private:
    std::shared_ptr<Message> m_message;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    std::vector<std::string> m_msg_recipients;
    int m_account_id;
    TelNetworkDefaultDataSubs_t m_sim_index;
    TelNetworkDefaultDataSubs_t m_default_sim_index;
};


class BaseMessageServiceCallbackData : public Common::CallbackUserData {
public:
    BaseMessageServiceCallbackData(JSContextRef globalCtx);
    virtual ~BaseMessageServiceCallbackData();

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

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

protected:
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;

    int m_op_handle;
};

class MessageBodyCallbackData : public BaseMessageServiceCallbackData {
public:
    MessageBodyCallbackData(JSContextRef globalCtx);
    virtual ~MessageBodyCallbackData();

    void setMessage(std::shared_ptr<Message> message);
    std::shared_ptr<Message> getMessage() const;

private:
    std::shared_ptr<Message> m_message;
};

class MessageAttachmentCallbackData : public BaseMessageServiceCallbackData {
public:
    MessageAttachmentCallbackData(JSContextRef globalCtx);
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

private:
    std::shared_ptr<MessageAttachment> m_message_attachment;
    int m_nth;
};

class SyncCallbackData : public BaseMessageServiceCallbackData {
public:
    SyncCallbackData(JSContextRef globalCtx);
    virtual ~SyncCallbackData();

    void setLimit(const unsigned long limit);
    bool isLimit() const;
    unsigned long getLimit() const;

    void setOpId(long op_id);
    long getOpId();
    void setAccountId(int account_id);
    int getAccountId() const;

protected:
    bool m_is_limit;
    unsigned long m_limit;

    long m_op_id;
    int m_account_id;
};

class SyncFolderCallbackData : public SyncCallbackData {
public:
    SyncFolderCallbackData(JSContextRef globalCtx);
    virtual ~SyncFolderCallbackData();

    void setMessageFolder(std::shared_ptr<MessageFolder> message_folder);
    std::shared_ptr<MessageFolder> getMessageFolder() const;

private:
    std::shared_ptr<MessageFolder> m_message_folder;
};


class MessageService : public Common::SecurityAccessor
{
public:
    virtual ~MessageService();

    virtual int getMsgServiceId() const;
    virtual std::string getMsgServiceIdStr() const;
    virtual MessageType getMsgServiceType() const;
    virtual std::string getMsgServiceName() const;
    virtual std::shared_ptr<MessageStorage> getMsgStorage() const;

    virtual void sendMessage(MessageRecipientsCallbackData *callback);
    virtual void loadMessageBody(MessageBodyCallbackData *callback);
    virtual void loadMessageAttachment(MessageAttachmentCallbackData *callback);
    virtual long sync(SyncCallbackData *callback);

    /**
     * @param callback - owned by this method unless exception is thrown
     * @return opId - "long Identifier which can be used to stop this service operation"
     *                (form JS documentation)
     *
     */
    virtual long syncFolder(SyncFolderCallbackData *callback);

    virtual void stopSync(long op_id);

protected:
    /**
     * We have child classes MessageServiceEmail and MessageServiceShortMsg which
     * should provide specialized implementation.
     */
    MessageService(int id,
            MessageType msgType,
            std::string name);

    int m_id;
    MessageType m_msg_type;
    std::string m_name;
    std::shared_ptr<MessageStorage> m_storage;
};

} // Messaging
} // DeviceAPI
#endif // __TIZEN_MESSAGE_SERVICE_H__
