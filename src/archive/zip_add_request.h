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

#ifndef __TIZEN_ARCHIVE_ZIP_ADD_REQUEST_H__
#define __TIZEN_ARCHIVE_ZIP_ADD_REQUEST_H__

#include <stdio.h>
#include <string>

#include "common/platform_result.h"
#include "filesystem_file.h"
#include "archive_callback_data.h"
#include "zip.h"

namespace extension {
namespace archive {

class ZipAddRequest
{
public:

    /**
     * When request has finished callback is set to NULL and is
     * deleted on main thread after calling all progress callbacks.
     * If exception is thrown please delete callback.
     */
    static PlatformResult execute(Zip& owner, AddProgressCallback*& callback);
    ~ZipAddRequest();

private:
    ZipAddRequest(Zip& owner, AddProgressCallback*& callback);
    PlatformResult run();

    void addNodeAndSubdirsToList(filesystem::NodePtr src_node,
            filesystem::NodeList& out_list_of_child_nodes);

    PlatformResult addEmptyDirectoryToZipArchive(std::string name_in_zip);
    PlatformResult addToZipArchive(filesystem::NodePtr src_file_node);

    std::string getNameInZipArchiveFor(filesystem::NodePtr node, bool strip);

    //-----------------------------------------------------------------------------
    //Input request variables
    Zip& m_owner;
    AddProgressCallback* m_callback;


    FILE* m_input_file;
    char* m_buffer;
    size_t m_buffer_size;


    unsigned long m_files_to_compress;
    unsigned long long m_bytes_to_compress;

    unsigned long m_files_compressed;
    unsigned long long m_bytes_compressed;

    filesystem::FilePtr m_root_src_file;
    filesystem::NodePtr m_root_src_file_node;
    std::string m_absoulte_path_to_extract;
    std::string m_destination_path_in_zip;

    unsigned int m_compression_level;

    bool m_new_file_in_zip_opened;
};

} //namespace archive
} //namespace extension

#endif // __TIZEN_ARCHIVE_ZIP_ADD_REQUEST_H__
