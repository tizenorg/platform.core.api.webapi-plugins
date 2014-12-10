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

#ifndef __TIZEN_FOLDERS_CHANGE_CALLBACK_H__
#define __TIZEN_FOLDERS_CHANGE_CALLBACK_H__

#include <JavaScriptCore/JavaScript.h>

#include <MultiCallbackUserData.h>

#include <AbstractFilter.h>

#include "MessageFolder.h"
#include "MessagingUtil.h"

namespace DeviceAPI {
namespace Messaging {

extern const char* FOLDERSADDED;
extern const char* FOLDERSUPDATED;
extern const char* FOLDERSREMOVED;

class FoldersChangeCallback {
public:
    FoldersChangeCallback(JSContextRef global_ctx,
            JSObjectRef on_added_obj,
            JSObjectRef on_updated_obj,
            JSObjectRef on_removed_obj,
            int service_id,
            MessageType service_type);
    virtual ~FoldersChangeCallback();

    void added(const FolderPtrVector& folders);
    void updated(const FolderPtrVector& folders);
    void removed(const FolderPtrVector& folders);

    void setFilter(Tizen::AbstractFilterPtr filter);
    Tizen::AbstractFilterPtr getFilter() const;

    int getServiceId() const;
    MessageType getServiceType() const;

    void setActive(bool act);
    bool isActive();

    void setItems(FolderPtrVector& items);
    FolderPtrVector getItems();
    JSContextRef getContext() const;
private:
    static FolderPtrVector filterFolders(Tizen::AbstractFilterPtr filter,
            const FolderPtrVector& source_folders);

    Common::MultiCallbackUserData m_callback_data;
    Tizen::AbstractFilterPtr m_filter;
    int m_id;
    MessageType m_msg_type;
    bool m_is_act;
    FolderPtrVector m_items;
};

} // Messaging
} // DeviceAPI

#endif // __TIZEN_FOLDERS_CHANGE_CALLBACK_H__
