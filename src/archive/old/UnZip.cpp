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

#include "UnZip.h"

#include <cstdio>
#include <errno.h>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <sys/stat.h>

#include <File.h>
#include <GlobalContextManager.h>
#include <JSUtil.h>
#include <Logger.h>
#include <Node.h>
#include <Path.h>
#include <PlatformException.h>

#include "ArchiveUtils.h"
#include "UnZipExtractRequest.h"
#include "ArchiveFile.h"

namespace DeviceAPI {

using namespace DeviceAPI::Common;

namespace Archive {

UnZip::UnZip(const std::string& filename) :
        m_zipfile_name(filename),
        m_unzip(NULL),
        m_default_buffer_size(1024 * 1024)
{
    LOGD("Entered");

    m_unzip = unzOpen(filename.c_str());
    if(!m_unzip) {
        LOGE("unzOpen returned NULL : It means the file is invalid.");
        throw InvalidValuesException("Failed to open zip file");
    }
    m_is_open = true;
}

UnZip::~UnZip()
{
    close();
}

void UnZip::close()
{
    LOGD("Entered");
    if(!m_is_open) {
        LOGD("Unzip already closed - exiting");
        return;
    }

    int errclose = unzClose(m_unzip);
    m_unzip = NULL;

    if (errclose != UNZ_OK) {
        LOGE("ret: %d",errclose);
        throwArchiveException(errclose, "unzClose()");
    }
    m_is_open = false;
}

UnZipPtr UnZip::open(const std::string& filename)
{
    LOGD("Entered");
    return UnZipPtr(new UnZip(filename));
}

ArchiveFileEntryPtrMapPtr UnZip::listEntries(unsigned long *decompressedSize)
{
    if(!m_is_open) {
        LOGE("Failed to get list of entries - UnZip closed");
        throw UnknownException("Failed to get list of files in zip archive");
    }
    unz_global_info gi;
    int err = unzGetGlobalInfo (m_unzip, &gi);
    if (UNZ_OK != err) {
        LOGE("ret: %d",err);
        throwArchiveException(err, "unzGetGlobalInfo()");
    }

    char filename_inzip[512];
    unz_file_info file_info;

    ArchiveFileEntryPtrMapPtr map = ArchiveFileEntryPtrMapPtr(
            new ArchiveFileEntryPtrMap());

    unsigned long totalDecompressed = 0;

    err = unzGoToFirstFile(m_unzip);
    if (err != UNZ_OK) {
        LOGW("%s",getArchiveLogMessage(err, "unzGoToFirstFile()").c_str());
    }

    for (uLong i = 0; i < gi.number_entry; i++) {

        err = unzGetCurrentFileInfo(m_unzip, &file_info,
                filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err != UNZ_OK) {
            LOGE("ret: %d",err);
            throwArchiveException(err, "unzGetCurrentFileInfo()");
        }

        LOGD("file: %s | unc size: %d | comp size: %d", filename_inzip,
                file_info.uncompressed_size, file_info.compressed_size);

        ArchiveFileEntryPtr entry = ArchiveFileEntryPtr(new ArchiveFileEntry());
        entry->setName(filename_inzip);
        entry->setSize(file_info.uncompressed_size);
        entry->setCompressedSize(file_info.compressed_size);

        totalDecompressed += file_info.uncompressed_size;

        tm date;// = file_info.tmu_date;
        date.tm_sec = file_info.tmu_date.tm_sec;
        date.tm_min = file_info.tmu_date.tm_min;
        date.tm_hour = file_info.tmu_date.tm_hour;
        date.tm_mday = file_info.tmu_date.tm_mday;
        date.tm_mon = file_info.tmu_date.tm_mon;
        date.tm_year = file_info.tmu_date.tm_year - 1900;
        date.tm_wday = 0;
        date.tm_yday = 0;
        date.tm_isdst = 0;
        LOGD("%d, %d, %d, %d, %d, %d", date.tm_hour, date.tm_min, date.tm_sec, date.tm_mday, date.tm_mon, date.tm_year);
        entry->setModified(mktime(&date));

        map->insert(std::make_pair(filename_inzip, entry));

        if ( (i+1) < gi.number_entry) {

            err = unzGoToNextFile(m_unzip);
            if (UNZ_OK != err) {
                LOGE("ret: %d",err);
                throwArchiveException(err, "unzGoToNextFile()");
            }
        }
    }

    (*decompressedSize) = totalDecompressed;

    return map;
}

void UnZip::extractAllFilesTo(const std::string& extract_path,
        ExtractAllProgressCallback* callback)
{
    if(!m_is_open) {
        LOGE("Failed to extract files - UnZip closed");
        throw UnknownException("Failed to extract zip archive");
    }

    //
    // Calculate number of entries to extract and total number of bytes
    //
    unz_global_info gi;
    updateCallbackWithArchiveStatistics(callback, gi);

    //
    // Begin extracting entries
    //
    int err = unzGoToFirstFile(m_unzip);
    if (err != UNZ_OK) {
        LOGW("%s",getArchiveLogMessage(err, "unzGoToFirstFile()").c_str());
    }

    for (uLong i = 0; i < gi.number_entry; i++) {

        if (callback->isCanceled()) {
            LOGD("Operation cancelled");
            throw OperationCanceledException();
        }

        extractCurrentFile(extract_path, std::string(), callback);

        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(m_unzip);
            if (UNZ_OK != err) {
                LOGE("ret: %d",err);
                throwArchiveException(err, "unzGoToNextFile()");
            }
        }
    }

    callback->callSuccessCallbackOnMainThread();
    callback = NULL;
}

struct ExtractDataHolder
{
    UnZip* unzip;
    ExtractEntryProgressCallback* callback;
    std::string root_output_path;
};

void UnZip::extractTo(ExtractEntryProgressCallback* callback)
{
    if(!m_is_open) {
        LOGE("Extract archive file entry failed - UnZip closed");
        throw UnknownException("Extract archive file entry failed");
    }

    if(!callback) {
        LOGE("callback is NULL");
        throw UnknownException("Extract archive file entry failed");
    }

    if(!callback->getArchiveFileEntry()) {
        LOGE("callback->getArchiveFileEntry() is NULL");
        throw UnknownException("Extract archive file entry failed");
    }

    Filesystem::FilePtr out_dir = callback->getDirectory();
    if(!out_dir) {
        LOGE("Output directory is not valid");
        throw InvalidValuesException("Output directory is not correct");
    }

    Filesystem::NodePtr out_node = out_dir->getNode();
    if(!out_node) {
        LOGE("Output directory is not valid");
        throw InvalidValuesException("Output directory is not correct");
    }

    Filesystem::PathPtr out_path = out_node->getPath();
    if(!out_path) {
        LOGE("Output directory is not valid");
        throw InvalidValuesException("Output directory is not correct");
    }

    auto entry_name_in_zip = callback->getArchiveFileEntry()->getName();
    auto root_output_path = out_path->getFullPath();
    LOGD("Extract: [%s] to root output directory: [%s] (stripBasePath: [%s])",
            entry_name_in_zip.c_str(),
            root_output_path.c_str(),
            callback->getStripBasePath().c_str());

    //
    // Calculate number of entries to extract and total number of bytes
    //
    unz_global_info gi;
    updateCallbackWithArchiveStatistics(callback, gi, entry_name_in_zip);

    //
    // Begin extracting entries
    //

    ExtractDataHolder h;
    h.unzip = this;
    h.callback = callback;
    h.root_output_path = root_output_path;

    // this loop call internally progress callbacks
    IterateFilesInZip(gi, entry_name_in_zip, callback, extractItFunction, &h);

    // after finish extracting success callback will be called
    callback->callSuccessCallbackOnMainThread();
    callback = NULL;
}

void UnZip::extractItFunction(const std::string& file_name, unz_file_info& file_info,
        void* user_data)
{
    ExtractDataHolder* h = static_cast<ExtractDataHolder*>(user_data);
    if(!h) {
        LOGE("ExtractDataHolder is NULL!");
        throw UnknownException("Could not list content of zip archive");
    }

    h->unzip->extractCurrentFile(h->root_output_path,
            h->callback->getStripBasePath(),
            h->callback);
}

unsigned int UnZip::IterateFilesInZip(unz_global_info& gi,
        const std::string& entry_name_in_zip,
        OperationCallbackData* callback,
        UnZip::IterateFunction itfunc,
        void* user_data)
{
    int err = unzGoToFirstFile(m_unzip);
    if (UNZ_OK != err) {
        LOGW("%s",getArchiveLogMessage(err, "unzGoToFirstFile()").c_str());
    }

    unsigned int num_file_or_folder_matched = 0;
    const bool is_directory = isDirectoryPath(entry_name_in_zip);

    unz_file_info cur_file_info;
    char tmp_fname[512];

    for (uLong i = 0; i < gi.number_entry; i++) {

        if (callback->isCanceled()) {
            LOGD("Operation cancelled");
            throw OperationCanceledException();
        }

        err = unzGetCurrentFileInfo(m_unzip, &cur_file_info,
                tmp_fname, sizeof(tmp_fname), NULL, 0, NULL, 0);
        if (UNZ_OK != err) {
            LOGE("ret: %d",err);
            throwArchiveException(err, "unzGetCurrentFileInfo()");
        }

        const std::string cur_filename_in_zip(tmp_fname);
        bool match = true;

        if(!entry_name_in_zip.empty()) {
            if(is_directory) {
                //If entry_name_in_zip is pointing at directory we need to check each entry in
                //zip if its path starts with entry_name_in_zip
                match = (0 == cur_filename_in_zip.find(entry_name_in_zip));
            } else {
                //If entry name points to file we only extract entry with matching name
                match = (cur_filename_in_zip == entry_name_in_zip);
            }
        }

        if(match) {
            itfunc(cur_filename_in_zip, cur_file_info, user_data);
            ++num_file_or_folder_matched;
        }

        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(m_unzip);
            if (UNZ_OK != err) {
                LOGE("ret: %d",err);
                throwArchiveException(err, "unzGoToNextFile()");
            }
        }
    }

    return num_file_or_folder_matched;
}

void UnZip::extractCurrentFile(const std::string& extract_path,
        const std::string& base_strip_path,
        BaseProgressCallback* callback)
{
    LOGD("Entered");

    if (callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    LOGD("extract_path: [%s] base_strip_path: [%s] ", extract_path.c_str(),
            base_strip_path.c_str());
    UnZipExtractRequest::execute(*this, extract_path, base_strip_path, callback);
}

struct ArchiveStatistics
{
    ArchiveStatistics() : uncompressed_size(0),
            number_of_files(0),
            number_of_folders(0) {}

    unsigned long uncompressed_size;
    unsigned long number_of_files;
    unsigned long number_of_folders;
};

void generateArchiveStatistics(const std::string& file_name, unz_file_info& file_info,
         void* user_data)
{
    if(user_data) {
        ArchiveStatistics* astats = static_cast<ArchiveStatistics*>(user_data);
        astats->uncompressed_size += file_info.uncompressed_size;

        if(isDirectoryPath(file_name)) {
            astats->number_of_folders += 1;
        }
        else {
            astats->number_of_files += 1;
        }
    }
}

void UnZip::updateCallbackWithArchiveStatistics(ExtractAllProgressCallback* callback,
        unz_global_info& out_global_info,
        const std::string optional_filter)
{
    int err = unzGetGlobalInfo(m_unzip, &out_global_info);
    if (UNZ_OK != err) {
        LOGE("ret: %d",err);
        throwArchiveException(err, "unzGetGlobalInfo()");
    }

    ArchiveStatistics astats;
    const auto num_matched = IterateFilesInZip(out_global_info, optional_filter,
            callback, generateArchiveStatistics, &astats);
    if(0 == num_matched) {
        LOGE("No matching file/directory: [%s] has been found in zip archive",
                optional_filter.c_str());
        LOGE("Throwing NotFoundException - Could not extract file from archive");
        throw NotFoundException("Could not extract file from archive");
    }

    callback->setExpectedDecompressedSize(astats.uncompressed_size);
    LOGD("Expected uncompressed size: %s",
            bytesToReadableString(astats.uncompressed_size).c_str());

    callback->setNumberOfFilesToExtract(astats.number_of_files);
    LOGD("Number entries to extract: files: %d folders: %d", astats.number_of_files,
            astats.number_of_folders);
}

} //namespace Archive
} //namespace DeviceAPI
