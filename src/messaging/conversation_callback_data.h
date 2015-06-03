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
