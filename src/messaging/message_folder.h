// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __TIZEN_MESSAGING_MESSAGE_FOLDER_H__
#define __TIZEN_MESSAGING_MESSAGE_FOLDER_H__

#include <string>
#include <memory>
#include <vector>

#include <email-types.h>
#include "MsgCommon/AbstractFilter.h"

namespace extension {
namespace messaging {

enum MessageFolderType {
    MESSAGE_FOLDER_TYPE_INBOX,
    MESSAGE_FOLDER_TYPE_OUTBOX,
    MESSAGE_FOLDER_TYPE_DRAFTS,
    MESSAGE_FOLDER_TYPE_SENTBOX,
    MESSAGE_FOLDER_TYPE_NOTSTANDARD
};

class MessageFolder;

struct MessageFolderHolder {
    std::shared_ptr<MessageFolder> ptr;
};

typedef std::shared_ptr<MessageFolder> FolderPtr;

typedef std::vector<FolderPtr> FolderPtrVector;

class MessageFolder : public tizen::FilterableObject{

public:
    MessageFolder(
            std::string id,
            std::string parent_id,
            std::string service_id,
            std::string content_type,
            std::string name,
            std::string path,
            MessageFolderType type,
            bool synchronizable);
    MessageFolder(email_mailbox_t mailbox);

    std::string getId() const;
    std::string getParentId() const;
    bool isParentIdSet() const;
    void setParentId(const std::string& parentId);
    std::string getServiceId() const;
    std::string getContentType() const;
    std::string getName() const;
    void setName(const std::string &value);
    std::string getPath() const;
    MessageFolderType getType() const;
    bool getSynchronizable() const;
    void setSynchronizable(const bool &value);

    // tizen::FilterableObject
    virtual bool isMatchingAttribute(const std::string& attribute_name,
            const tizen::FilterMatchFlag match_flag,
            tizen::AnyPtr match_value) const;

    virtual bool isMatchingAttributeRange(const std::string& attribute_name,
            tizen::AnyPtr initial_value,
            tizen::AnyPtr end_value) const;
private:
    MessageFolderType convertPlatformFolderType(
            email_mailbox_type_e folderType);

    std::string m_id;
    std::string m_parent_id;
    bool m_parent_id_set;
    std::string m_service_id;
    std::string m_content_type;
    std::string m_name;
    std::string m_path;
    MessageFolderType m_type;
    bool m_synchronizable;
};

}    //messaging
}    //extension

#endif // __TIZEN_MESSAGING_MESSAGE_FOLDER_H__
