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

#include <Logger.h>

#include "ZipAddRequest.h"

#include <PlatformException.h>

#include "ArchiveFile.h"
#include "ArchiveFileEntry.h"
#include "ArchiveUtils.h"

namespace DeviceAPI {

using namespace DeviceAPI::Common;

namespace Archive {

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
}

ZipAddRequest::~ZipAddRequest()
{
    if(m_input_file) {
        fclose(m_input_file);
        m_input_file = NULL;
    }

    delete [] m_buffer;
    m_buffer = NULL;


    if(m_new_file_in_zip_opened) {
        int err = zipCloseFileInZip(m_owner.m_zip);
        if (ZIP_OK != err) {
            LOGE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
        }
    }
}

void ZipAddRequest::execute(Zip& owner, AddProgressCallback*& callback)
{
    ZipAddRequest req(owner, callback);
    req.run();
}

void ZipAddRequest::run()
{
    if(!m_callback) {
        LOGE("m_callback is NULL");
        throw UnknownException("Could not add file(-s) to archive");
    }

    if(!m_callback->getFileEntry()) {
        LOGE("m_callback->getFileEntry() is NULL");
        throw InvalidValuesException("Provided ArchiveFileEntry is not correct");
    }

    if(m_callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    m_compression_level = m_callback->getFileEntry()->getCompressionLevel();
    m_root_src_file = m_callback->getFileEntry()->getFile();
    m_root_src_file_node = m_root_src_file->getNode();

    //We just need read permission to list files in subdirectories
    m_root_src_file_node->setPermissions(Filesystem::PERM_READ);

    std::string src_basepath, src_name;
    getBasePathAndName(m_root_src_file_node->getPath()->getFullPath(), src_basepath,
            src_name);

    m_absoulte_path_to_extract = src_basepath;
    LOGD("m_absoulte_path_to_extract: [%s]", m_absoulte_path_to_extract.c_str());

    m_destination_path_in_zip = removeDuplicatedSlashesFromPath(
            m_callback->getFileEntry()->getDestination());

    // If we have destination set then we need to create all folders and sub folders
    // inside this zip archive.
    //
    if(m_destination_path_in_zip.length() > 0) {
        LOGD("destination is: [%s]", m_destination_path_in_zip.c_str());

        for(size_t i = 0; i < m_destination_path_in_zip.length(); ++i) {
            const char cur_char = m_destination_path_in_zip[i];

            if( ((cur_char == '/' || cur_char == '\\') && i > 0 ) ||
                (i == m_destination_path_in_zip.length() - 1)) {

                //Extract left side with '/':
                const std::string new_dir = m_destination_path_in_zip.substr(0, i + 1);

                LOGD("Adding empty directory: [%s] to archive", new_dir.c_str());
                addEmptyDirectoryToZipArchive(new_dir);
            }
        }
    }

    // Generate list of files and all subdirectories
    //
    Filesystem::NodeList all_sub_nodes;
    addNodeAndSubdirsToList(m_root_src_file_node, all_sub_nodes);

    if(m_callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    // Calculate total size to be compressed
    //
    m_bytes_to_compress = 0;
    m_files_to_compress = 0;

    int i = 0;
    for(auto it = all_sub_nodes.begin(); it != all_sub_nodes.end(); ++it, ++i) {

        unsigned long long size = 0;
        if((*it)->getType() == Filesystem::NT_FILE) {
            size = (*it)->getSize();
            m_bytes_to_compress += size;
            ++m_files_to_compress;
        }

        LOGD("[%d] : [%s] --zip--> [%s] | size: %s", i, (*it)->getPath()->getFullPath().c_str(),
                getNameInZipArchiveFor(*it, m_callback->getFileEntry()->getStriped()).c_str(),
                bytesToReadableString(size).c_str());
    }

    LOGD("m_files_to_compress: %d", m_files_to_compress);
    LOGD("m_bytes_to_compress: %llu (%s)", m_bytes_to_compress,
            bytesToReadableString(m_bytes_to_compress).c_str());

    if(m_callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    // Begin files compression
    //
    for(auto it = all_sub_nodes.begin(); it != all_sub_nodes.end(); ++it, ++i) {
            addToZipArchive(*it);
    }

    m_callback->callSuccessCallbackOnMainThread();
    m_callback = NULL;
}

void ZipAddRequest::addNodeAndSubdirsToList(Filesystem::NodePtr src_node,
        Filesystem::NodeList& out_list_of_child_nodes)
{
    out_list_of_child_nodes.push_back(src_node);

    if(Filesystem::NT_DIRECTORY == src_node->getType()) {
        auto child_nodes = src_node->getChildNodes();
        for(auto it = child_nodes.begin(); it != child_nodes.end(); ++it) {
            addNodeAndSubdirsToList(*it, out_list_of_child_nodes);
        }
    }
}

void ZipAddRequest::addEmptyDirectoryToZipArchive(std::string name_in_zip)
{
    LOGD("Entered name_in_zip:%s", name_in_zip.c_str());

    if(name_in_zip.length() == 0) {
        LOGW("Trying to create directory with empty name - \"\"");
        return;
    }

    const char last_char = name_in_zip[name_in_zip.length()-1];
    if(last_char != '/' && last_char != '\\') {
            name_in_zip += "/";
            LOGD("Corrected name_in_zip: [%s]", name_in_zip.c_str());
    }

    if(m_new_file_in_zip_opened) {
        LOGE("WARNING: Previous new file in zip archive is opened!");
        int err = zipCloseFileInZip(m_owner.m_zip);
        if (ZIP_OK != err) {
            LOGE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
        }
    }

    bool is_directory = false;
    std::string conflicting_name;

    if(m_callback->getArchiveFile()->isEntryWithNameInArchive(name_in_zip,
            &is_directory, &conflicting_name)) {

        if(!is_directory) {
            LOGE("Entry: [%s] exists and is NOT directory!", conflicting_name.c_str());

            LOGE("Throwing InvalidValuesException - File with the same name exists");
            throw InvalidValuesException("File with the same name exists");
        }

        LOGD("Directory: [%s] already exists -> nothing to do", name_in_zip.c_str());
            return;
    }

    if(m_callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    zip_fileinfo new_dir_info;
    memset(&new_dir_info, 0, sizeof(zip_fileinfo));

    //
    // Since this directory does not exist we will set current time
    //
    time_t current_time = time(NULL);
    struct tm* current_time_tm = localtime(&current_time);
    if(current_time_tm) {
        new_dir_info.tmz_date.tm_sec  = current_time_tm->tm_sec;
        new_dir_info.tmz_date.tm_min  = current_time_tm->tm_min;
        new_dir_info.tmz_date.tm_hour = current_time_tm->tm_hour;
        new_dir_info.tmz_date.tm_mday = current_time_tm->tm_mday;
        new_dir_info.tmz_date.tm_mon  = current_time_tm->tm_mon ;
        new_dir_info.tmz_date.tm_year = current_time_tm->tm_year;
    }

    int err = zipOpenNewFileInZip3(m_owner.m_zip, name_in_zip.c_str(), &new_dir_info,
            NULL, 0, NULL, 0, NULL,
            (m_compression_level != 0) ? Z_DEFLATED : 0,
            m_compression_level, 0,
            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
            NULL, 0);

    if (err != ZIP_OK) {
        LOGE("ret: %d", err);
        throwArchiveException(err, "zipOpenNewFileInZip3()");
    }

    m_new_file_in_zip_opened = true;

    err = zipCloseFileInZip(m_owner.m_zip);
    if (ZIP_OK != err) {
        LOGE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
    }

    LOGD("Added new empty directory to archive: [%s]", name_in_zip.c_str());
    m_new_file_in_zip_opened = false;
}

void ZipAddRequest::addToZipArchive(Filesystem::NodePtr src_file_node)
{
    const std::string name_in_zip = getNameInZipArchiveFor(src_file_node,
            m_callback->getFileEntry()->getStriped());
    const std::string src_file_path = src_file_node->getPath()->getFullPath();

    LOGD("Compress: [%s] to zip archive as: [%s]", src_file_path.c_str(),
            name_in_zip.c_str());

    zip_fileinfo new_file_info;
    Zip::generateZipFileInfo(src_file_path, new_file_info);

    if(m_new_file_in_zip_opened) {
        LOGE("WARNING: Previous new file in zip archive is opened!");
        int err = zipCloseFileInZip(m_owner.m_zip);
        if (ZIP_OK != err) {
            LOGE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
        }
    }

    if(m_callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    std::string conflicting_name;
    if(m_callback->getArchiveFile()->isEntryWithNameInArchive(name_in_zip,
            NULL, &conflicting_name)) {

        LOGE("Cannot add new entry with name name: [%s] "
                "it would conflict with existing entry: [%s]",
                name_in_zip.c_str(), conflicting_name.c_str());

        LOGE("Throwing InvalidModificationException - Archive entry name conflicts");
        throw InvalidModificationException("Archive entry name conflicts");
    }

    int err = zipOpenNewFileInZip3(m_owner.m_zip, name_in_zip.c_str(), &new_file_info,
            NULL, 0, NULL, 0, NULL,
            (m_compression_level != 0) ? Z_DEFLATED : 0,
            m_compression_level, 0,
            -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
            NULL, 0);

    if (err != ZIP_OK) {
        LOGE("ret: %d", err);
        throwArchiveException(err, "zipOpenNewFileInZip3()");
    }

    m_new_file_in_zip_opened = true;

    if(m_input_file) {
        LOGW("WARNING: Previous m_input_file has not been closed");
        fclose(m_input_file);
        m_input_file = NULL;
    }

    Filesystem::File::PermissionList perm_list;
    Filesystem::FilePtr cur_file(new Filesystem::File(src_file_node, perm_list));
    ArchiveFileEntryPtr cur_afentry(new ArchiveFileEntry(cur_file));
    cur_afentry->setCompressionLevel(m_compression_level);
    cur_afentry->setName(name_in_zip);
    cur_afentry->setModified(src_file_node->getModified());

    auto entry = m_callback->getFileEntry();
    cur_afentry->setDestination(entry->getDestination());
    cur_afentry->setStriped(entry->getStriped());
    cur_afentry->setSize(0);

    LOGD("m_bytes_compressed:%llu / m_bytes_to_compress: %llu",
            m_bytes_compressed, m_bytes_to_compress);

    LOGD("m_files_compressed:%d / m_files_to_compress: %d",
            m_files_compressed, m_files_to_compress);

    if(src_file_node->getType() == Filesystem::NT_FILE) {
        m_input_file = fopen(src_file_path.c_str(), "rb");
        if (!m_input_file) {
            LOGE("Error opening source file:%s", src_file_path.c_str());
            throw UnknownException("Could not open file to be added");
        }

        //Get file length
        fseek(m_input_file, 0, SEEK_END);
        const size_t in_file_size = ftell(m_input_file);
        fseek(m_input_file, 0, SEEK_SET);
        LOGD("Source file: [%s] size: %d - %s", src_file_path.c_str(),
                in_file_size,
                bytesToReadableString(in_file_size).c_str());

        cur_afentry->setSize(in_file_size);

        if(!m_buffer) {
            m_buffer = new(std::nothrow) char[m_buffer_size];
            if(!m_buffer) {
                LOGE("Couldn't allocate m_buffer");
                throw UnknownException("Memory allocation error");
            }
        }

        size_t total_bytes_read = 0;
        size_t size_read = 0;

        do {
            size_read = fread(m_buffer, 1, m_buffer_size, m_input_file);
            if (size_read < m_buffer_size &&
                        feof(m_input_file) == 0) {
                LOGE("Error reading source file: %s\n", src_file_path.c_str());
                throw UnknownException("New file addition failed");
            }

            LOGD("Read: %d bytes from input file:[%s]", size_read,
                    src_file_path.c_str());
            total_bytes_read += size_read;
            m_bytes_compressed += size_read;

            if (size_read > 0) {
                err = zipWriteInFileInZip (m_owner.m_zip, m_buffer, size_read);
                if (err < 0) {
                    LOGE("Error during adding file: %s into zip archive",
                            src_file_path.c_str());
                    throw UnknownException("New file addition failed");
                }
            }

            if(total_bytes_read == in_file_size) {
                LOGD("Finished reading and compressing source file: [%s]",
                        src_file_path.c_str());
                ++m_files_compressed;
            }

            LOGD("Callculatting overall progress: %llu/%llu bytes; "
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

            LOGD("Wrote: %s total progress: %.2f%% %d/%d files",
                    bytesToReadableString(size_read).c_str(), progress * 100.0,
                    m_files_compressed, m_files_to_compress);

            LOGD("Calling add onprogress callback(%f, %s)", progress,
                    name_in_zip.c_str());
            m_callback->callProgressCallbackOnMainThread(progress, cur_afentry);

        } while (size_read > 0 && total_bytes_read < in_file_size);

        if(in_file_size != total_bytes_read) {
            LOGE("in_file_size(%d) != total_bytes_read(%d)", in_file_size,
                    total_bytes_read);
            throw UnknownException("Could not add file to archive");
        }

        fclose(m_input_file);
        m_input_file = NULL;
    }

    err = zipCloseFileInZip(m_owner.m_zip);
    if (ZIP_OK != err) {
        LOGE("%s",getArchiveLogMessage(err, "zipCloseFileInZip()").c_str());
    }

    m_new_file_in_zip_opened = false;
}

std::string removeDirCharsFromFront(const std::string& path)
{
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

std::string ZipAddRequest::getNameInZipArchiveFor(Filesystem::NodePtr node, bool strip)
{
    const std::string node_full_path = node->getPath()->getFullPath();
    std::string cut_path;

    const size_t pos = node_full_path.find(m_absoulte_path_to_extract);
    if(std::string::npos == pos) {
        LOGW("node: [%s] is not containing m_absoulte_path_to_extract: [%s]!",
                node_full_path.c_str(),
                m_absoulte_path_to_extract.c_str());
    }

    cut_path = node_full_path.substr(pos+m_absoulte_path_to_extract.length());
    LOGD("node_full_path:%s cut_path: %s", node_full_path.c_str(), cut_path.c_str());

    if(!strip) {
        cut_path = m_callback->getBaseVirtualPath() + "/" + cut_path;
        LOGD("nonstripped cut_path: %s", cut_path.c_str());
    }

    std::string name = generateFullPathForZip(m_destination_path_in_zip + "/" + cut_path);
    if(node->getType() == Filesystem::NT_DIRECTORY) {
        if(name.length() > 0
                && name[name.length()-1] != '/'
                && name[name.length()-1] != '\\') {
            name += "/";
            LOGD("Directory: [%s] added \\", name.c_str());
        }
    }

    return name;
}

} //namespace Archive
} //namespace DeviceAPI
