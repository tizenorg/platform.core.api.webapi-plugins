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

#ifndef _FILESYSTEM_PATH_H_
#define _FILESYSTEM_PATH_H_

#include <string>
#include <vector>
#include <memory>

namespace extension {
namespace filesystem {

class Path;
typedef std::shared_ptr<Path> PathPtr;

class Path : public std::enable_shared_from_this<Path>
{
public:
    typedef char SeparatorType;
    static PathPtr create(const std::string& path);
    static Path::SeparatorType getSeparator();
    std::string getFullPath() const;
    std::string getPath() const;
    std::string getName() const;
    PathPtr append(const std::string& path);
    PathPtr append(const PathPtr& path);
    bool isAbsolute() const;
    PathPtr clone() const;

private:
    /**
     * Checks whether specified string is a valid path.
     * @param path String to verify.
     * @return True when string is a valid path, false otherwise.
     */
    static bool isValid(const std::string& str);
    Path();
    void reset(const std::string& str);
    static const SeparatorType m_pathSeparator; ///< Path separator.
    std::string m_fullPath; ///< Full path.
    std::string m_path;   ///< Base path.
    std::string m_name;   ///< Last part of the path.
};

PathPtr operator+(const Path& lhs, const Path& rhs);

PathPtr operator+(const Path& lhs, const std::string& rhs);

PathPtr operator+(const std::string& lhs, const Path& rhs);

bool operator==(const Path& lhs, const Path& rhs);

bool operator==(const Path& lhs, const std::string& rhs);

bool operator==(const std::string& lhs, const Path& rhs);

bool operator!=(const Path& lhs, const Path& rhs);

bool operator!=(const Path& lhs, const std::string& rhs);

bool operator!=(const std::string& lhs, const Path& rhs);


} // filesystem
} // extension

#endif /* _FILESYSTEM_PATH_H_ */
