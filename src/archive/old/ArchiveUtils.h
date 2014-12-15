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

#ifndef __TIZEN_ARCHIVE_ARCHIVE_UTILS_H__
#define __TIZEN_ARCHIVE_ARCHIVE_UTILS_H__

#include <JavaScriptCore/JavaScript.h>
#include <File.h>
#include <PlatformException.h>

#include <string>
#include "ArchiveFile.h"

namespace DeviceAPI {
namespace Archive {

extern std::string bytesToReadableString(const size_t num_bytes);
extern std::string fileModeToString(FileMode fm);
extern FileMode stringToFileMode(std::string fmString);
extern Filesystem::FilePtr fileReferenceToFile(
    JSContextRef context, JSValueRef fileReference);

/**
 * Gets base path and name from full path string, example cases:
 * full path             |  base path        | name
 * ----------------------+-------------------+------------------
 * ""                    | ""                | ""
 * "/opt/usr/media/TEST" | "/opt/usr/media/" | "TEST"
 * "/opt/usr/media/TEST/"| "/opt/usr/media/" | "TEST"
 * "opt/usr/media/TEST"  | "opt/usr/media/"  | "TEST"
 * "opt/usr/media/TEST/" | "opt/usr/media/"  | "TEST"
 * "TEST"                | ""                | "TEST"
 * "TEST/"               | ""                | "TEST"
 * "/TEST/"              | "/"               | "TEST"
 */
extern void getBasePathAndName(const std::string& filepath,
        std::string& out_basepath,
        std::string& out_name);

/**
 * If path contains duplicated slashes they will be removed
 *
 * Note this function does not fix slash style '/', '\'
 * (Linux/Windows)
 *
 * Example path:              | Result:
 * ---------------------------+------------------------------
 * "my/a//b/c/d/e"            | "my/a/b/c/d/e"
 * "add\ff\\\g"               | "add\f\g"
 * "first//second\\a\/"       | "first/second\a"  (mixed / with \)
 */
extern std::string removeDuplicatedSlashesFromPath(const std::string& path);

/**
 * Return true if last character of string is '/' or '\'
 */
extern bool isDirectoryPath(const std::string& path);

/**
 * If path contains trailing '/' or '\' it will be removed.
 *
 * Example path:              | Result:
 * ---------------------------+------------------------------
 * "documents/someName/"      | "documents/someName"
 * "documents/yetAnotherName" | "documents/yetAnotherName"
 */
extern std::string removeTrailingDirectorySlashFromPath(const std::string& path);

/**
 * Returns FILE name without leading base path:
 *
 * Example path:              | Result:
 * ---------------------------+------------------------------
 * "documents/ABC/a.txt"      | "a.txt"
 * "documents/A/X/Y/Z/my.log" | "my.log"
 *
 * "documents/A/B/C/"         | "" (fullpath is directory)
 * "a.txt"                    | "a.txt"
 * "A/"                       | "" (fullpath is directory)
 */
extern std::string stripBasePathFromPath(const std::string& fullpath);

/**
 * Returns path without last directory or file part
 *
 * Example path:              | Result:               | Extracted ending:
 * ---------------------------+-----------------------+--------------
 * "documents/ABC/a.txt"      | "documents/ABC/"      | "a.txt"
 * "documents/A/X/Y/Z/my.log" | "documents/A/X/Y/Z/"  | "my.log"
 *
 * "documents/A/B/C/"         | "documents/A/B/"      | "C/"
 * "a.txt"                    | ""                    | "a.txt"
 * "A/"                       | ""                    | "A/"
 */
extern std::string getBasePathFromPath(const std::string& fullpath);

extern std::string getArchiveLogMessage(const int errorCode, const std::string &hint);

template <class T = DeviceAPI::Common::UnknownException>
void throwArchiveException(const int errorCode, const std::string &hint)
{
    std::string log = getArchiveLogMessage(errorCode, hint);
    LOGE("%s", log.c_str());
    throw T(log.c_str());
}

} //namespace Archive
} //namespace DeviceAPI

#endif // __TIZEN_ARCHIVE_ARCHIVE_UTILS_H__
