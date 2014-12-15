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

#ifndef __TIZEN_ARCHIVE_ZIP_H__
#define __TIZEN_ARCHIVE_ZIP_H__

#include <memory>
#include <string>

#include <zip.h>

#include "ArchiveCallbackData.h"

namespace DeviceAPI {
namespace Archive {


class ZipAddRequest;

class Zip;
typedef std::shared_ptr<Zip> ZipPtr;

class Zip
{
public:
    static ZipPtr createNew(const std::string& filename);
    static ZipPtr open(const std::string& filename);
    ~Zip();

    /**
     * When request has finished callback is set to NULL and is
     * deleted on main thread after calling all progress callbacks.
     * If exception is thrown please delete callback.
     */
    void addFile(AddProgressCallback*& callback);

    void close();

private:
    enum ZipOpenMode {
        ZOM_CREATE,
        ZOM_CREATEAFTER,
        ZOM_ADDINZIP
    };

    Zip(const std::string& filename, ZipOpenMode open_mode);

    static void generateZipFileInfo(const std::string& filename, zip_fileinfo& out_zi);

    std::string m_zipfile_name;
    zipFile m_zip;
    size_t m_default_buffer_size;
    bool m_is_open;


    friend class ZipAddRequest;
};

} //namespace Archive
} //namespace DeviceAPI

#endif // __TIZEN_ARCHIVE_ZIP_H__
