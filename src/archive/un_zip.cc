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

#include "un_zip.h"

#include <cstdio>
#include <errno.h>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <sys/stat.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "filesystem_file.h"

#include "archive_file.h"
#include "archive_utils.h"
#include "un_zip_extract_request.h"

namespace extension {
namespace archive {

using namespace common;

UnZip::UnZip(const std::string& filename) :
        m_zipfile_name(filename),
        m_unzip(NULL),
        m_default_buffer_size(1024 * 1024)
{
    LoggerD("Entered");
    m_unzip = unzOpen(filename.c_str());
}

UnZip::~UnZip()
{
    LoggerD("Enter");
    close();
}

PlatformResult UnZip::close()
{
    LoggerD("Entered");
    if(!m_is_open) {
        LoggerD("Unzip already closed - exiting");
        return PlatformResult(ErrorCode::NO_ERROR);
    }

    int errclose = unzClose(m_unzip);
    m_unzip = NULL;

    if (errclose != UNZ_OK) {
        LoggerE("ret: %d",errclose);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(errclose, "unzClose()"));
    }
    m_is_open = false;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::open(const std::string& filename, UnZipPtr* out_unzip)
{
    LoggerD("Entered");
    UnZipPtr unzip = UnZipPtr(new UnZip(filename));

    if(!unzip->m_unzip) {
        LoggerE("unzOpen returned NULL : It means the file is invalid.");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Failed to open zip file");
    }
    unzip->m_is_open = true;
    *out_unzip = unzip;
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::listEntries(unsigned long *decompressedSize, ArchiveFileEntryPtrMapPtr* out_map)
{
    LoggerD("Enter");
    if(!m_is_open) {
        LoggerE("Failed to get list of entries - UnZip closed");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get list of files in zip archive");
    }
    unz_global_info gi;
    int err = unzGetGlobalInfo (m_unzip, &gi);
    if (UNZ_OK != err) {
        LoggerE("ret: %d",err);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGetGlobalInfo()"));
    }

    char filename_inzip[512];
    unz_file_info file_info;

    ArchiveFileEntryPtrMapPtr map = ArchiveFileEntryPtrMapPtr(new ArchiveFileEntryPtrMap());

    unsigned long totalDecompressed = 0;

    err = unzGoToFirstFile(m_unzip);
    if (err != UNZ_OK) {
        LoggerW("%s",getArchiveLogMessage(err, "unzGoToFirstFile()").c_str());
    }

    for (uLong i = 0; i < gi.number_entry; i++) {

        err = unzGetCurrentFileInfo(m_unzip, &file_info,
                filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err != UNZ_OK) {
            LoggerE("ret: %d",err);
            return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGetCurrentFileInfo()"));
        }

        LoggerD("file: %s | unc size: %d | comp size: %d", filename_inzip,
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
        LoggerD("%d, %d, %d, %d, %d, %d", date.tm_hour, date.tm_min, date.tm_sec, date.tm_mday, date.tm_mon, date.tm_year);
        entry->setModified(mktime(&date));

        map->insert(std::make_pair(filename_inzip, entry));

        if ( (i+1) < gi.number_entry) {

            err = unzGoToNextFile(m_unzip);
            if (UNZ_OK != err) {
                LoggerE("ret: %d",err);
                return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGoToNextFile()"));
            }
        }
    }

    (*decompressedSize) = totalDecompressed;

    *out_map = map;

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::extractAllFilesTo(const std::string& extract_path,
                                  ExtractAllProgressCallback* callback)
{
    LoggerD("Enter");
    if(!m_is_open) {
        LoggerE("Failed to extract files - UnZip closed");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to extract zip archive");
    }

    //
    // Calculate number of entries to extract and total number of bytes
    //
    unz_global_info gi;
    PlatformResult result = updateCallbackWithArchiveStatistics(callback, gi);
    if ( result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Error: %s", result.message().c_str());
        return result;
    }

    //
    // Begin extracting entries
    //
    int err = unzGoToFirstFile(m_unzip);
    if (err != UNZ_OK) {
        LoggerE("%s",getArchiveLogMessage(err, "unzGoToFirstFile()").c_str());
    }

    for (uLong i = 0; i < gi.number_entry; i++) {

        if (callback->isCanceled()) {
            LoggerD("Operation cancelled");
            return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
        }

        result = extractCurrentFile(extract_path, std::string(), callback);
        if ( result.error_code() != ErrorCode::NO_ERROR) {
            LoggerE("Fail: extractCurrentFile()");
            return result;
        }

        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(m_unzip);
            if (UNZ_OK != err) {
                LoggerE("ret: %d",err);
                return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGoToNextFile()"));
            }
        }
    }

    callback->callSuccessCallbackOnMainThread();
    callback = NULL;

    return PlatformResult(ErrorCode::NO_ERROR);
}

struct ExtractDataHolder
{
    UnZip* unzip;
    ExtractEntryProgressCallback* callback;
    std::string root_output_path;
};

PlatformResult UnZip::extractTo(ExtractEntryProgressCallback* callback)
{
    LoggerD("Enter");
    if(!m_is_open) {
        LoggerE("Extract archive file entry failed - UnZip closed");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Extract archive file entry failed");
    }

    if(!callback) {
        LoggerE("callback is NULL");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Extract archive file entry failed");
    }

    if(!callback->getArchiveFileEntry()) {
        LoggerE("callback->getArchiveFileEntry() is NULL");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Extract archive file entry failed");
    }

    filesystem::FilePtr out_dir = callback->getDirectory();
    if(!out_dir) {
        LoggerE("Output directory is not valid");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Output directory is not correct");
    }

    NodePtr out_node = out_dir->getNode();
    if(!out_node) {
        LoggerE("Output directory is not valid");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Output directory is not correct");
    }

    PathPtr out_path = out_node->getPath();
    if(!out_path) {
        LoggerE("Output directory is not valid");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Output directory is not correct");
    }

    auto entry_name_in_zip = callback->getArchiveFileEntry()->getName();
    auto root_output_path = out_dir->getNode()->getPath()->getFullPath();
    LoggerD("Extract: [%s] to root output directory: [%s] (stripBasePath: [%s])",
            entry_name_in_zip.c_str(),
            root_output_path.c_str(),
            callback->getStripBasePath().c_str());


    //
    // Calculate number of entries to extract and total number of bytes
    //
    unz_global_info gi;
    PlatformResult result = updateCallbackWithArchiveStatistics(callback, gi, entry_name_in_zip);
    if ( result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail: updateCallbackWithArchiveStatistics()");
        return result;
    }

    //
    // Begin extracting entries
    //

    ExtractDataHolder h;
    h.unzip = this;
    h.callback = callback;
    h.root_output_path = root_output_path;

    // this loop call internally progress callbacks
    unsigned int matched;
    result = IterateFilesInZip(gi, entry_name_in_zip, callback, extractItFunction, matched, &h);
    if ( result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Fail: IterateFilesInZip()");
        return result;
    }

    // after finish extracting success callback will be called
    callback->callSuccessCallbackOnMainThread();
    callback = NULL;

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::extractItFunction(const std::string& file_name, unz_file_info& file_info,
                                       void* user_data)
{
    LoggerD("Enter");
    ExtractDataHolder* h = static_cast<ExtractDataHolder*>(user_data);
    if(!h) {
        LoggerE("ExtractDataHolder is NULL!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not list content of zip archive");
    }

    PlatformResult result = h->unzip->extractCurrentFile(h->root_output_path,
                                          h->callback->getStripBasePath(),
                                          h->callback);
    if ( result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Error: %s", result.message().c_str());
        return result;
    }
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::IterateFilesInZip(unz_global_info& gi,
        const std::string& entry_name_in_zip,
        OperationCallbackData* callback,
        UnZip::IterateFunction itfunc,
        unsigned int& num_file_or_folder_matched,
        void* user_data)
{
    LoggerD("Enter");
    int err = unzGoToFirstFile(m_unzip);
    if (UNZ_OK != err) {
        LoggerW("%s",getArchiveLogMessage(err, "unzGoToFirstFile()").c_str());
    }

    num_file_or_folder_matched = 0;
    const bool is_directory = isDirectoryPath(entry_name_in_zip);

    unz_file_info cur_file_info;
    char tmp_fname[512];

    for (uLong i = 0; i < gi.number_entry; i++) {

        if (callback->isCanceled()) {
            LoggerD("Operation cancelled");
            return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
        }

        err = unzGetCurrentFileInfo(m_unzip, &cur_file_info,
                tmp_fname, sizeof(tmp_fname), NULL, 0, NULL, 0);
        if (UNZ_OK != err) {
            LoggerE("ret: %d",err);
            return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGetCurrentFileInfo()"));
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
            PlatformResult result = itfunc(cur_filename_in_zip, cur_file_info, user_data);
            if ( result.error_code() != ErrorCode::NO_ERROR) {
                LoggerE("Error: %s", result.message().c_str());
                return result;
            }
            ++num_file_or_folder_matched;
        }

        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(m_unzip);
            if (UNZ_OK != err) {
                LoggerE("ret: %d",err);
                return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGoToNextFile()"));
            }
        }
    }

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::extractCurrentFile(const std::string& extract_path,
        const std::string& base_strip_path,
        BaseProgressCallback* callback)
{
    LoggerD("Entered");

    if (callback->isCanceled()) {
        LoggerD("Operation cancelled");
        return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
    }

    LoggerD("extract_path: [%s] base_strip_path: [%s] ", extract_path.c_str(),
            base_strip_path.c_str());
    return UnZipExtractRequest::execute(*this, extract_path, base_strip_path, callback);
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

PlatformResult generateArchiveStatistics(const std::string& file_name, unz_file_info& file_info,
         void* user_data)
{
    LoggerD("Enter");
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
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UnZip::updateCallbackWithArchiveStatistics(ExtractAllProgressCallback* callback,
        unz_global_info& out_global_info,
        const std::string& optional_filter)
{
    LoggerD("Enter");
    int err = unzGetGlobalInfo(m_unzip, &out_global_info);
    if (UNZ_OK != err) {
        LoggerE("ret: %d",err);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "unzGetGlobalInfo()"));
    }

    ArchiveStatistics astats;
    unsigned int num_matched;

    PlatformResult result = IterateFilesInZip(out_global_info, optional_filter,
            callback, generateArchiveStatistics, num_matched, &astats);
    if ( result.error_code() != ErrorCode::NO_ERROR) {
        LoggerE("Error: %s", result.message().c_str());
        return result;
    }
    if(0 == num_matched) {
        LoggerE("No matching file/directory: [%s] has been found in zip archive",
                optional_filter.c_str());
        LoggerE("Throwing NotFoundException - Could not extract file from archive");
        return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Could not extract file from archive");
    }

    callback->setExpectedDecompressedSize(astats.uncompressed_size);
    LoggerD("Expected uncompressed size: %s",
            bytesToReadableString(astats.uncompressed_size).c_str());

    callback->setNumberOfFilesToExtract(astats.number_of_files);
    LoggerD("Number entries to extract: files: %d folders: %d", astats.number_of_files,
            astats.number_of_folders);

    return PlatformResult(ErrorCode::NO_ERROR);
}

} //namespace archive
} //namespace extension
