//
// Tizen Web Device API
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#ifndef _TIZEN_FILESYSTEM_FILE_H_
#define _TIZEN_FILESYSTEM_FILE_H_

#include <string>
#include <memory>
#include <vector>

#include "common/logger.h"

namespace DeviceAPI {
namespace Filesystem {

enum NodeType
{
    NT_DIRECTORY,
    NT_FILE
};

class File;
typedef std::shared_ptr<File> FilePtr;
typedef std::shared_ptr<File> NodePtr;  //STUB bad trick
typedef std::vector<NodePtr> NodeList;  //STUB bad trick

class File : public std::enable_shared_from_this<File>
{
public:
    File()
    {
    }

    File(const std::string& fname) :
            m_full_path(fname)
    {
    }

    FilePtr getNode()
    {
        LOGW("STUB");
        return shared_from_this();
    }

    size_t getSize()
    {
        LOGW("STUB");
        return 0;
    }

    FilePtr getPath()
    {
        LOGW("STUB");
        return shared_from_this();
    }

    //! \brief Gets type of current node.
    //! @return Node's type.
    NodeType getType() const
    {
        LOGW("STUB");
        return NT_FILE;
    }

    const std::string& getFullPath() const
    {
        return m_full_path;
    }

    std::string m_full_path;
};

class Path;
typedef std::shared_ptr<Path> PathPtr;

class Path : public std::enable_shared_from_this<Path>
{
public:
    typedef char SeparatorType;
    static Path::SeparatorType getSeparator()
    {
        LOGW("STUB");
        return '/';
    }
};

class External {
public:
    static std::string toVirtualPath(const std::string& path)
    {
        LOGW("STUB Not implemented path -> virtual path. Return not changed path");
        return path;
    }

    static void removeDirectoryRecursive(const std::string& fullpath)
    {
        LOGW("STUB Not implemented. Directory: [%s] will not be removed!", fullpath.c_str());
    }
};


} // Filesystem
} // DeviceAPI

#endif /* _TIZEN_FILESYSTEM_FILE_H_ */
