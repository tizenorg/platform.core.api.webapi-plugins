
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGE_SERVICE_H_
#define MESSAGING_MESSAGE_SERVICE_H_

#include <memory>
#include <string>

#include "common/picojson.h"
#include "common/callback_user_data.h"

#include "messaging_util.h"
#include "message_storage.h"

namespace extension {
namespace messaging {

enum MessageServiceAccountId
{
    UNKNOWN_ACCOUNT_ID = 0,
    SMS_ACCOUNT_ID = 101,
    MMS_ACCOUNT_ID = 102
};

class BaseMessageServiceCallbackData : public common::CallbackUserData {
public:
    BaseMessageServiceCallbackData();
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
    void setCallbackId(const double callback_id);
    double getCallbackId() const;

protected:
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;

    int m_op_handle;
    double m_callback_id;
};

class SyncCallbackData : public BaseMessageServiceCallbackData {
public:
    SyncCallbackData();
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

class MessageService
{
public:
    virtual ~MessageService();

    virtual int getMsgServiceId() const;
    virtual std::string getMsgServiceIdStr() const;
    virtual MessageType getMsgServiceType() const;
    virtual std::string getMsgServiceName() const;

    virtual MessageStoragePtr getMsgStorage() const;

    virtual void sendMessage();
    virtual void loadMessageBody();
    virtual void loadMessageAttachment();
    virtual long sync(SyncCallbackData *callback);
    virtual long syncFolder();
    virtual void stopSync(long op_id);

    picojson::object toPicoJS() const;

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
    MessageStoragePtr m_storage;
};

} // messaging
} // extension
#endif // MESSAGING_MESSAGE_SERVICE_EMAIL_H_
