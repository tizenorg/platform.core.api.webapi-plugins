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

#include "zip.h"

#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <sys/stat.h>

#include "common/logger.h"
#include "common/platform_result.h"
#include "filesystem_file.h"
#include "archive_file.h"
#include "archive_utils.h"
#include "crypt.h"
#include "zip_add_request.h"

namespace extension {
namespace archive {

using namespace common;

void Zip::generateZipFileInfo(const std::string& filename, zip_fileinfo& out_zi)
{
    LoggerD("Enter");

    memset(&out_zi, 0, sizeof(zip_fileinfo));

    time_t tm_t = 0;
    if (filename != "-") {
        std::string name = filename;
        if (name[name.length() - 1] == '/') {
            name[name.length() - 1] = '\0';
        }

        struct stat s;
        // not all systems allow stat'ing a file with / appended
        if (stat(name.c_str(), &s) == 0) {
            tm_t = s.st_mtime;
        }
    }

    struct tm filedate = {0};
    tzset();
    if (nullptr != localtime_r(&tm_t, &filedate)) {
        out_zi.tmz_date.tm_sec  = filedate.tm_sec;
        out_zi.tmz_date.tm_min  = filedate.tm_min;
        out_zi.tmz_date.tm_hour = filedate.tm_hour;
        out_zi.tmz_date.tm_mday = filedate.tm_mday;
        out_zi.tmz_date.tm_mon  = filedate.tm_mon ;
        out_zi.tmz_date.tm_year = filedate.tm_year;
    }
}

Zip::Zip(const std::string& filename, ZipOpenMode open_mode) :
        m_zipfile_name(filename),
        m_zip(NULL),
        m_default_buffer_size(1024 * 1024)
{
    LoggerD("Entered");

    int append_mode = APPEND_STATUS_CREATE;
    if(ZOM_CREATEAFTER == open_mode) {
        append_mode = APPEND_STATUS_CREATEAFTER;
    } else if(ZOM_ADDINZIP == open_mode) {
        append_mode = APPEND_STATUS_ADDINZIP;
    }
    LoggerD("append_mode: %d", append_mode);

    m_zip = zipOpen(filename.c_str(), append_mode);
}

Zip::~Zip()
{
    LoggerD("Enter");

    close();
}

PlatformResult Zip::close()
{
    LoggerD("Entered");
    if(!m_is_open) {
        LoggerD("Already closed - exiting.");
        return PlatformResult(ErrorCode::NO_ERROR);
    }

    int errclose = zipClose(m_zip, NULL);
    m_zip = NULL;

    if (errclose != ZIP_OK) {
        LoggerE("ret: %d", errclose);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(errclose, "zipClose()"));
    }
    m_is_open = false;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Zip::createNew(const std::string& filename, ZipPtr* out_zip)
{
    LoggerD("Entered");
    ZipPtr zip = ZipPtr(new Zip(filename, ZOM_CREATE));
    if(!zip->m_zip) {
        LoggerE("zipOpen returned NULL!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Opening/creating zip file failed");
    }
    zip->m_is_open = true;
    *out_zip = zip;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Zip::open(const std::string& filename, ZipPtr* out_zip)
{
    LoggerD("Entered");
    ZipPtr zip = ZipPtr(new Zip(filename, ZOM_ADDINZIP));
    if(!zip->m_zip) {
        LoggerE("zipOpen returned NULL!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Opening/creating zip file failed");
    }
    zip->m_is_open = true;
    *out_zip = zip;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Zip::addFile(AddProgressCallback*& callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("callback is NULL!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add file(-s) to archive");
    }
    if(!m_is_open) {
        LoggerE("Zip file not opened - exiting");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add file(-s) to archive - zip file closed");
    }

    return ZipAddRequest::execute(*this, callback);
}

} //namespace archive
} //namespace extension
