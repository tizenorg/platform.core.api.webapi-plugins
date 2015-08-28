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
#include "filesystem_node.h"
#include "common/logger.h"

namespace extension {
namespace filesystem {

using namespace common;

#define MAX_NODE_LENGTH 256
PlatformResult Node::checkPermission(const PathPtr &path, const std::string &mode, NodeType type, bool* granted)
{
    LoggerD("Enter");
    *granted = false;

    switch (type)
    {
        case NT_DIRECTORY:
        {
            DIR* dir = opendir(path->getFullPath().c_str());

            if (!dir) {
                LoggerE("throw InvalidValuesException");
                return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Node has been deleted from platform.");
            }

            if (closedir(dir) != 0) {
                LoggerE("throw InvalidValuesException");
                return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Could not close platform node.");
            }

            if (mode == "r") {
                *granted = true;
                return PlatformResult(ErrorCode::NO_ERROR);
            }

            std::stringstream ss;
            time_t now;
            time(&now);
            ss << now;
            PathPtr tempFilePath = Path::create(path->getFullPath());
            tempFilePath->append(ss.str());

            PlatformResult result = createAsFileInternal(tempFilePath);
            if (result.error_code() != ErrorCode::NO_ERROR) {
                return result;
            }

            result = removeAsFile(tempFilePath);
            if (result.error_code() != ErrorCode::NO_ERROR) {
                return result;
            }

            if (mode == "rw" || mode == "w"  || mode == "a") {
                *granted = true;
                return PlatformResult(ErrorCode::NO_ERROR);
            }
            LoggerE("throw InvalidValuesException");
            return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "invalid mode");
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
                LoggerE("throw InvalidValuesException");
                return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "invalid mode");
            }

            stream.open(path->getFullPath().c_str(), modeBit);

            if (stream.is_open())
            {
                stream.close();
                *granted = true;
            }
            return PlatformResult(ErrorCode::NO_ERROR);
        }
        break;
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::resolve(const PathPtr& path, NodePtr* node)
{
    LoggerD("Entered path:[%s]", path->getFullPath().c_str());

    struct stat info;
    struct stat syminfo;

    if (lstat(path->getFullPath().c_str(), &info) != 0) {
        LoggerE("File:[%s] error no:%d", path->getFullPath().c_str(), errno);

        switch (errno)
        {
            case EACCES:
                LoggerE("throw InvalidValuesException for file:[%s]", path->getFullPath().c_str());
                return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Node access denied");
                break;
            case ENOENT:
                LoggerE("throw NotFoundException for file:[%s]", path->getFullPath().c_str());
                return PlatformResult(ErrorCode::NOT_FOUND_ERR, "NotFoundError");
                break;
            default:
                LoggerE("throw IOException for file:[%s]", path->getFullPath().c_str());
                return PlatformResult(ErrorCode::IO_ERR, "Platform exception fail");
        }
    }

    if ((!S_ISDIR(info.st_mode)) & (!S_ISREG(info.st_mode)) && !S_ISLNK(info.st_mode)) {
        LoggerE("throw IOException for file:[%s]", path->getFullPath().c_str());
        return PlatformResult(ErrorCode::IO_ERR, "Platform node is of unsupported type.");
    }

    NodeType type = S_ISDIR(info.st_mode) ? NT_DIRECTORY : NT_FILE;

    if (S_ISLNK(info.st_mode)) {
        PlatformResult result = stat(path, &syminfo);
        if (result.error_code() != ErrorCode::NO_ERROR) {
            LoggerE("Error: %s", result.message().c_str());
            return result;
        }

        type = S_ISDIR(syminfo.st_mode) ? NT_DIRECTORY : NT_FILE;
        LoggerD("%x", type);
    }

    *node = std::shared_ptr<Node>(new Node(path, type));

    LoggerD("Finished execution for file:[%s] type:%s", path->getFullPath().c_str(),
            type == NT_DIRECTORY ? "directory" : "file");

    return PlatformResult(ErrorCode::NO_ERROR);
}

PathPtr Node::getPath() const
{
    return Path::create(m_path->getFullPath());
}

PlatformResult Node::getChild(const PathPtr& path, NodePtr* node)
{
    LoggerD("Enter");
    if (m_type != NT_DIRECTORY) {
        LoggerW("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Not a directory.");
    }
    return Node::resolve(*m_path + *path, node);
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

PlatformResult Node::getChildNames(Node::NameList* out_name_list) const
{
    LoggerD("Enter");
    if (m_type != NT_DIRECTORY) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Node is not directory.");
    }

    if ((m_perms & PERM_READ) == 0) {
        LoggerE("throw InvalidValuesException");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "No permission.");
    }

    DIR* dir = opendir(m_path->getFullPath().c_str());
    if (!dir) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Node has been deleted from platform.");
    }

    int err = 0;
    struct dirent entry = {0};
    struct dirent* result = nullptr;
    NameList name_list;
    while ((0 == (err = readdir_r(dir, &entry, &result))) && result) {
        if (!strcmp(entry.d_name, ".") || !strncmp(entry.d_name, "..", 2)) {
            continue;
        }
        name_list.push_back(entry.d_name);
    }

    if (closedir(dir) != 0) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Could not close platform node.");
    }

    if (0 != err) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Error while reading directory.");
    }

    *out_name_list = name_list;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::getChildNodes(NodeList* out_node_list) const
{
    LoggerD("Enter");

    if (m_type != NT_DIRECTORY) {
        LoggerE("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerE("throw IOException");
        LoggerE("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        return PlatformResult(ErrorCode::IO_ERR, "Node is not directory.");
    }

    if ((m_perms & PERM_READ) == 0) {
        LoggerE("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerE("throw InvalidValuesException");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "No permission.");
    }

    DIR* dir = opendir(m_path->getFullPath().c_str());
    if (!dir) {
        LoggerE("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Node has been deleted from platform.");
    }

    int err = 0;
    struct dirent entry = {0};
    struct dirent* result = nullptr;
    NodeList node_list;
    while ((0 == (err = readdir_r(dir, &entry, &result))) && result) {
        if (!strcmp(entry.d_name, ".") || !strncmp(entry.d_name, "..", 2)) {
            continue;
        }

        NodePtr node;
        Node::resolve(*m_path + entry.d_name, &node);
        node->setPermissions(getPermissions()); // inherit access rights
        node_list.push_back(node);
    }

    if (closedir(dir) != 0) {
        LoggerE("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Could not close platform node.");
    }

    if (0 != err) {
        LoggerE("Path %s Perm %d", m_path->getFullPath().c_str(), m_perms);
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Error while reading directory.");
    }

    *out_node_list = node_list;

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::createChild(
        const PathPtr& path,
        NodeType type,
        NodePtr* node,
        int options)
{
    LoggerD("Enter");
    if (m_type != NT_DIRECTORY) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Parent node is not a directory.");
    }

    if ((m_perms & PERM_WRITE) == 0) {
        LoggerE("throw InvalidValuesException");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Not enough permissions.");
    }

    PathPtr childPath = *m_path + *path;
    bool existed;
    PlatformResult result = exists(childPath, &existed);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail: exists()");
        return result;
    }
    if (existed) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Node already exists.");
    }

    switch (type) {
        case NT_FILE:
            result = createAsFile(childPath, node, options);
            break;
        case NT_DIRECTORY:
            result = createAsDirectory(childPath, node, options);
            break;
        default:
            LoggerE("throw IOException");
            return PlatformResult(ErrorCode::IO_ERR, "Unsupported node type.");
    }
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail CreateAs...()");
        return result;
    }

    if (!!(*node)) {
        (*node)->m_perms = m_perms;
    } else {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Node creation error");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::remove(int options)
{
    LoggerD("Enter");
    PlatformResult result(ErrorCode::NO_ERROR);
    switch (m_type) {
        case NT_FILE:
            result = removeAsFile(m_path);
            break;
        case NT_DIRECTORY:
            result = removeAsDirectory(m_path, (options & OPT_RECURSIVE));
            break;
        default:
            LoggerE("throw UnknownError");
            result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Not supported value of m_type");
    }
    return result;
}

PlatformResult Node::getSize(unsigned long long* size) const
{
    LoggerD("Enter");
    if (m_type == NT_DIRECTORY) {
        LoggerE("Getting size for directories is not supported.");
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Getting size for directories is not supported.");
    }

    struct stat info;
    PlatformResult result = stat(m_path, &info);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail: stat()");
        return result;
    }

    if (!S_ISREG(info.st_mode)) {
        LoggerE("Specified node is not a regular file.");
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Specified node is not a regular file.");
    }

    *size =  info.st_size;

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::getCreated(std::time_t* time) const
{
    LoggerD("Enter");
    struct stat info;
    PlatformResult result = stat(m_path, &info);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail: stat()");
        return result;
    }
    *time = info.st_ctime;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::getModified(std::time_t* time) const
{
    LoggerD("Enter");
    struct stat info;
    PlatformResult result = stat(m_path, &info);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail: stat()");
        return result;
    }
    *time = info.st_mtime;
    return PlatformResult(ErrorCode::NO_ERROR);
}

// TODO Optimize it, maybe store a flag indicating that node is a root.
PlatformResult Node::getParent(NodePtr* node) const
{
    LoggerD("Enter");

//    LocationPaths roots = Manager::getInstance().getLocationPaths();
//    for (LocationPaths::iterator it = roots.begin(); it != roots.end(); ++it) {
//        if (*(*it) == *m_path) {
//            return NodePtr();
//        }
//    }
    PlatformResult result = Node::resolve(Path::create(m_path->getPath()), node);
    if (result.error_code() == ErrorCode::NO_ERROR) {
        (*node)->setPermissions(getPermissions());
    }

    return result;
}

PlatformResult Node::getMode(int* mode) const
{
    LoggerD("Enter");

    struct stat info;
    PlatformResult result = stat(m_path, &info);
    if (result.error_code() == ErrorCode::NO_ERROR) {
        *mode = 0;
        if (info.st_mode & S_IRUSR) { *mode |= PM_USER_READ; }
        if (info.st_mode & S_IWUSR) { *mode |= PM_USER_WRITE; }
        if (info.st_mode & S_IXUSR) { *mode |= PM_USER_EXEC; }
        if (info.st_mode & S_IRGRP) { *mode |= PM_GROUP_READ; }
        if (info.st_mode & S_IWGRP) { *mode |= PM_GROUP_WRITE; }
        if (info.st_mode & S_IXGRP) { *mode |= PM_GROUP_EXEC; }
        if (info.st_mode & S_IROTH) { *mode |= PM_OTHER_READ; }
        if (info.st_mode & S_IWOTH) { *mode |= PM_OTHER_WRITE; }
        if (info.st_mode & S_IXOTH) { *mode |= PM_OTHER_EXEC; }
    }
    return result;
}

PlatformResult Node::exists(const PathPtr& path, bool* existed)
{
    LoggerD("Enter");

    struct stat info;
    memset(&info, 0, sizeof(struct stat));
    int status = lstat(path->getFullPath().c_str(), &info);

    *existed = false;

    if (0 == status)
    {
        *existed = true;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    else if (ENAMETOOLONG == errno)
    {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "file name is too long");
    }
    else if (errno != ENOENT)
    {
        *existed = true;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::stat(const PathPtr& path, struct stat* out_info)
{
    LoggerD("Enter");
    struct stat info;
    memset(&info, 0, sizeof(struct stat));

    if (::stat(path->getFullPath().c_str(), &info) != 0)
    {
        LoggerE("File: %s", path->getFullPath().c_str());
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Node does not exist or no access");
    }
    *out_info = info;
    return PlatformResult(ErrorCode::NO_ERROR);
}

Node::Node(const PathPtr& path, NodeType type):
            m_path(path),
            m_type(type),
            m_perms(PERM_NONE)
{
    LoggerD("Enter");
}

PlatformResult Node::createAsFile(const PathPtr& path, NodePtr* node, int /* options */)
{
    LoggerD("Enter");

    PlatformResult result = createAsFileInternal(path);
    if (result.error_code() == ErrorCode::NO_ERROR) {
        node->reset(new Node(path, NT_FILE));
    }
    return result;
}

PlatformResult Node::createAsFileInternal(const PathPtr& path)
{
    LoggerD("Enter");

    FILE* file = std::fopen(path->getFullPath().c_str(), "wb");
    if (!file) {
        LoggerE("fopen fails IOException throw for path [%s]",
                path->getFullPath().c_str());
        return PlatformResult(ErrorCode::IO_ERR, "Platform node could not be created.");
    }
    std::fclose(file);
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::createAsDirectory(const PathPtr& path, NodePtr* node, int options)
{
    LoggerD("Enter");

//    if (options & OPT_RECURSIVE) {
//        auto parts = Utils::getParts(path);
//        for (auto it = parts.begin(); it != parts.end(); ++it) {
//            if (!exists(*it)) { createAsDirectoryInternal(*it); }
//        }
//    }
    PlatformResult result = createAsDirectoryInternal(path);
    if (result.error_code() == ErrorCode::NO_ERROR) {
        node->reset(new Node(path, NT_DIRECTORY));
    }
    return result;
}

PlatformResult Node::createAsDirectoryInternal(const PathPtr& path)
{
    LoggerD("Enter");

    if (mkdir(path->getFullPath().c_str(), S_IRWXU | S_IRWXG | S_IROTH |
            S_IXOTH) != 0) {
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Platform node could not be created.");
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::removeAsFile(const PathPtr& path)
{
    LoggerD("Enter");

    auto fullPath = path->getFullPath();
    if (unlink(fullPath.c_str()) != 0) {
        LoggerE("remove [%s]", fullPath.c_str());
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Error while removing platform node.");
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Node::removeAsDirectory(const PathPtr& path, bool recursive)
{
    LoggerD("Enter");

    if (recursive) {
        DIR* dir = opendir(path->getFullPath().c_str());
        if (!dir) {
            LoggerE("File %s", path->getFullPath().c_str());
            LoggerE("throw IOException");
            return PlatformResult(ErrorCode::IO_ERR, "Node does not exist or access denied.");
        }
        struct dirent entry = {0};
        struct dirent* result = nullptr;
        PlatformResult platform_result(ErrorCode::NO_ERROR);
        while ((0 == (readdir_r(dir, &entry, &result))) && result) {
            if (!strcmp(entry.d_name, ".") || !strncmp(entry.d_name, "..", 2)) {
                continue;
            }
            PathPtr subPath = *path + entry.d_name;
            struct stat info;
            memset(&info, 0, sizeof(struct stat));
            if (lstat(subPath->getFullPath().c_str(), &info) == 0) {
                if (S_ISDIR(info.st_mode)) {
                    platform_result = removeAsDirectory(subPath, true);
                } else if (S_ISREG(info.st_mode)) {
                    platform_result = removeAsFile(subPath);
                }
                if (platform_result.error_code() != ErrorCode::NO_ERROR) {
                    LoggerE("Fail: getFullPath() (%d)",platform_result.error_code());
                    closedir(dir);
                    return platform_result;
                }
            }
        }
        closedir(dir);
    }

    errno = 0;
    if (rmdir(path->getFullPath().c_str()) != 0) {
        if (errno == EEXIST) {
            LoggerE("throw IOException");
            return PlatformResult(ErrorCode::IO_ERR, "Node has child nodes.");
        }
        LoggerE("throw IOException");
        return PlatformResult(ErrorCode::IO_ERR, "Error while removing platform node.");
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

std::string Node::toUri(int /*widgetId*/) const
{
    LoggerD("Enter");
    // TODO I believe moving this feature to WrtWrapper would make more sense.
    return "file://" + m_path->getFullPath();
}

bool Node::isSubPath(std::string aDirPath, PathPtr aFilePath)
{
    LoggerD("Enter");
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

}   // filesystem
}   // extension
