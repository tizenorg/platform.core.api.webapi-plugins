// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//#include <JSWebAPIErrorFactory.h>
//#include <PlatformException.h>
//#include <JSUtil.h>
//#include <GlobalContextManager.h>

//#include "FoldersChangeCallback.h"
//#include "JSMessageFolder.h"
//#include "MessagingUtil.h"
//#include <Logger.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "messaging_instance.h"
#include "messaging_util.h"

#include "folders_change_callback.h"

namespace extension {
namespace messaging {

const char* FOLDERSADDED = "foldersadded";
const char* FOLDERSUPDATED = "foldersupdated";
const char* FOLDERSREMOVED = "foldersremoved";

FoldersChangeCallback::FoldersChangeCallback(
        long cid,
        int service_id,
        MessageType service_type):
    m_callback_data(cid, true),
    m_id(service_id),
    m_msg_type(service_type),
    m_is_act(true)
{
    LoggerD("Entered");
}

FoldersChangeCallback::~FoldersChangeCallback()
{
    LoggerD("Entered");
}

FolderPtrVector FoldersChangeCallback::filterFolders(
        tizen::AbstractFilterPtr filter,
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

            LoggerD("[%d] folder id:%s", i, folder->getId().c_str());
            LoggerD("[%d] folder name:%s", i, folder->getName().c_str());
            LoggerD("[%d] matched filter: %s", i, matched ? "YES" : "NO");
        }

        return filtered_folders;
    }
    else {
        return source_folders;
    }
}

void FoldersChangeCallback::added(const FolderPtrVector& folders)
{
    LoggerD("Entered folders.size()=%d", folders.size());
    if (!m_is_act) {
        return;
    }

    FolderPtrVector filtered = filterFolders(m_filter, folders);

    picojson::array array;
    auto each = [&array] (std::shared_ptr<MessageFolder> f)->void {
        array.push_back(MessagingUtil::folderToJson(f));
    };
    for_each(filtered.begin(), filtered.end(), each);

    LoggerD("Calling:%s with:%d added folders", FOLDERSADDED,
            filtered.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(FOLDERSADDED);
    obj[JSON_DATA] = picojson::value(array);

    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      PostQueue::getInstance().addAndResolve(obj.at(
          JSON_CALLBACK_ID).get<double>(), PostPriority::MEDIUM, json->serialize());
    } else {
      LoggerE("Callback id is missing");
    }
}

void FoldersChangeCallback::updated(const FolderPtrVector& folders)
{
    LoggerD("Entered folders.size()=%d", folders.size());
    if (!m_is_act) {
        return;
    }

    FolderPtrVector filtered = filterFolders(m_filter, folders);

    picojson::array array;
    auto each = [&array] (std::shared_ptr<MessageFolder> f)->void {
        array.push_back(MessagingUtil::folderToJson(f));
    };
    for_each(filtered.begin(), filtered.end(), each);

    LoggerD("Calling:%s with:%d updated folders", FOLDERSUPDATED,
            filtered.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(FOLDERSUPDATED);
    obj[JSON_DATA] = picojson::value(array);

    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      PostQueue::getInstance().addAndResolve(obj.at(
          JSON_CALLBACK_ID).get<double>(), PostPriority::LOW, json->serialize());
    } else {
      LoggerE("Callback id is missing");
    }
}

void FoldersChangeCallback::removed(const FolderPtrVector& folders)
{
    LoggerD("Entered folders.size()=%d", folders.size());
    if (!m_is_act) {
        return;
    }

    FolderPtrVector filtered = filterFolders(m_filter, folders);

    picojson::array array;
    auto each = [&array] (std::shared_ptr<MessageFolder> f)->void {
        array.push_back(MessagingUtil::folderToJson(f));
    };
    for_each(filtered.begin(), filtered.end(), each);

    LoggerD("Calling:%s with:%d removed folders", FOLDERSREMOVED,
            filtered.size());

    auto json = m_callback_data.getJson();
    picojson::object& obj = json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(FOLDERSREMOVED);
    obj[JSON_DATA] = picojson::value(array);

    if (json->contains(JSON_CALLBACK_ID) && obj.at(JSON_CALLBACK_ID).is<double>()) {
      PostQueue::getInstance().addAndResolve(obj.at(
          JSON_CALLBACK_ID).get<double>(), PostPriority::LAST, json->serialize());
    } else {
      LoggerE("Callback id is missing");
    }
}

void FoldersChangeCallback::setFilter(tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

tizen::AbstractFilterPtr FoldersChangeCallback::getFilter() const
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

} //namespace messaging
} //namespace extension

