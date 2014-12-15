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

#ifndef __TIZEN_ARCHIVE_UNZIP_EXTRACT_REQUEST_H__
#define __TIZEN_ARCHIVE_UNZIP_EXTRACT_REQUEST_H__

#include <stdio.h>
#include <string>
#include "UnZip.h"

namespace DeviceAPI {
namespace Archive {

enum FilePathStatus {
    FPS_NOT_EXIST = 0,
    FPS_FILE = 1,
    FPS_DIRECTORY = 2
};

class UnZipExtractRequest
{
public:
    static void execute(UnZip& owner,
            const std::string& extract_path,
            const std::string& base_strip_path,
            BaseProgressCallback* callback);

    ~UnZipExtractRequest();

private:
    UnZipExtractRequest(UnZip& owner,
            const std::string& extract_path,
            const std::string& base_strip_path,
            BaseProgressCallback* callback);
    void run();

    void getCurrentFileInfo();

    void handleDirectoryEntry();

    void handleFileEntry();
    bool prepareOutputSubdirectory();

    //-----------------------------------------------------------------------------
    //Input request variables
    UnZip& m_owner;
    const std::string m_extract_path;
    const std::string m_base_strip_path;
    BaseProgressCallback* m_callback;

    //-----------------------------------------------------------------------------
    //Working variables
    FILE* m_output_file; //Used to write extracted file into output directory
    char* m_buffer; //Memory buffer passed between Minizip lib and fwrite function

    bool m_delete_output_file;
    bool m_close_unz_current_file;

    unz_file_info m_file_info; //Informations about current archive entry (from minizip)
    char m_filename_inzip[512]; //Name of archive file entry (from minizip)
    std::string m_output_filepath; //Extracted output file name with full path

    FilePathStatus m_new_dir_status;
    bool m_is_directory_entry; //Is this request for directory
    std::string m_new_dir_path;
};

} //namespace Archive
} //namespace DeviceAPI

#endif // __TIZEN_ARCHIVE_UNZIP_EXTRACT_REQUEST_H__
