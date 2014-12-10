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

/**
 * @file: FindMsgCallbackUserData.h
 */

#ifndef __TIZEN_FIND_MSG_CALLBACK_USER_DATA_H
#define __TIZEN_FIND_MSG_CALLBACK_USER_DATA_H

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

class FindMsgCallbackUserData: public Common::CallbackUserData {
public:
    FindMsgCallbackUserData(JSContextRef globalCtx);
    virtual ~FindMsgCallbackUserData();

    void setFilter(Tizen::AbstractFilterPtr filter);
    void setSortMode(Tizen::SortModePtr sortMode);
    void setLimit(long limit);
    void setOffset(long offset);
    void addMessage(std::shared_ptr<Message> msg);
    std::vector<std::shared_ptr<Message>> getMessages() const;

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
    //@TODO replace dpl shared_ptr when JSAttributeFilter is changed
    Tizen::AbstractFilterPtr m_filter;
    Tizen::SortModePtr m_sort;
    long m_limit;
    long m_offset;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    std::vector<std::shared_ptr<Message>> m_messages;
    int m_account_id;
    MessageType m_service_type;
};

}//Messaging
}//DeviceAPI

#endif /* __TIZEN_FIND_MSG_CALLBACK_USER_DATA_H */
