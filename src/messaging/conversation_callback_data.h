// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_CONVERSATION_CALLBACK_DATA_H_
#define MESSAGING_CONVERSATION_CALLBACK_DATA_H_

#include "common/callback_user_data.h"
#include <memory>
#include <string>
#include <vector>
#include "MsgCommon/AttributeFilter.h"
#include "MsgCommon/SortMode.h"
#include "messaging_util.h"

using namespace extension::tizen;

namespace extension {
namespace messaging {

class Message;
class MessageConversation;

class ConversationCallbackData: public common::CallbackUserData {
public:
    ConversationCallbackData(PostQueue& queue);
    ConversationCallbackData(long cid, PostQueue& queue, bool keep = false);
    virtual ~ConversationCallbackData();

    void setFilter(AbstractFilterPtr filter);
    void setSortMode(SortModePtr sortMode);
    void setLimit(long limit);
    void setOffset(long offset);
    void addConversation(std::shared_ptr<MessageConversation> msg);

    std::vector<std::shared_ptr<MessageConversation>> getConversations() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    void SetError(const common::PlatformResult& error);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void setAccountId(int account_id);
    int getAccountId() const;

    void setMessageServiceType(MessageType m_msg_type);
    MessageType getMessageServiceType() const;
    AbstractFilterPtr getFilter() const;
    SortModePtr getSortMode() const;
    long getLimit() const;
    long getOffset() const;

    PostQueue& getQueue() { return queue_;};
private:
    AbstractFilterPtr m_filter;
    SortModePtr m_sort;
    long m_limit;
    long m_offset;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    std::vector<std::shared_ptr<MessageConversation>> m_conversations;
    int m_account_id;
    MessageType m_service_type;
    PostQueue& queue_;
};

}//messaging
}//extension

#endif // MESSAGING_CONVERSATION_CALLBACK_DATA_H_
