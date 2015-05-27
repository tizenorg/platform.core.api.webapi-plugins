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

#include <memory>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include <limits.h>

#include "filesystem_path.h"
#include "common/logger.h"

namespace extension {
namespace filesystem {

const Path::SeparatorType Path::m_pathSeparator = '/';

PathPtr Path::create(const std::string& path)
{
    LoggerD("Enter");
    auto result = std::shared_ptr<Path>(new Path());
    result->reset(path);
    return result;
}

std::string Path::getFullPath() const
{
    return m_fullPath;
}

std::string Path::getPath() const
{
    return m_path;
}

std::string Path::getName() const
{
    return m_name;
}

PathPtr Path::append(const std::string& path)
{
    reset(m_fullPath + m_pathSeparator + path);
    return shared_from_this();
}

PathPtr Path::append(const PathPtr& path)
{
    reset(m_fullPath + m_pathSeparator + path->getFullPath());
    return shared_from_this();
}

bool Path::isAbsolute() const
{
    return (!m_fullPath.empty() && (m_fullPath[0] == m_pathSeparator));
}

Path::SeparatorType Path::getSeparator()
{
    return m_pathSeparator;
}

bool Path::isValid(const std::string& str)
{
    return !str.empty();
}

PathPtr Path::clone() const
{
    return Path::create(m_fullPath);
}

Path::Path()
{
}

void Path::reset(const std::string& str)
{
    LoggerD("Enter");
    if (!isValid(str)) {
        LoggerD("Invalid string %s", str.c_str());
        LoggerW("throw NotFoundException");
        // The only way to remove throw statement from here is to just return, because
        // this function is used in the operator functions and they're not able
        // to handle a PlatformResult value;
        return;
    }
//    std::string tmp = Commons::String::unique(/*Commons::String::trim*/(
//                                                  str), m_pathSeparator);
    std::string tmp = str;

    if (m_pathSeparator == tmp.back()) {
        tmp.erase(tmp.end() - 1, tmp.end());
    }
    std::string::size_type pos = tmp.find_last_of(m_pathSeparator);
    if (std::string::npos == pos) {
        m_fullPath = m_name = tmp;
        m_path.clear();
    } else {
        if (0 == pos) {
            m_fullPath = m_path = m_pathSeparator;
        } else {
            m_fullPath = m_path = tmp.substr(0, pos);
            m_fullPath += m_pathSeparator;
        }
        m_name = tmp.substr(pos + 1);
        m_fullPath += m_name;
    }
}

PathPtr operator+(const Path& lhs, const Path& rhs)
{
    return Path::create(lhs.getFullPath())->append(rhs.getFullPath());
}

PathPtr operator+(const Path& lhs, const std::string& rhs)
{
    return Path::create(lhs.getFullPath())->append(rhs);
}

PathPtr operator+(const std::string& lhs, const Path& rhs)
{
    return Path::create(lhs)->append(rhs.getFullPath());
}

bool operator==(const Path& lhs, const Path& rhs)
{
    return (lhs.getFullPath() == rhs.getFullPath());
}

bool operator==(const Path& lhs, const std::string& rhs)
{
    return (lhs.getFullPath() == rhs);
}

bool operator==(const std::string& lhs, const Path& rhs)
{
    return (lhs == rhs.getFullPath());
}

bool operator!=(const Path& lhs, const Path& rhs)
{
    return (lhs.getFullPath() != rhs.getFullPath());
}

bool operator!=(const Path& lhs, const std::string& rhs)
{
    return (lhs.getFullPath() != rhs);
}

bool operator!=(const std::string& lhs, const Path& rhs)
{
    return (lhs != rhs.getFullPath());
}

} // filesystem
} // extension
