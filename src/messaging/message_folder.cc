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

#include "messaging_util.h"
#include "message_folder.h"

namespace extension {
namespace messaging {

using namespace tizen;

MessageFolder::MessageFolder(
        std::string id,
        std::string parent_id,
        std::string service_id,
        std::string content_type,
        std::string name,
        std::string path,
        MessageFolderType type,
        bool synchronizable):
    m_id(id),
    m_parent_id(parent_id),
    m_parent_id_set(true),
    m_service_id(service_id),
    m_content_type(content_type),
    m_name(name),
    m_path(path),
    m_type(type),
    m_synchronizable(synchronizable)
{
    LoggerD("Entered");
}

MessageFolder::MessageFolder(email_mailbox_t mailbox)
{
    LoggerD("Entered");
    m_id = std::to_string(mailbox.mailbox_id);
    m_parent_id_set = false;
    m_service_id = std::to_string(mailbox.account_id);
    m_content_type = MessagingUtil::messageTypeToString(EMAIL);
    m_name = mailbox.alias;
    m_path = mailbox.mailbox_name;
    m_type = convertPlatformFolderType(mailbox.mailbox_type);
    if (0 == mailbox.local) {
        m_synchronizable = true;
    }
    else {
        m_synchronizable = false;
    }
}

std::string MessageFolder::getId() const
{
    return m_id;
}

std::string MessageFolder::getParentId() const
{
    return m_parent_id;
}

bool MessageFolder::isParentIdSet() const
{
    return m_parent_id_set;
}

void MessageFolder::setParentId(const std::string& parentId)
{
    m_parent_id = parentId;
    m_parent_id_set = true;
}

std::string MessageFolder::getServiceId() const
{
    return m_service_id;
}

std::string MessageFolder::getContentType() const
{
    return m_content_type;
}

std::string MessageFolder::getName() const
{
    return m_name;
}

std::string MessageFolder::getPath() const
{
    return m_path;
}

MessageFolderType MessageFolder::getType() const
{
    return m_type;
}

bool MessageFolder::getSynchronizable() const
{
    return m_synchronizable;
}

void MessageFolder::setName(const std::string &value)
{
    m_name = value;
}

void MessageFolder::setSynchronizable(const bool &value)
{
    m_synchronizable = value;
}

MessageFolderType MessageFolder::convertPlatformFolderType(
        email_mailbox_type_e folderType)
{
    LoggerD("Entered");
    switch (folderType) {
        case email_mailbox_type_e::EMAIL_MAILBOX_TYPE_INBOX:
            return MessageFolderType::MESSAGE_FOLDER_TYPE_INBOX;
        case email_mailbox_type_e::EMAIL_MAILBOX_TYPE_SENTBOX:
            return MessageFolderType::MESSAGE_FOLDER_TYPE_SENTBOX;
        case email_mailbox_type_e::EMAIL_MAILBOX_TYPE_DRAFT:
            return MessageFolderType::MESSAGE_FOLDER_TYPE_DRAFTS;
        case email_mailbox_type_e::EMAIL_MAILBOX_TYPE_OUTBOX:
            return MessageFolderType::MESSAGE_FOLDER_TYPE_OUTBOX;
        case email_mailbox_type_e::EMAIL_MAILBOX_TYPE_ALL_EMAILS:
            return MessageFolderType::MESSAGE_FOLDER_TYPE_NOTSTANDARD;
        default:
            return MessageFolderType::MESSAGE_FOLDER_TYPE_NOTSTANDARD;
    }
}

/**
 *
 *  Attribute      | Attribute filter| Attribute range filter
 *                 | supported       | supported
 * ----------------+-----------------+------------------------
 *  id             | No              | No
 *  parentId       | No              | No
 *  serviceId      | Yes             | No
 *  contentType    | No              | No
 *  name           | No              | No
 *  path           | No              | No
 *  type           | No              | No
 *  synchronizable | No              | No
 */

namespace FOLDER_FILTER_ATTRIBUTE {
const std::string SERVICE_ID = "serviceId";
} //namespace FOLDER_FILTER_ATTRIBUTE

bool MessageFolder::isMatchingAttribute(const std::string& attribute_name,
            const FilterMatchFlag match_flag,
            AnyPtr match_value) const
{
    LoggerD("Entered");
    auto key = match_value->toString();
    LoggerD("attribute_name: %s match_flag:%d match_value:%s", attribute_name.c_str(),
            match_flag, key.c_str());

    using namespace FOLDER_FILTER_ATTRIBUTE;

    if (SERVICE_ID == attribute_name) {
        return FilterUtils::isStringMatching(key, getServiceId() , match_flag);
    }
    else {
        LoggerD("attribute:%s is NOT SUPPORTED", attribute_name.c_str());
    }

    return false;
}


bool MessageFolder::isMatchingAttributeRange(const std::string& attribute_name,
            AnyPtr initial_value,
            AnyPtr end_value) const
{
    LoggerD("Entered");
    LoggerD("attribute_name: %s NOT SUPPORTED", attribute_name.c_str());
    return false;
}

}    //messaging
}    //extension
