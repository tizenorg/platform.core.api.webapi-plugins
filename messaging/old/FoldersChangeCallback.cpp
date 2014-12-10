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

#include <JSWebAPIErrorFactory.h>
#include <PlatformException.h>
#include <JSUtil.h>
#include <GlobalContextManager.h>

#include "FoldersChangeCallback.h"
#include "JSMessageFolder.h"
#include "MessagingUtil.h"
#include <Logger.h>

using namespace DeviceAPI::Common;
using namespace DeviceAPI::Tizen;

namespace DeviceAPI {
namespace Messaging {

const char* FOLDERSADDED = "foldersadded";
const char* FOLDERSUPDATED = "foldersupdated";
const char* FOLDERSREMOVED = "foldersremoved";

FoldersChangeCallback::FoldersChangeCallback(JSContextRef global_ctx,
        JSObjectRef on_added_obj,
        JSObjectRef on_updated_obj,
        JSObjectRef on_removed_obj,
        int service_id,
        MessageType service_type):
    m_callback_data(global_ctx),
    m_id(service_id),
    m_msg_type(service_type),
    m_is_act(true)
{
    LOGD("Entered");

    m_callback_data.setCallback(FOLDERSADDED, on_added_obj);
    m_callback_data.setCallback(FOLDERSUPDATED, on_updated_obj);
    m_callback_data.setCallback(FOLDERSREMOVED, on_removed_obj);
}

FoldersChangeCallback::~FoldersChangeCallback()
{
    LOGD("Entered");
}

FolderPtrVector FoldersChangeCallback::filterFolders(
        AbstractFilterPtr filter,
        const FolderPtrVector& source_folders)
{
    if (filter) {
        FolderPtrVector filtered_folders;
        FolderPtrVector::const_iterator it = source_folders.begin();
        FolderPtrVector::const_iterator end_it = source_folders.end();

        for(int i = 0; it != end_it; ++i, ++it) {
            const FolderPtr& folder = *it;
            const bool matched = filter->isMatching(folder.get());
            if (matched) {
                filtered_folders.push_back(folder);
            }

            LOGD("[%d] folder id:%s", i, folder->getId().c_str());
            LOGD("[%d] folder name:%s", i, folder->getName().c_str());
            LOGD("[%d] matched filter: %s", i, matched ? "YES" : "NO");
        }

        return filtered_folders;
    }
    else {
        return source_folders;
    }
}

void FoldersChangeCallback::added(const FolderPtrVector& folders)
{
    LOGD("Entered folders.size()=%d", folders.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    FolderPtrVector filtered = filterFolders(m_filter, folders);
    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<FolderPtr,JSMessageFolder>(
            ctx, filtered);

    LOGD("Calling:%s with:%d added folders", FOLDERSADDED,
            filtered.size());

    m_callback_data.invokeCallback(FOLDERSADDED, js_obj);
}

void FoldersChangeCallback::updated(const FolderPtrVector& folders)
{
    LOGD("Entered folders.size()=%d", folders.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    FolderPtrVector filtered = filterFolders(m_filter, folders);
    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<FolderPtr,JSMessageFolder>(
            ctx, filtered);

    LOGD("Calling:%s with:%d updated folders", FOLDERSUPDATED,
            filtered.size());

    m_callback_data.invokeCallback(FOLDERSUPDATED, js_obj);
}

void FoldersChangeCallback::removed(const FolderPtrVector& folders)
{
    LOGD("Entered folders.size()=%d", folders.size());
    if (!m_is_act) {
        return;
    }

    JSContextRef ctx = m_callback_data.getContext();
    CHECK_CURRENT_CONTEXT_ALIVE(ctx)
    FolderPtrVector filtered = filterFolders(m_filter, folders);
    JSObjectRef js_obj = MessagingUtil::vectorToJSObjectArray<FolderPtr,JSMessageFolder>(
            ctx, filtered);

    LOGD("Calling:%s with:%d removed folders", FOLDERSREMOVED,
            filtered.size());

    m_callback_data.invokeCallback(FOLDERSREMOVED, js_obj);
}

void FoldersChangeCallback::setFilter(AbstractFilterPtr filter)
{
    m_filter = filter;
}

AbstractFilterPtr FoldersChangeCallback::getFilter() const
{
    return m_filter;
}

int FoldersChangeCallback::getServiceId() const
{
    return m_id;
}

MessageType FoldersChangeCallback::getServiceType() const
{
    return m_msg_type;
}

void FoldersChangeCallback::setActive(bool act) {
    m_is_act = act;
}

bool FoldersChangeCallback::isActive() {
    return m_is_act;
}

void FoldersChangeCallback::setItems(FolderPtrVector& items)
{
    m_items = items;
}
FolderPtrVector FoldersChangeCallback::getItems()
{
    return m_items;
}

JSContextRef FoldersChangeCallback::getContext() const
{
    return m_callback_data.getContext();
}

} // Messaging
} // DeviceAPI


