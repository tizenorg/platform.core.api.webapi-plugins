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

#ifndef _TIZEN_FILESYSTEM_FILE_H_
#define _TIZEN_FILESYSTEM_FILE_H_

#include <string>
#include <memory>
#include <vector>

#include "common/logger.h"
#include "filesystem_node.h"
#include "filesystem_path.h"

namespace extension {
namespace filesystem {

class File;
typedef std::shared_ptr<File> FilePtr;

class File : public std::enable_shared_from_this<File>
{
public:
    typedef std::vector<int> PermissionList;

    File(NodePtr node,
            const PermissionList &parentPermissions,
            const std::string& original_location = std::string());
    virtual ~File();

    NodePtr getNode() const;
    PermissionList getParentPermissions() const;
    void setParentPermissions(const PermissionList &permissions);
    void pushParentPermissions(int permissions);

    const std::string& getOriginalURI() const;
    const std::string& getOriginalFullPath() const;
private:
    NodePtr m_node;
    PermissionList m_parentPerms;

    std::string m_original_URI;
    std::string m_original_fullpath;
};

class External {
public:
    static std::string toVirtualPath(const std::string& path);
};


} // filesystem
} // extension

#endif /* _TIZEN_FILESYSTEM_FILE_H_ */
