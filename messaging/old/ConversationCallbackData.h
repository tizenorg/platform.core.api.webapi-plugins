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

#ifndef __TIZEN_CONVERSATION_CALLBACK_DATA_H__
#define __TIZEN_CONVERSATION_CALLBACK_DATA_H__

#include <CallbackUserData.h>
#include <memory>
#include <string>
#include <vector>
#include <AttributeFilter.h>
#include <SortMode.h>
#include "MessagingUtil.h"

namespace DeviceAPI {
namespace Messaging {

class Message;
class MessageConversation;

class ConversationCallbackData: public Common::CallbackUserData {
public:
    ConversationCallbackData(JSContextRef globalCtx);
    virtual ~ConversationCallbackData();

    void setFilter(Tizen::AbstractFilterPtr filter);
    void setSortMode(Tizen::SortModePtr sortMode);
    void setLimit(long limit);
    void setOffset(long offset);
    void addConversation(std::shared_ptr<MessageConversation> msg);
    void addConversations(JSContextRef context,
            const std::vector<JSValueRef>& jsobject_conservations);

    std::vector<std::shared_ptr<MessageConversation>> getConversations() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    void setAccountId(int account_id);
    int getAccountId() const;

    void setMessageServiceType(MessageType m_msg_type);
    MessageType getMessageServiceType() const;
    Tizen::AbstractFilterPtr getFilter() const;
    Tizen::SortModePtr getSortMode() const;
    long getLimit() const;
    long getOffset() const;

private:
    Tizen::AbstractFilterPtr m_filter;
    Tizen::SortModePtr m_sort;
    long m_limit;
    long m_offset;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    std::vector<std::shared_ptr<MessageConversation>> m_conversations;
    int m_account_id;
    MessageType m_service_type;
};

}//Messaging
}//DeviceAPI

#endif // __TIZEN_CONVERSATION_CALLBACK_DATA_H__
