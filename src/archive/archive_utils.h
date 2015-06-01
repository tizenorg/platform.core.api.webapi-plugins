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
 
#ifndef __TIZEN_ARCHIVE_ARCHIVE_UTILS_H__
#define __TIZEN_ARCHIVE_ARCHIVE_UTILS_H__

#include <string>

#include "common/platform_result.h"
#include "filesystem_file.h"
#include "archive_file.h"

namespace extension {
namespace archive {

std::string bytesToReadableString(const size_t num_bytes);
common::PlatformResult fileModeToString(FileMode fm, std::string* fm_str);
common::PlatformResult stringToFileMode(std::string fmString, FileMode* fm);

//extern Filesystem::FilePtr fileReferenceToFile(
//    JSContextRef context, JSValueRef fileReference);

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
void getBasePathAndName(const std::string& filepath,
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
std::string removeDuplicatedSlashesFromPath(const std::string& path);

/**
 * Return true if last character of string is '/' or '\'
 */
bool isDirectoryPath(const std::string& path);

/**
 * If path contains trailing '/' or '\' it will be removed.
 *
 * Example path:              | Result:
 * ---------------------------+------------------------------
 * "documents/someName/"      | "documents/someName"
 * "documents/yetAnotherName" | "documents/yetAnotherName"
 */
std::string removeTrailingDirectorySlashFromPath(const std::string& path);

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
std::string stripBasePathFromPath(const std::string& fullpath);

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
std::string getBasePathFromPath(const std::string& fullpath);

std::string getArchiveLogMessage(const int errorCode, const std::string &hint);

} //namespace archive
} //namespace extension

#endif // __TIZEN_ARCHIVE_ARCHIVE_UTILS_H__
