// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef __MESSAGING_FOLDERS_CHANGE_CALLBACK_H__
#define __MESSAGING_FOLDERS_CHANGE_CALLBACK_H__


#include "MsgCommon/AbstractFilter.h"

#include "messaging_util.h"
#include "message_folder.h"

#include "folders_callback_data.h"

//#include <MultiCallbackUserData.h>

namespace extension {
namespace messaging {

extern const char* FOLDERSADDED;
extern const char* FOLDERSUPDATED;
extern const char* FOLDERSREMOVED;

class FoldersChangeCallback {
public:
    FoldersChangeCallback(
            long cid,
            int service_id,
            MessageType service_type,
            PostQueue& queue);
    virtual ~FoldersChangeCallback();

    void added(const FolderPtrVector& folders);
    void updated(const FolderPtrVector& folders);
    void removed(const FolderPtrVector& folders);

    void setFilter(tizen::AbstractFilterPtr filter);
    tizen::AbstractFilterPtr getFilter() const;

    int getServiceId() const;
    MessageType getServiceType() const;

    void setActive(bool act);
    bool isActive();

    void setItems(FolderPtrVector& items);
    FolderPtrVector getItems();

private:
    static FolderPtrVector filterFolders(tizen::AbstractFilterPtr filter,
            const FolderPtrVector& source_folders);

    FoldersCallbackData m_callback_data;
    tizen::AbstractFilterPtr m_filter;
    int m_id;
    MessageType m_msg_type;
    bool m_is_act;
    FolderPtrVector m_items;
};

} //messaging
} //extension

#endif // __MESSAGING_FOLDERS_CHANGE_CALLBACK_H__
