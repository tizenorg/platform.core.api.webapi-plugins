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

#ifndef __TIZEN_FOLDERS_CALLBACK_DATA_H__
#define __TIZEN_FOLDERS_CALLBACK_DATA_H__

#include "common/callback_user_data.h"
#include "common/platform_result.h"

#include <memory>
#include <vector>
#include <string>

#include "MsgCommon/AbstractFilter.h"

#include "message_folder.h"
#include "messaging_util.h"

namespace extension {
namespace messaging {

class MessageFolder;

class FoldersCallbackData: public common::CallbackUserData {
public:
    FoldersCallbackData(PostQueue& queue);
    FoldersCallbackData(long cid, PostQueue& queue, bool keep = false);
    virtual ~FoldersCallbackData();

    void addFolder(std::shared_ptr<MessageFolder> folder);
    const std::vector<std::shared_ptr<MessageFolder>>& getFolders() const;

    void setFilter(tizen::AbstractFilterPtr filter);
    tizen::AbstractFilterPtr getFilter() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    void SetError(const common::PlatformResult& error);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

    PostQueue& getQueue() { return queue_;};
private:
    std::vector<std::shared_ptr<MessageFolder>> m_folders;
    tizen::AbstractFilterPtr m_filter;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
    PostQueue& queue_;

};

}//messaging
}//extension

#endif /* __TIZEN_FOLDERS_CALLBACK_DATA_H__ */
