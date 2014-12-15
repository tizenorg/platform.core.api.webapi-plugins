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

#include "Zip.h"

#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <sys/stat.h>

#include <File.h>
#include <Logger.h>
#include <Path.h>
#include <PlatformException.h>

#include "ArchiveFile.h"
#include "ArchiveUtils.h"
#include "crypt.h"

#include "ZipAddRequest.h"

namespace DeviceAPI {

using namespace DeviceAPI::Common;

namespace Archive {

void Zip::generateZipFileInfo(const std::string& filename, zip_fileinfo& out_zi)
{
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

    struct tm* filedate = localtime(&tm_t);
    if(filedate) {
        out_zi.tmz_date.tm_sec  = filedate->tm_sec;
        out_zi.tmz_date.tm_min  = filedate->tm_min;
        out_zi.tmz_date.tm_hour = filedate->tm_hour;
        out_zi.tmz_date.tm_mday = filedate->tm_mday;
        out_zi.tmz_date.tm_mon  = filedate->tm_mon ;
        out_zi.tmz_date.tm_year = filedate->tm_year;
    }
}

Zip::Zip(const std::string& filename, ZipOpenMode open_mode) :
        m_zipfile_name(filename),
        m_zip(NULL),
        m_default_buffer_size(1024 * 1024)
{
    LOGD("Entered");

    int append_mode = APPEND_STATUS_CREATE;
    if(ZOM_CREATEAFTER == open_mode) {
        append_mode = APPEND_STATUS_CREATEAFTER;
    } else if(ZOM_ADDINZIP == open_mode) {
        append_mode = APPEND_STATUS_ADDINZIP;
    }
    LOGD("append_mode: %d", append_mode);

    m_zip = zipOpen(filename.c_str(), append_mode);
    if(!m_zip) {
        LOGE("zipOpen returned NULL!");
        throw UnknownException("Opening/creating zip file failed");
    }
    m_is_open = true;
}

Zip::~Zip()
{
    close();
}

void Zip::close()
{
    LOGD("Entered");
    if(!m_is_open) {
        LOGD("Already closed - exiting.");
        return;
    }

    int errclose = zipClose(m_zip, NULL);
    m_zip = NULL;

    if (errclose != ZIP_OK) {
        LOGE("ret: %d", errclose);
        throwArchiveException(errclose, "zipClose()");
    }
    m_is_open = false;
}

ZipPtr Zip::createNew(const std::string& filename)
{
    LOGD("Entered");
    return ZipPtr(new Zip(filename, ZOM_CREATE));
}

ZipPtr Zip::open(const std::string& filename)
{
    LOGD("Entered");
    return ZipPtr(new Zip(filename, ZOM_ADDINZIP));
}

void Zip::addFile(AddProgressCallback*& callback)
{
    LOGD("Entered");
    if(!callback) {
        LOGE("callback is NULL!");
        throw UnknownException("Could not add file(-s) to archive");
    }
    if(!m_is_open) {
        LOGE("Zip file not opened - exiting");
        throw UnknownException("Could not add file(-s) to archive - zip file closed");
    }

    ZipAddRequest::execute(*this, callback);
}

} //namespace Archive
} //namespace DeviceAPI
