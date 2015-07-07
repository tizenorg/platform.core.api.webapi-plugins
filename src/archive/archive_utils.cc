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
 
#include "archive_utils.h"
#include <sstream>
#include <iomanip>
#include "common/logger.h"

//TODO:
//#include <FilesystemExternalUtils.h>

using namespace common;

namespace extension {
namespace archive {

using namespace filesystem;

std::string bytesToReadableString(const size_t num_bytes)
{
    LoggerD("Enter");
    std::stringstream ss;
    static const size_t one_mb = 1024 * 1024;
    static const size_t one_kb = 1024;
    ss << std::setprecision(2) << std::fixed;

    if(num_bytes >= one_mb) {
        ss << (float)num_bytes / one_mb << " MB";
    } else if(num_bytes >= one_kb) {
        ss << (float)num_bytes / one_kb << " KB";
    } else {
        ss << num_bytes << " B";
    }

    return ss.str();
}

PlatformResult fileModeToString(FileMode fm, std::string* fm_str)
{
    LoggerD("Enter");
    switch(fm) {
        case FileMode::READ:
            *fm_str = "r";
            break;
        case FileMode::WRITE:
            *fm_str = "w";
            break;
        case FileMode::READ_WRITE:
            *fm_str = "rw";
            break;
        case FileMode::ADD:
            *fm_str = "a";
            break;
        default:
            LoggerE("Unknown file mode");
            return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown file mode");
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult stringToFileMode(std::string fmString, FileMode* fm)
{
    LoggerD("Enter");
    if (!fmString.compare("r")) {
        *fm = FileMode::READ;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    else if (!fmString.compare("w")) {
        *fm = FileMode::WRITE;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    else if(!fmString.compare("rw")) {
        *fm = FileMode::READ_WRITE;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    else if(!fmString.compare("a")) {
        *fm = FileMode::ADD;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    // In widl it's "TypeMismatchError" so this exception used
    // instead of InvalidValues
    LoggerE("Invalid FileMode");
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Invalid FileMode");
}

// FilePtr fileReferenceToFile(JSContextRef context, JSValueRef fileReference)
// {
//     auto g_ctx = GlobalContextManager::getInstance()->getGlobalContext(context);
//
//     FilePtr file_ptr;
//     try {
//         file_ptr = JSFile::getPrivateObject(context, fileReference);
//     } catch (const TypeMismatchException &tme) {
//         LOGD("Use virtual path.");
//         std::string virtual_path =
//             JSUtil::JSValueToString(context, fileReference);
//         if (!External::isVirtualPath(virtual_path)) {
//             LOGE("FileReference can be File object or a virtual path");
//             throw TypeMismatchException(
//                 "FileReference can be File object or a virtual path");
//         }
//         std::string string_path =
//             External::fromVirtualPath(virtual_path, g_ctx);
//         LOGD("Path: %s", string_path.c_str());
//
//         PathPtr path = Path::create(string_path);
//         NodePtr node_ptr = Node::resolve(path);
//         file_ptr = FilePtr(new File(node_ptr, File::PermissionList()));
//     }
//
//     return file_ptr;
// }

void getBasePathAndName(const std::string& filepath,
        std::string& out_basepath,
        std::string& out_name)
{
    LoggerD("Enter");
    const size_t filepath_len = filepath.length();

    size_t name_end_index = filepath_len;
    size_t name_start_index = 0;

    for(int i = static_cast<int>(filepath_len) - 1; i >= 0; --i) {
        const char& cur = filepath[i];
        if(cur == '/' || cur == '\\') {
            if((static_cast<int>(filepath_len)-1) == i) {
                name_end_index = static_cast<std::size_t>(i);
            } else {
                name_start_index = static_cast<std::size_t>(i) + 1;
                out_name = filepath.substr(name_start_index,
                    name_end_index - name_start_index);

                out_basepath = filepath.substr(0, name_start_index);
                return;
            }
        }
    }

    // \ / is not found
    out_basepath = "";
    out_name = filepath.substr(0, name_end_index);
}

std::string removeDuplicatedSlashesFromPath(const std::string& path)
{
    LoggerD("Enter");
    const size_t path_len = path.length();

    std::string out;
    out.reserve(path_len);

    bool prev_is_dir = false;
    for(size_t i = 0; i < path_len; ++i) {
        const char& cur = path[i];
        if(cur == '/' || cur == '\\') {
            if(!prev_is_dir) {
                out += cur;
            }
            prev_is_dir = true;
        } else {
            prev_is_dir = false;
            out += cur;
        }
    }

    return out;
}

bool isDirectoryPath(const std::string& path)
{
    LoggerD("Enter");
    if(path.empty()) {
        return false;
    }

    const char last_char = path[path.length() - 1];
    return last_char ==  '\\' || last_char == '/';
}

std::string removeTrailingDirectorySlashFromPath(const std::string& path)
{
    LoggerD("Enter");
    if(!isDirectoryPath(path)) {
        return path;
    }

    return path.substr(0, path.length() - 1);
}

std::string stripBasePathFromPath(const std::string& fullpath)
{
    LoggerD("Enter");
    const size_t location = fullpath.find_last_of("/\\");
    if(std::string::npos == location) {
        return fullpath;
    }

    return fullpath.substr(location + 1);
}

namespace{
static std::string errErrno = "ERRNO";
static std::string errEndOfListOfFile = "End list of file";
static std::string errParamater = "Invalid parameter";
static std::string errBadFile = "Incorrect file";
static std::string errInternal = "Internal error";
static std::string errCRC = "CRC error";
static std::string errUnknown = "Unknown error";

const std::string& getArchiveErrorMessage(int errorCode)
{
    LoggerD("Enter");
    /**
     * All errors are defined in minizip library in files:
     * zip.h and unzip.h
     */
    switch (errorCode) {
        // ZIP_ERRNO & UNZ_ERRNO both value Z_ERRNO
        case ZIP_ERRNO:
            return errErrno;
        // UNZ_END_OF_LIST_OF_FILE both value -100
        case UNZ_END_OF_LIST_OF_FILE:
            return errEndOfListOfFile;
        // ZIP_PARAMERROR & UNZ_PARAMERROR both value -102
        case ZIP_PARAMERROR:
            return errParamater;
        // ZIP_BADZIPFILE & UNZ_BADZIPFILE both value -103
        case ZIP_BADZIPFILE:
            return errBadFile;
        // ZIP_INTERNALERROR & UNZ_INTERNALERROR bot value -104
        case ZIP_INTERNALERROR:
            return errInternal;
        // UNZ_CRCERROR -105
        case UNZ_CRCERROR:
            return errCRC;
        default:
            return errUnknown;
    }
}
}

std::string getBasePathFromPath(const std::string& fullpath)
{
    LoggerD("Enter");
    const std::string tmp_path = removeTrailingDirectorySlashFromPath(fullpath);
    const size_t location = tmp_path.find_last_of("/\\");
    if(std::string::npos == location) {
        return std::string();
    }

    return tmp_path.substr(0, location + 1);
}

std::string getArchiveLogMessage(const int errorCode, const std::string &hint)
{
    LoggerD("Enter");
    std::stringstream ss;
    ss << "Failed " << hint << " : " << getArchiveErrorMessage(errorCode) << ", " << errorCode;
    return std::string(ss.str());
}

} //namespace archive
} //namespace extension
