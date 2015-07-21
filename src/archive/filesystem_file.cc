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
#include "filesystem_file.h"
#include "common/logger.h"

using namespace common;

namespace extension {
namespace filesystem {

File::File(NodePtr node, const File::PermissionList &parentPermissions,
        const std::string& original_location) :
    m_node(node),
    m_parentPerms(parentPermissions)
{
    LoggerD("original_location is fullPath: %s", original_location.c_str());
    m_original_fullpath = original_location;
}

File::~File()
{
    LoggerD("Enter");
}

NodePtr File::getNode() const
{
    return m_node;
}

File::PermissionList File::getParentPermissions() const
{
    return m_parentPerms;
}

void File::setParentPermissions(const PermissionList &permissions)
{
    m_parentPerms = permissions;
}

void File::pushParentPermissions(int permissions)
{
    m_parentPerms.push_back(permissions);
}

const std::string& File::getOriginalURI() const
{
    return m_original_URI;
}

const std::string& File::getOriginalFullPath() const
{
    return m_original_fullpath;
}

std::string External::toVirtualPath(const std::string& path)
{
    //TODO::implement this method
    LoggerW("Just STUB");
    return path;
}

} // filesystem
} // extension
