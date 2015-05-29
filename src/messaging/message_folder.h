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
