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

#include "zip_add_request.h"
#include "common/logger.h"
#include "archive_file.h"
#include "archive_file_entry.h"
#include "archive_utils.h"

using namespace common;

namespace extension {
namespace archive {

ZipAddRequest::ZipAddRequest(Zip& owner, AddProgressCallback*& callback) :
        m_owner(owner),
        m_callback(callback),
        m_input_file(NULL),
        m_buffer(NULL),
        m_buffer_size(m_owner.m_default_buffer_size),
        m_files_to_compress(0),
        m_bytes_to_compress(0),
        m_files_compressed(0),
        m_bytes_compressed(0),
        m_compression_level(0),
        m_new_file_in_zip_opened(false)
{
    LoggerD("Enter");

}

ZipAddRequest::~ZipAddRequest()
{
    LoggerD("Enter");
    if(m_input_file) {
        fclose(m_input_file);
        m_input_file = NULL;
    }

    delete [] m_buffer;
    m_buffer = NULL;


    if(m_new_file_in_zip_opened) {
        int err = zipCloseFileInZip(m_owner.m_zip);
        if (ZIP_OK != err) {
            LoggerE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
        }
    }
}

PlatformResult ZipAddRequest::execute(Zip& owner, AddProgressCallback*& callback)
{
    LoggerD("Enter");
    ZipAddRequest req(owner, callback);
    return req.run();
}

PlatformResult ZipAddRequest::run()
{
    LoggerD("Enter");
    if(!m_callback) {
        LoggerE("m_callback is NULL");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add file(-s) to archive");
    }

    if(!m_callback->getFileEntry()) {
        LoggerE("m_callback->getFileEntry() is NULL");
        return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Provided ArchiveFileEntry is not correct");
    }

    if(m_callback->isCanceled()) {
        LoggerD("Operation cancelled");
        return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
    }

    m_compression_level = m_callback->getFileEntry()->getCompressionLevel();
    m_root_src_file = m_callback->getFileEntry()->getFile();
    m_root_src_file_node = m_root_src_file->getNode();

    //We just need read permission to list files in subdirectories
    LoggerW("STUB Not setting PERM_READ permissions");
    //m_root_src_file_node->setPermissions(filesystem::PERM_READ);

    std::string src_basepath, src_name;
    getBasePathAndName(m_root_src_file_node->getPath()->getFullPath(), src_basepath,
            src_name);

    m_absoulte_path_to_extract = src_basepath;
    LoggerD("m_absoulte_path_to_extract: [%s]", m_absoulte_path_to_extract.c_str());

    m_destination_path_in_zip = removeDuplicatedSlashesFromPath(
            m_callback->getFileEntry()->getDestination());

    // If we have destination set then we need to create all folders and sub folders
    // inside this zip archive.
    //
    PlatformResult result(ErrorCode::NO_ERROR);
    if(m_destination_path_in_zip.length() > 0) {
        LoggerD("destination is: [%s]", m_destination_path_in_zip.c_str());

        for(size_t i = 0; i < m_destination_path_in_zip.length(); ++i) {
            const char cur_char = m_destination_path_in_zip[i];

            if(((cur_char == '/' || cur_char == '\\') && i > 0 ) ||
                (i == m_destination_path_in_zip.length() - 1)) {

                //Extract left side with '/':
                const std::string new_dir = m_destination_path_in_zip.substr(0, i + 1);

                LoggerD("Adding empty directory: [%s] to archive", new_dir.c_str());
                result = addEmptyDirectoryToZipArchive(new_dir);
                if (result.error_code() != ErrorCode::NO_ERROR) {
                    return result;
                }
            }
        }
    }

    // Generate list of files and all subdirectories
    //
    filesystem::NodeList all_sub_nodes;
    addNodeAndSubdirsToList(m_root_src_file_node, all_sub_nodes);

    if(m_callback->isCanceled()) {
        LoggerD("Operation cancelled");
        return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
    }

    // Calculate total size to be compressed
    //
    m_bytes_to_compress = 0;
    m_files_to_compress = 0;

    int i = 0;
    for(auto it = all_sub_nodes.begin(); it != all_sub_nodes.end(); ++it, ++i) {

        unsigned long long size = 0;
        if((*it)->getType() == filesystem::NT_FILE) {
            result = (*it)->getSize(&size);
            if (result.error_code() != ErrorCode::NO_ERROR) {
                return result;
            }
            m_bytes_to_compress += size;
            ++m_files_to_compress;
        }

        LoggerD("[%d] : [%s] --zip--> [%s] | size: %s", i, (*it)->getPath()->getFullPath().c_str(),
                getNameInZipArchiveFor(*it, m_callback->getFileEntry()->getStriped()).c_str(),
                bytesToReadableString(size).c_str());
    }

    LoggerD("m_files_to_compress: %d", m_files_to_compress);
    LoggerD("m_bytes_to_compress: %llu (%s)", m_bytes_to_compress,
            bytesToReadableString(m_bytes_to_compress).c_str());

    if(m_callback->isCanceled()) {
        LoggerD("Operation cancelled");
        return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
    }

    // Begin files compression
    //
    for(auto it = all_sub_nodes.begin(); it != all_sub_nodes.end(); ++it, ++i) {
        result = addToZipArchive(*it);
        if (result.error_code() != ErrorCode::NO_ERROR) {
            return result;
        }
    }

    m_callback->callSuccessCallbackOnMainThread();
    m_callback = NULL;

    return PlatformResult(ErrorCode::NO_ERROR);
}

void ZipAddRequest::addNodeAndSubdirsToList(filesystem::NodePtr src_node,
        filesystem::NodeList& out_list_of_child_nodes)
{
    LoggerD("Enter");
    out_list_of_child_nodes.push_back(src_node);

    if(filesystem::NT_DIRECTORY == src_node->getType()) {
        LoggerW("STUB Not generating recursive list of files in directory");
        //auto child_nodes = src_node->getChildNodes();
        //for(auto it = child_nodes.begin(); it != child_nodes.end(); ++it) {
        //    addNodeAndSubdirsToList(*it, out_list_of_child_nodes);
        //}
    }
}

PlatformResult ZipAddRequest::addEmptyDirectoryToZipArchive(std::string name_in_zip)
{
    LoggerD("Entered name_in_zip:%s", name_in_zip.c_str());

    if(name_in_zip.length() == 0) {
        LoggerW("Trying to create directory with empty name - \"\"");
        return PlatformResult(ErrorCode::NO_ERROR);
    }

    const char last_char = name_in_zip[name_in_zip.length()-1];
    if(last_char != '/' && last_char != '\\') {
        name_in_zip += "/";
        LoggerD("Corrected name_in_zip: [%s]", name_in_zip.c_str());
    }

    if(m_new_file_in_zip_opened) {
        LoggerE("WARNING: Previous new file in zip archive is opened!");
        int err = zipCloseFileInZip(m_owner.m_zip);
        if (ZIP_OK != err) {
            LoggerE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
        }
    }

    bool is_directory = false;
    std::string conflicting_name;

    if(m_callback->getArchiveFile()->isEntryWithNameInArchive(name_in_zip,
            &is_directory, &conflicting_name)) {

        if(!is_directory) {
            LoggerE("Entry: [%s] exists and is NOT directory!", conflicting_name.c_str());

            LoggerE("Throwing InvalidValuesException - File with the same name exists");
            return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "File with the same name exists");
        }

        LoggerD("Directory: [%s] already exists -> nothing to do", name_in_zip.c_str());
        return PlatformResult(ErrorCode::NO_ERROR);
    }

    if(m_callback->isCanceled()) {
        LoggerD("Operation cancelled");
        return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
    }

    zip_fileinfo new_dir_info;
    memset(&new_dir_info, 0, sizeof(zip_fileinfo));

    //
    // Since this directory does not exist we will set current time
    //
    time_t current_time = time(NULL);
    struct tm current_time_tm = {0};
    tzset();
    if (nullptr != localtime_r(&current_time, &current_time_tm)) {
        new_dir_info.tmz_date.tm_sec  = current_time_tm.tm_sec;
        new_dir_info.tmz_date.tm_min  = current_time_tm.tm_min;
        new_dir_info.tmz_date.tm_hour = current_time_tm.tm_hour;
        new_dir_info.tmz_date.tm_mday = current_time_tm.tm_mday;
        new_dir_info.tmz_date.tm_mon  = current_time_tm.tm_mon ;
        new_dir_info.tmz_date.tm_year = current_time_tm.tm_year;
    }

    int err = zipOpenNewFileInZip3(m_owner.m_zip, name_in_zip.c_str(), &new_dir_info,
            NULL, 0, NULL, 0, NULL,
            (m_compression_level != 0) ? Z_DEFLATED : 0,
            m_compression_level, 0,
            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
            NULL, 0);

    if (err != ZIP_OK) {
        LoggerE("ret: %d", err);
        return PlatformResult(ErrorCode::UNKNOWN_ERR, getArchiveLogMessage(err, "zipOpenNewFileInZip3()"));
    }

    m_new_file_in_zip_opened = true;

    err = zipCloseFileInZip(m_owner.m_zip);
    if (ZIP_OK != err) {
        LoggerE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
    }

    LoggerD("Added new empty directory to archive: [%s]", name_in_zip.c_str());
    m_new_file_in_zip_opened = false;

    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ZipAddRequest::addToZipArchive(filesystem::NodePtr src_file_node)
{
    LoggerD("Enter");
    const std::string name_in_zip = getNameInZipArchiveFor(src_file_node,
            m_callback->getFileEntry()->getStriped());
    const std::string src_file_path = src_file_node->getPath()->getFullPath();

    LoggerD("Compress: [%s] to zip archive as: [%s]", src_file_path.c_str(),
            name_in_zip.c_str());

    zip_fileinfo new_file_info;
    Zip::generateZipFileInfo(src_file_path, new_file_info);

    if(m_new_file_in_zip_opened) {
        LoggerE("WARNING: Previous new file in zip archive is opened!");
        int err = zipCloseFileInZip(m_owner.m_zip);
        if (ZIP_OK != err) {
            LoggerE("zipCloseFileInZip failed with error: %d", err);
        }
    }

    if(m_callback->isCanceled()) {
        LoggerD("Operation cancelled");
        return PlatformResult(ErrorCode::OPERATION_CANCELED_ERR);
    }

    std::string conflicting_name;
    if(m_callback->getArchiveFile()->isEntryWithNameInArchive(name_in_zip,
            NULL, &conflicting_name)) {

        LoggerE("Cannot add new entry with name name: [%s] "
                "it would conflict with existing entry: [%s]",
                name_in_zip.c_str(), conflicting_name.c_str());

        LoggerE("Throwing InvalidModificationException - Archive entry name conflicts");
        return PlatformResult(ErrorCode::INVALID_MODIFICATION_ERR, "Archive entry name conflicts");
    }

    int err = zipOpenNewFileInZip3(m_owner.m_zip, name_in_zip.c_str(), &new_file_info,
            NULL, 0, NULL, 0, NULL,
            (m_compression_level != 0) ? Z_DEFLATED : 0,
            m_compression_level, 0,
            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
            NULL, 0);

    if (err != ZIP_OK) {
        LoggerE("Error opening new file: [%s] in zipfile", name_in_zip.c_str());
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add new file to zip archive");
    }

    m_new_file_in_zip_opened = true;

    if(m_input_file) {
        LoggerW("WARNING: Previous m_input_file has not been closed");
        fclose(m_input_file);
        m_input_file = NULL;
    }

    filesystem::File::PermissionList perm_list;
    filesystem::FilePtr cur_file(new filesystem::File(src_file_node, perm_list));
    ArchiveFileEntryPtr cur_afentry(new ArchiveFileEntry(cur_file));
    cur_afentry->setCompressionLevel(m_compression_level);
    cur_afentry->setName(name_in_zip);

    std::time_t time;
    PlatformResult result = src_file_node->getModified(&time);
    if (result.error_code() != ErrorCode::NO_ERROR) {
        return result;
    }
    cur_afentry->setModified(time);

    auto entry = m_callback->getFileEntry();
    cur_afentry->setDestination(entry->getDestination());
    cur_afentry->setStriped(entry->getStriped());
    cur_afentry->setSize(0);

    LoggerD("m_bytes_compressed:%llu / m_bytes_to_compress: %llu",
            m_bytes_compressed, m_bytes_to_compress);

    LoggerD("m_files_compressed:%d / m_files_to_compress: %d",
            m_files_compressed, m_files_to_compress);

    if(src_file_node->getType() == filesystem::NT_FILE) {
        m_input_file = fopen(src_file_path.c_str(), "rb");
        if (!m_input_file) {
            LoggerE("Error opening source file:%s", src_file_path.c_str());
            return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not open file to be added");
        }

        //Get file length
        fseek(m_input_file, 0, SEEK_END);
        const size_t in_file_size = ftell(m_input_file);
        fseek(m_input_file, 0, SEEK_SET);
        LoggerD("Source file: [%s] size: %d - %s", src_file_path.c_str(),
                in_file_size,
                bytesToReadableString(in_file_size).c_str());

        cur_afentry->setSize(in_file_size);

        if(!m_buffer) {
            m_buffer = new(std::nothrow) char[m_buffer_size];
            if(!m_buffer) {
                LoggerE("Couldn't allocate m_buffer");
                return PlatformResult(ErrorCode::UNKNOWN_ERR, "Memory allocation error");
            }
        }

        size_t total_bytes_read = 0;
        size_t size_read = 0;

        do {
            size_read = fread(m_buffer, 1, m_buffer_size, m_input_file);
            if (size_read < m_buffer_size &&
                        feof(m_input_file) == 0) {
                LoggerE("Error reading source file: %s\n", src_file_path.c_str());
                return PlatformResult(ErrorCode::UNKNOWN_ERR, "New file addition failed");
            }

            LoggerD("Read: %d bytes from input file:[%s]", size_read,
                    src_file_path.c_str());
            total_bytes_read += size_read;
            m_bytes_compressed += size_read;

            if (size_read > 0) {
                err = zipWriteInFileInZip (m_owner.m_zip, m_buffer, size_read);
                if (err < 0) {
                    LoggerE("Error during adding file: %s into zip archive",
                            src_file_path.c_str());
                    return PlatformResult(ErrorCode::UNKNOWN_ERR, "New file addition failed");
                }
            }

            if(total_bytes_read == in_file_size) {
                LoggerD("Finished reading and compressing source file: [%s]",
                        src_file_path.c_str());
                ++m_files_compressed;
            }

            LoggerD("Callculatting overall progress: %llu/%llu bytes; "
                    "%d/%d files; current file: [%s] progress: %d/%d bytes; ",
                    m_bytes_compressed, m_bytes_to_compress,
                    m_files_compressed, m_files_to_compress,
                    src_file_path.c_str(),
                    total_bytes_read, in_file_size);

            double progress = 1.0;
            if(m_bytes_to_compress > 0 || m_files_to_compress > 0) {
                progress = static_cast<double>(m_bytes_compressed + m_files_compressed) /
                        static_cast<double>(m_bytes_to_compress + m_files_to_compress);
            }

            LoggerD("Wrote: %s total progress: %.2f%% %d/%d files",
                    bytesToReadableString(size_read).c_str(), progress * 100.0,
                    m_files_compressed, m_files_to_compress);

            LoggerD("Calling add onprogress callback(%f, %s)", progress,
                    name_in_zip.c_str());
            m_callback->callProgressCallbackOnMainThread(progress, cur_afentry);

        } while (size_read > 0 && total_bytes_read < in_file_size);

        if(in_file_size != total_bytes_read) {
            LoggerE("in_file_size(%d) != total_bytes_read(%d)", in_file_size,
                    total_bytes_read);
            return PlatformResult(ErrorCode::UNKNOWN_ERR, "Could not add file to archive");
        }

        fclose(m_input_file);
        m_input_file = NULL;
    }

    err = zipCloseFileInZip(m_owner.m_zip);
    if (ZIP_OK != err) {
        LoggerE("Error in closing added file:%s in zipfile", src_file_path.c_str());
    }

    m_new_file_in_zip_opened = false;

    return PlatformResult(ErrorCode::NO_ERROR);
}

std::string removeDirCharsFromFront(const std::string& path)
{
    LoggerD("Enter");
    for(size_t i = 0; i < path.length(); ++i) {
        const char& cur = path[i];
        if(cur != '/' && cur != '\\') {
            return path.substr(i);
        }
    }

    return std::string();   //in case path contained only slashes
}

std::string generateFullPathForZip(const std::string& path)
{
    LoggerD("Enter");
    //Step 1: Remove / from begining
    const size_t path_len = path.length();

    size_t start_i = 0;
    for(size_t i = 0; i < path_len; ++i) {
        const char& cur = path[i];
        if(cur != '/' && cur != '\\') {
            start_i = i;
            break;
        }
    }

    std::string out;
    out.reserve(path_len);

    //Step 1: Remove duplicated / characters
    bool prev_is_dir = false;
    for(size_t i = start_i; i < path_len; ++i) {
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

std::string ZipAddRequest::getNameInZipArchiveFor(filesystem::NodePtr node, bool strip)
{
    LoggerD("Enter");
    const std::string node_full_path = node->getPath()->getFullPath();
    std::string cut_path;

    const size_t pos = node_full_path.find(m_absoulte_path_to_extract);
    if(std::string::npos == pos) {
        LoggerW("node: [%s] is not containing m_absoulte_path_to_extract: [%s]!",
                node_full_path.c_str(),
                m_absoulte_path_to_extract.c_str());
    }

    cut_path = node_full_path.substr(pos+m_absoulte_path_to_extract.length());
    LoggerD("node_full_path:%s cut_path: %s", node_full_path.c_str(), cut_path.c_str());

    if(!strip) {
        cut_path = m_callback->getBaseVirtualPath() + "/" + cut_path;
        LoggerD("nonstripped cut_path: %s", cut_path.c_str());
    }

    std::string name = generateFullPathForZip(m_destination_path_in_zip + "/" + cut_path);
    if(node->getType() == filesystem::NT_DIRECTORY) {
        if(name.length() > 0
                && name[name.length()-1] != '/'
                && name[name.length()-1] != '\\') {
            name += "/";
            LoggerD("Directory: [%s] added \\", name.c_str());
        }
    }

    return name;
}

} //namespace archive
} //namespace extension
