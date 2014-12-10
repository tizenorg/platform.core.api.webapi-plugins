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

#ifndef __TIZEN_FOLDERS_CALLBACK_DATA_H__
#define __TIZEN_FOLDERS_CALLBACK_DATA_H__

#include <CallbackUserData.h>
#include <memory>
#include <vector>
#include <string>

#include <AbstractFilter.h>

namespace DeviceAPI {
namespace Messaging {

class MessageFolder;

class FoldersCallbackData: public Common::CallbackUserData {
public:
    FoldersCallbackData(JSContextRef globalCtx);
    virtual ~FoldersCallbackData();

    void addFolder(std::shared_ptr<MessageFolder> folder);
    const std::vector<std::shared_ptr<MessageFolder>>& getFolders() const;

    void setFilter(DeviceAPI::Tizen::AbstractFilterPtr filter);
    DeviceAPI::Tizen::AbstractFilterPtr getFilter() const;

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    std::string getErrorName() const;
    std::string getErrorMessage() const;

private:
    std::vector<std::shared_ptr<MessageFolder>> m_folders;
    DeviceAPI::Tizen::AbstractFilterPtr m_filter;
    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;

};

}//Messaging
}//DeviceAPI

#endif /* __TIZEN_FOLDERS_CALLBACK_DATA_H__ */
