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

#ifndef __TIZEN_ARCHIVE_UNZIP_H__
#define __TIZEN_ARCHIVE_UNZIP_H__

#include <memory>
#include <string>
#include <queue>

#include <unzip.h>

#include "ArchiveCallbackData.h"
#include "ArchiveFileEntry.h"

namespace DeviceAPI {
namespace Archive {

class UnZipExtractRequest;

typedef std::shared_ptr<std::vector<char>> CharVectorPtr;

class UnZip;
typedef std::shared_ptr<UnZip> UnZipPtr;

class UnZip
{
public:
    static UnZipPtr open(const std::string& filename);
    ~UnZip();

    ArchiveFileEntryPtrMapPtr listEntries(unsigned long *decompressedSize);

    /**
     * \brief Extract all files to output directory
     * \param callback which keep pointer to ArchiveFile.
     */
    void extractAllFilesTo(const std::string& extract_path,
            ExtractAllProgressCallback* callback);

    void extractTo(ExtractEntryProgressCallback* callback);

    void close();

private:
    UnZip(const std::string& filename);

    /**
     * \brief Extract current file (iterated with minizip library)
     * \param callback which keep pointer to ArchiveFile.
     */
    void extractCurrentFile(const std::string& extract_path,
            const std::string& base_strip_path,
            BaseProgressCallback* callback);

    static void extractItFunction(const std::string& file_name,
            unz_file_info& file_info,
            void* user_data);

    typedef void (*IterateFunction) (const std::string& file_name,
            unz_file_info& file_info,
            void* user_data);

    unsigned int IterateFilesInZip(unz_global_info& gi,
            const std::string& entry_name_in_zip,
            OperationCallbackData* callback,
            IterateFunction itfunc,
            void* user_data);

    void updateCallbackWithArchiveStatistics(ExtractAllProgressCallback* callback,
            unz_global_info& out_global_info,
            const std::string optional_filter = std::string());

    std::string m_zipfile_name;
    unzFile m_unzip;
    size_t m_default_buffer_size;
    bool m_is_open;

    friend class UnZipExtractRequest;
};

} //namespace Archive
} //namespace DeviceAPI

#endif // __TIZEN_ARCHIVE_ZIP_H__
