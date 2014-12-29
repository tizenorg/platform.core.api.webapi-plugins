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

/**
 * @file        Node.cpp
 */

#include "filesystem_node.h"

#include <algorithm>
#include <memory>
#include <typeinfo>
#include <sys/types.h>
#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <pcrecpp.h>
#include <sstream>
#include <fstream>

#include "filesystem_path.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace filesystem {

using namespace common;

#define MAX_NODE_LENGTH 256
bool Node::checkPermission(const PathPtr &path, const std::string &mode, NodeType type)
{
    switch (type)
    {
        case NT_DIRECTORY:
        {
            DIR* dir = opendir(path->getFullPath().c_str());

            if (!dir) {
                LoggerW("throw InvalidValuesException");
                throw InvalidValuesException("Node has been deleted from platform.");
            }

            if (closedir(dir) != 0) {
                LoggerW("throw InvalidValuesException");
                throw InvalidValuesException("Could not close platform node.");
            }

            if (mode == "r")
                return true;

            std::stringstream ss;
            time_t now;
            time(&now);
            ss << now;
            PathPtr tempFilePath = Path::create(path->getFullPath());
            tempFilePath->append(ss.str());
            try
            {
                createAsFileInternal(tempFilePath);
                removeAsFile(tempFilePath);
            }
            catch (const PlatformException& ex)
            {
                LoggerW("Exception: %s", ex.message().c_str());
                return false;
            }

            if (mode == "rw" || mode == "w"  || mode == "a") {
                return true;
            }
            LoggerW("throw InvalidValuesException");
            throw InvalidValuesException("invalid mode");
        }
        break;
        case NT_FILE:
        {
            std::fstream stream;
            std::ios_base::openmode modeBit = std::fstream::binary;

            if (mode == "r")
            {
                modeBit |= 	std::fstream::in;
            }
            else if (mode == "rw" || mode == "w" || mode == "a")
            {
                modeBit |= 	std::fstream::app;
            }
            else
            {
                LoggerW("throw InvalidValuesException");
                throw InvalidValuesException("invalid mode");
            }

            stream.open(path->getFullPath().c_str(), modeBit);

            if (stream.is_open())
            {
                stream.close();
                return true;
            }
            return false;
        }
        break;
    }
    return false;
}

NodePtr Node::resolve(const PathPtr& path)
{
    LoggerD("Entered path:[%s]", path->getFullPath().c_str());

    struct stat info;
    struct stat syminfo;

    if (lstat(path->getFullPath().c_str(), &info) != 0) {
        LoggerE("File:[%s] error no:%d", path->getFullPath().c_str(), errno);

        switch (errno)
        {
            case EACCES:
                LoggerW("throw InvalidValuesException for file:[%s]", path->getFullPath().c_str());
                throw InvalidValuesException("Node access denied");
                break;
            case ENOENT:
                LoggerW("throw NotFoundException for file:[%s]", path->getFullPath().c_str());
                throw NotFoundException("NotFoundError");
                break;
            default:
                LoggerW("throw IOException for file:[%s]", path->getFullPath().c_str());
                throw IOException("Platform exception fail");
        }
    }

    if (!S_ISDIR(info.st_mode) & !S_ISREG(info.st_mode) && !S_ISLNK(info.st_mode)) {
        LoggerW("throw IOException for file:[%s]", path->getFullPath().c_str());
        throw IOException("Platform node is of unsupported type.");
    }

    NodeType type = S_ISDIR(info.st_mode) ? NT_DIRECTORY : NT_FILE;

    if (S_ISLNK(info.st_mode)) {
        syminfo = stat(path);

        type = S_ISDIR(syminfo.st_mode) ? NT_DIRECTORY : NT_FILE;
        LoggerD("%x", type);
    }

    auto result = std::shared_ptr<Node>(new Node(path, type));

    LoggerD("Finished execution for file:[%s] type:%s", path->getFullPath().c_str(),
            type == NT_DIRECTORY ? "directory" : "file");
    return result;
}

PathPtr Node::getPath() const
{
    return Path::create(m_path->getFullPath());
}

NodePtr Node::getChild(const PathPtr& path)
{
    if (m_type != NT_DIRECTORY) {
        LoggerW("throw IOException");
        throw IOException("Not a directory.");
    }
    return Node::resolve(*m_path + *path);
}

NodeType Node::getType() const
{
    return m_type;
}

int Node::getPermissions() const
{
    return m_perms;
}

void Node::setPermissions(int perms)
{
    m_perms = perms;
}

Node::NameList Node::getChildNames() const
{
    if (m_type != NT_DIRECTORY) {
        LoggerW("throw IOException");
        throw IOException("Node is not directory.");
    }

    if ((m_perms & PERM_READ) == 0) {
        LoggerW("throw InvalidValuesException");
        throw InvalidValuesException("No permission.");
    }

    DIR* dir = opendir(m_path->getFullPath().c_str());
    if (!dir) {
        LoggerW("throw IOException");
        throw IOException("Node has been deleted from platform.");
    }

    NameList result;
    errno = 0;
    struct dirent *entry = NULL;
    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strncmp(entry->d_name, "..", 2)) {
            continue;
        }
        result.push_back(entry->d_name);
    }
    if (errno != 0) {
        LoggerW("throw IOException");
        throw IOException("Error while reading directory.");
    }

    if (closedir(dir) != 0) {
        LoggerW("throw IOException");
        throw IOException("Could not close platform node.");
    }

    return result;
}

NodeList Node::getChildNodes() const
{
    if (m_type != NT_DIRECTORY) {
        LoggerW("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerW("throw IOException");
        throw IOException("Node is not directory.");
        LoggerW("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
    }

    if ((m_perms & PERM_READ) == 0) {
        LoggerW("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerW("throw InvalidValuesException");
        throw InvalidValuesException("No permission.");
    }

    DIR* dir = opendir(m_path->getFullPath().c_str());
    if (!dir) {
        LoggerW("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerW("throw IOException");
        throw IOException("Node has been deleted from platform.");
    }

    errno = 0;
    NodeList result;
    struct dirent *entry = NULL;
    while ((entry = readdir(dir))) {
        if (!strcmp(entry->d_name, ".") || !strncmp(entry->d_name, "..", 2)) {
            continue;
        }
        try {
            NodePtr node = Node::resolve(*m_path + entry->d_name);
            node->setPermissions(getPermissions()); // inherit access rights
            result.push_back(node);
        }
        catch (const PlatformException& ex)
        {
            LoggerW("caught BasePlatformException ignored");
        }
    }

    if (errno != 0) {
        LoggerW("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerW("throw IOException");
        throw IOException("Error while reading directory.");
    }

    if (closedir(dir) != 0) {
        LoggerW("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerW("throw IOException");
        throw IOException("Could not close platform node.");
    }

    return result;
}

NodePtr Node::createChild(
        const PathPtr& path,
        NodeType type,
        int options)
{
    if (m_type != NT_DIRECTORY) {
        LoggerW("throw IOException");
        throw IOException("Parent node is not a directory.");
    }

    if ((m_perms & PERM_WRITE) == 0) {
        LoggerW("throw InvalidValuesException");
        throw InvalidValuesException("Not enough permissions.");
    }

    PathPtr childPath = *m_path + *path;
    if (exists(childPath)) {
        LoggerW("throw IOException");
        throw IOException("Node already exists.");
    }

    NodePtr result;
    switch (type) {
        case NT_FILE:
            result = createAsFile(childPath, options);
            break;
        case NT_DIRECTORY:
            result = createAsDirectory(childPath, options);
            break;
        default:
            LoggerW("throw IOException");
            throw IOException("Unsupported node type.");
    }
    if (!!result) {
        result->m_perms = m_perms;
    } else {
        LoggerW("throw IOException");
        throw IOException("Node creation error");
    }

    return result;
}

void Node::remove(int options)
{
    switch (m_type) {
        case NT_FILE:
            removeAsFile(m_path);
            break;
        case NT_DIRECTORY:
            removeAsDirectory(m_path, (options & OPT_RECURSIVE));
            break;
        default:
            LoggerE("throw UnknownError");
            throw UnknownException("Not supported value of m_type");
    }
}

unsigned long long Node::getSize() const
{
    if (m_type == NT_DIRECTORY) {
        LoggerW("throw IOException");
        throw IOException("Getting size for directories is not supported.");
    }

    struct stat info = stat(m_path);
    if (!S_ISREG(info.st_mode)) {
        LoggerW("throw IOException");
        throw IOException("Specified node is not a regular file.");
    }

    return info.st_size;
}

std::time_t Node::getCreated() const
{
    return stat(m_path).st_ctime;
}

std::time_t Node::getModified() const
{
    return stat(m_path).st_mtime;
}

// TODO Optimize it, maybe store a flag indicating that node is a root.
NodePtr Node::getParent() const
{
//    LocationPaths roots = Manager::getInstance().getLocationPaths();
//    for (LocationPaths::iterator it = roots.begin(); it != roots.end(); ++it) {
//        if (*(*it) == *m_path) {
//            return NodePtr();
//        }
//    }
    NodePtr parent = Node::resolve(Path::create(m_path->getPath()));
    parent->setPermissions(getPermissions());
    return parent;
}

int Node::getMode() const
{
    int result = 0;
    struct stat info = stat(m_path);
    if (info.st_mode & S_IRUSR) { result |= PM_USER_READ; }
    if (info.st_mode & S_IWUSR) { result |= PM_USER_WRITE; }
    if (info.st_mode & S_IXUSR) { result |= PM_USER_EXEC; }
    if (info.st_mode & S_IRGRP) { result |= PM_GROUP_READ; }
    if (info.st_mode & S_IWGRP) { result |= PM_GROUP_WRITE; }
    if (info.st_mode & S_IXGRP) { result |= PM_GROUP_EXEC; }
    if (info.st_mode & S_IROTH) { result |= PM_OTHER_READ; }
    if (info.st_mode & S_IWOTH) { result |= PM_OTHER_WRITE; }
    if (info.st_mode & S_IXOTH) { result |= PM_OTHER_EXEC; }
    return result;
}

bool Node::exists(const PathPtr& path)
{
    struct stat info;
    memset(&info, 0, sizeof(struct stat));
    int status = lstat(path->getFullPath().c_str(), &info);

    if (0 == status)
    {
        return true;
    }
    else if (ENAMETOOLONG == errno)
    {
        LoggerW("throw IOException");
        throw IOException("file name is too long");
    }
    else if (errno != ENOENT)
    {
        return true;
    }
    return false;
}

struct stat Node::stat(const PathPtr& path)
{
    struct stat result;
    memset(&result, 0, sizeof(struct stat));
    if (::stat(path->getFullPath().c_str(),
            &result) != 0)
    {
        LoggerE("File: %s", path->getFullPath().c_str());
        LoggerW("throw IOException");
        throw IOException("Node does not exist or no access");
    }
    return result;
}

Node::Node(const PathPtr& path, NodeType type):
            m_path(path),
            m_type(type),
            m_perms(PERM_NONE)
{
}

NodePtr Node::createAsFile(const PathPtr& path,
        int /* options */)
{
    createAsFileInternal(path);
    return std::shared_ptr<Node>(new Node(path, NT_FILE));
}

void Node::createAsFileInternal(const PathPtr& path)
{
    FILE* file = std::fopen(path->getFullPath().c_str(), "wb");
    if (!file) {
        LoggerW("fopen fails IOException throw for path [%s]",
                path->getFullPath().c_str());
        throw IOException("Platform node could not be created.");
    }
    std::fclose(file);
}

NodePtr Node::createAsDirectory(const PathPtr& path,
        int options)
{
//    if (options & OPT_RECURSIVE) {
//        auto parts = Utils::getParts(path);
//        for (auto it = parts.begin(); it != parts.end(); ++it) {
//            if (!exists(*it)) { createAsDirectoryInternal(*it); }
//        }
//    }
    createAsDirectoryInternal(path);
    return std::shared_ptr<Node>(new Node(path, NT_DIRECTORY));
}

void Node::createAsDirectoryInternal(const PathPtr& path)
{
    if (mkdir(path->getFullPath().c_str(), S_IRWXU | S_IRWXG | S_IROTH |
            S_IXOTH) != 0) {
        LoggerW("throw IOException");
        throw IOException("Platform node could not be created.");
    }
}

void Node::removeAsFile(const PathPtr& path)
{
    auto fullPath = path->getFullPath();
    if (unlink(fullPath.c_str()) != 0) {
        LoggerW("remove [%s]", fullPath.c_str());
        LoggerW("throw IOException");
        throw IOException("Error while removing platform node.");
    }
}

void Node::removeAsDirectory(const PathPtr& path,
        bool recursive)
{
    if (recursive) {
        DIR* dir = opendir(path->getFullPath().c_str());
        if (!dir) {
            LoggerW("File %s", path->getFullPath().c_str());
            LoggerW("throw IOException");
            throw IOException("Node does not exist or access denied.");
        }
        errno = 0;
        struct dirent *entry = NULL;
        while ((entry = readdir(dir))) {
            if (!strcmp(entry->d_name, ".") || !strncmp(entry->d_name, "..", 2)) {
                continue;
            }
            PathPtr subPath = *path + entry->d_name;
            struct stat info;
            memset(&info, 0, sizeof(struct stat));
            if (lstat(subPath->getFullPath().c_str(), &info) == 0) {
                try {
                    if (S_ISDIR(info.st_mode)) {
                        removeAsDirectory(subPath, true);
                    } else if (S_ISREG(info.st_mode)) {
                        removeAsFile(subPath);
                    }
                }
                catch (const PlatformException& ex)
                {
                }
                // TODO: Not sure if above exception should be swallowed.
            }
        }
        closedir(dir);
    }

    errno = 0;
    if (rmdir(path->getFullPath().c_str()) != 0) {
        if (errno == EEXIST) {
            LoggerW("throw IOException");
            throw IOException("Node has child nodes.");
        }
        LoggerW("throw IOException");
        throw IOException("Error while removing platform node.");
    }
}

std::string Node::toUri(int /*widgetId*/) const
{
    // TODO I believe moving this feature to WrtWrapper would make more sense.
    return "file://" + m_path->getFullPath();
}

bool Node::isSubPath(std::string aDirPath, PathPtr aFilePath)
{
    auto myPath = aDirPath;
    if(!myPath.empty() && myPath[myPath.length()-1] != Path::getSeparator()) {
        myPath += Path::getSeparator();
        LoggerD("updated aDirPath to:%s", aDirPath.c_str());
    }

    auto childPath = aFilePath->getFullPath();
    bool isSubP = strncmp(myPath.c_str(), childPath.c_str(), myPath.size()) == 0;

    LoggerD("aDirPath:%s myPath:%s aFilePath:%s isSubP:%d",
            aDirPath.c_str(),
            myPath.c_str(),
            aFilePath->getFullPath().c_str(),
            isSubP);

    return isSubP;
}

bool Node::isSubPath(PathPtr aPath) const
{
    LoggerD("Entered thisPath:%s aPath: %s",
            this->getPath()->getFullPath().c_str(),
            aPath->getFullPath().c_str());

    resolve(aPath);

    bool isSubP = isSubPath(m_path->getFullPath(), aPath);
    LoggerD("thisPath:%s aPath: %s isSubP:%d",
            this->getPath()->getFullPath().c_str(),
            aPath->getFullPath().c_str(),
            isSubP);
    return isSubP;
}

}
}
