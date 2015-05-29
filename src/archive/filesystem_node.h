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

#ifndef _FILESYSTEM_NODE_H_
#define _FILESYSTEM_NODE_H_

#include <ctime>
#include <cstddef>
#include <sys/stat.h>
#include <set>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <ctime>
#include <map>

#include "common/platform_result.h"

namespace extension {
namespace filesystem {

enum LocationType
{
    LT_APPS,
    LT_DOCUMENTS,
    LT_DOWNLOADS,
    LT_GAMES,
    LT_IMAGES,
    LT_OTHERS,
    LT_ROOT,
    LT_SDCARD,
    LT_USBHOST,
    LT_SOUNDS,
    LT_TEMP,
    LT_VIDEOS,
    LT_RINGTONES,
    LT_WGTPKG,
    LT_WGTPRV,
    LT_WGTPRVTMP,
    LT_CAMERA
};

enum NodeType
{
    NT_DIRECTORY,
    NT_FILE
};

enum AccessMode
{
    AM_READ     = 0x0001,
    AM_WRITE    = 0x0002,
    AM_READ_WRITE = 0x0003,
    AM_APPEND   = 0x0004
};

/**
 * Used in @see Manager::access().
 */
enum AccessType
{
    AT_EXISTS   = 0x0000, //!< AT_EXISTS - checks for existence
    AT_READ     = 0x0001, //!< AT_READ   - checks for read access
    AT_WRITE    = 0x0002, //!< AT_WRITE  - checks for write access
    AT_EXEC     = 0x0004 //!< AT_EXEC   - checks for execution access
};

enum Permissions
{
    PERM_NONE   = 0x0000,
    PERM_READ   = 0x0001,
    PERM_WRITE  = 0x0002
};

enum PlatformMode
{
    PM_USER_READ    = 0x0100,
    PM_USER_WRITE   = 0x0080,
    PM_USER_EXEC    = 0x0040,
    PM_GROUP_READ   = 0x0020,
    PM_GROUP_WRITE  = 0x0010,
    PM_GROUP_EXEC   = 0x0008,
    PM_OTHER_READ   = 0x0004,
    PM_OTHER_WRITE  = 0x0002,
    PM_OTHER_EXEC   = 0x0001,
    PM_NONE         = 0x0000
};

enum Options
{
    OPT_NONE        = 0x0000,
    OPT_OVERWRITE   = 0x0001,
    OPT_RECURSIVE   = 0x0002
};

enum FindFilter
{
    FF_NAME,
    FF_CREATED,
    FF_MODIFIED,
    FF_SIZE
};
typedef std::map<FindFilter, std::string> FiltersMap;

class Node;
typedef std::shared_ptr<Node> NodePtr;
typedef std::vector<NodePtr> NodeList;
typedef NodeList::iterator NodeListIterator;

class Path;
typedef std::shared_ptr<Path> PathPtr;

class Node : public std::enable_shared_from_this<Node>
{
public:
    typedef std::vector<std::string> NameList;
    typedef NameList::iterator NameListIterator;

    //! \brief Resolves Path into Node
    //!
    //! This function creates Node wich represent folder or file
    //! in file system.
    //! @param path virtual path in filesystem
    //! @param node output pointer
    //! @return a platform result
    static common::PlatformResult resolve(const PathPtr& path, NodePtr* node);
    //! \brief Checks if path can be accessed
    //!
    //! Function opens path and base at reqested mode and type verifies access
    //! priviliges. If type is NT_DIRECTORY, then functions open directory.
    //! If mode is readonly, then function returns true. If mode is other than
    //! "r", then function checksm, if write is possible. If type is NT_FILE,
    //! then function open file for read mode for "r" mode! and for R/W
    //! for other mode.
    //!
    //! @param path virtual path in filesystem
    //! @param mode access mode: "r", "rw"
    //! @param type folder or file
    //! @param granted true if access is granted, false if not
    //! @return a platform result
    common::PlatformResult checkPermission(const PathPtr& path,
                                   const std::string& mode,
                                   NodeType type,
                                   bool* granted);
    //! \brief Gets path of current node.
    //! @return Node's path.
    PathPtr getPath() const;
    //! \brief Gets type of current node.
    //! @return Node's type.
    NodeType getType() const;
    //! \brief Gets permissions of the virtual node (not real filesystem node).
    //! @return Node's permissions.
    int getPermissions() const;
    //! \brief Sets permissions on the virtual node (not real filesystem node).
    //! @param perms Node's permissions @see Api::Filesystem::Permissions.
    void setPermissions(int perms);
    //! \brief Gets size of this node.
    //! @param size output size of a file.
    //! @return a platform result.
    common::PlatformResult getSize(unsigned long long* size) const;
    //! \brief Gets creation date of this node.
    //! @param time output date.
    //! @return a platform result.
    common::PlatformResult getCreated(std::time_t* time) const;
    //! \brief Gets last modification date of this node.
    //! @param time output date.
    //! @return a platform result.
    common::PlatformResult getModified(std::time_t* time) const;
    //! \brief Gets parent of this node.
    //! @param node the output node pointer or NULL if no parent (e.g. in case of a root node).
    //! @return a platform result
    common::PlatformResult getParent(NodePtr* node) const;
    //! \brief Gets platform permissions.
    //! @param mode output Platform permissions (set of flags from @see Permissions enum).
    //! @return a platform result
    common::PlatformResult getMode(int* mode) const;
    //! \brief Gets direct child of this node.
    //! @param path Path to the child node.
    //! @param output Ptr to the child node.
    //! @return a platform result
    //! @remarks Ownership passed to the caller.
    common::PlatformResult getChild(const PathPtr& path, NodePtr* node);
    //! \brief Gets list of names of direct child nodes.
    //! @param out_name_list the pointer to the list of nodes
    //! @return a platform result
    common::PlatformResult getChildNames(NameList* out_name_list) const;
    //! \brief Gets list of direct child nodes.
    //! @param out_node_list the pointer to the list of nodes
    //! @return a platform result
    //! @remarks Ownership passed to the caller.
    //! @deprecated
    common::PlatformResult getChildNodes(NodeList* out_node_list) const;
    //! \brief Creates child of current node.
    //! @param path Path to the node to create.
    //! @param type Type of the node @see Api::Filesystem::NodeType.
    //! @param node of the created file or directory
    //! @param options Additional options see remarks (no options by default).
    //! @return Ptr to newly created node.
    //! @remarks
    //! Valid options:
    //! - OPT_RECURSIVE - attempt to create all necessary sub-directories
    common::PlatformResult createChild(
        const PathPtr& path,
        NodeType,
        NodePtr* node,
        int options = OPT_NONE);
    //! \brief Removes underlying filesystem node.
    //! @param options Removal options (by default removal is recursive).
    //! @return a platform result
    //! @remarks Synchronous.
    //! Valid options:
    //! - OPT_RECURSIVE - remove node recursively.
    common::PlatformResult remove(int options);

    std::string toUri(int widgetId) const;

    static bool isSubPath(std::string aDirPath, PathPtr aFilePath);

private:
    static common::PlatformResult exists(const PathPtr& path, bool* existed);
    static common::PlatformResult stat(const PathPtr& path, struct stat* out_info);

    Node(const PathPtr& path, NodeType type);

    common::PlatformResult createAsFile(const PathPtr& path, NodePtr* node, int options);
    common::PlatformResult createAsFileInternal(const PathPtr& path);

    common::PlatformResult createAsDirectory(const PathPtr& path, NodePtr* node, int options);
    common::PlatformResult createAsDirectoryInternal(const PathPtr& path);

    common::PlatformResult removeAsFile(const PathPtr& path);
    common::PlatformResult removeAsDirectory(const PathPtr& path, bool recursive);

    PathPtr m_path;
    NodeType m_type;
    int m_perms;

};

} // extension
} // filesystem

#endif /* _FILESYSTEM_NODE_H_ */

