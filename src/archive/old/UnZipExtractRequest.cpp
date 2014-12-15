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

#include "UnZipExtractRequest.h"

#include <cstdio>
#include <errno.h>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>

#include <Logger.h>
#include <Node.h>
#include <Path.h>
#include <PlatformException.h>

#include "ArchiveFile.h"
#include "ArchiveUtils.h"
#include "UnZip.h"

namespace DeviceAPI {

using namespace DeviceAPI::Common;

namespace Archive {

FilePathStatus getPathStatus(const std::string& path)
{
    if(path.empty()) {
        return FPS_NOT_EXIST;
    }

    std::string npath = removeTrailingDirectorySlashFromPath(path);

    struct stat sb;
    if (stat(npath.c_str(), &sb) == -1) {
        return FPS_NOT_EXIST;
    }
    if(sb.st_mode & S_IFDIR) {
        return FPS_DIRECTORY;
    } else {
        return FPS_FILE;
    }
}

void divideToPathAndName(const std::string& filepath, std::string& out_path,
        std::string& out_name)
{

    size_t pos_last_dir = filepath.find_last_of("/\\");
    if(pos_last_dir == std::string::npos) {
        out_path = "";
        out_name = filepath;
    } else {
        out_path = filepath.substr(0, pos_last_dir+1);
        out_name = filepath.substr(pos_last_dir+1);
    }
}

void createMissingDirectories(const std::string& path, bool check_first = true)
{
    if(check_first) {
        const FilePathStatus path_status = getPathStatus(path);
        //LOGD("[%s] status: %d", path.c_str(), path_status);
        if(FPS_DIRECTORY == path_status) {
            return;
        }
    }

    const size_t extract_path_len = path.length();
    for(size_t i = 0; i < extract_path_len; ++i) {
        const char& cur = path[i];
        if( (('\\' == cur || '/' == cur) && i > 0) ||   //handle left side from current /
                (extract_path_len-1 == i) ) {           //handle last subdirectory path

            const std::string left_part = path.substr(0,i+1);
            const FilePathStatus status = getPathStatus(left_part);
            //LOGD("left_part: [%s] status:%d", left_part.c_str(), status);

            if(FPS_DIRECTORY != status) {
                //TODO investigate 0775 (mode) - Filesystem assumed that file should have parent mode
                if(mkdir(left_part.c_str(), 0775) == -1) {
                    LOGE("Couldn't create new directory: %s errno:%s",
                            left_part.c_str(), strerror(errno));
               //TODO check why mkdir return -1 but directory is successfully created
               //     throw UnknownException(
               //             "Could not create new directory");
                }
            }
        }
    }
}

void changeFileAccessAndModifyDate(const std::string& filepath, tm_unz tmu_date)
{
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min = tmu_date.tm_min;
  newdate.tm_hour = tmu_date.tm_hour;
  newdate.tm_mday = tmu_date.tm_mday;
  newdate.tm_mon = tmu_date.tm_mon;

  if (tmu_date.tm_year > 1900) {
      newdate.tm_year = tmu_date.tm_year - 1900;
  } else {
      newdate.tm_year = tmu_date.tm_year ;
  }
  newdate.tm_isdst = -1;

  ut.actime = ut.modtime = mktime(&newdate);
  if(utime(filepath.c_str(), &ut) == -1) {
      LOGE("Couldn't set time for: [%s] errno:%s", filepath.c_str(), strerror(errno));
  }
}

void UnZipExtractRequest::execute(UnZip& owner, const std::string& extract_path,
        const std::string& base_strip_path,
        BaseProgressCallback* callback)
{
    UnZipExtractRequest req(owner, extract_path, base_strip_path, callback);
    req.run();
}

UnZipExtractRequest::UnZipExtractRequest(UnZip& owner,
        const std::string& extract_path,
        const std::string& base_strip_path,
        BaseProgressCallback* callback) :

        m_owner(owner),
        m_extract_path(extract_path),
        m_base_strip_path(base_strip_path),
        m_callback(callback),

        m_output_file(NULL),
        m_buffer(NULL),
        m_delete_output_file(false),
        m_close_unz_current_file(false),
        m_new_dir_status(FPS_NOT_EXIST),

        m_is_directory_entry(false)
{
    if(!m_callback){
        LOGE("Callback is null");
        throw UnknownException("Problem with callback functionality");
    }
}

void UnZipExtractRequest::run()
{
    LOGD("Entered");

    getCurrentFileInfo();

    if(m_is_directory_entry) {
        handleDirectoryEntry();
    } else {
        handleFileEntry();
    }
}

UnZipExtractRequest::~UnZipExtractRequest()
{
    if(m_output_file) {
        fclose(m_output_file);
        m_output_file = NULL;
    }

    if(m_delete_output_file && !m_is_directory_entry) {
        if(std::remove(m_output_filepath.c_str()) != 0) {
            LOGE("Couldn't remove partial file! "
                    "std::remove(\"%s\") failed with errno:%s",
                    m_output_filepath.c_str(), strerror(errno));
        }
    }

    delete [] m_buffer;
    m_buffer = NULL;

    if(m_close_unz_current_file) {
        int err = unzCloseCurrentFile (m_owner.m_unzip);
        if(UNZ_OK != err) {
            LOGW("%s",getArchiveLogMessage(err, "unzCloseCurrentFile()").c_str());
        }
    }
}

void UnZipExtractRequest::getCurrentFileInfo()
{
    LOGD("Entered");
    int err = unzGetCurrentFileInfo(m_owner.m_unzip, &m_file_info,
            m_filename_inzip, sizeof(m_filename_inzip), NULL, 0, NULL, 0);
    if (err != UNZ_OK) {
        LOGE("ret: %d", err);
        throwArchiveException(err, "unzGetCurrentFileInfo()");
    }

    LOGD("Input from ZIP: m_filename_inzip: [%s]", m_filename_inzip);
    LOGD("m_base_strip_path: [%s]", m_base_strip_path.c_str());

    std::string file_path = m_filename_inzip;
    if(!m_base_strip_path.empty()) {
        if(file_path.find(m_base_strip_path) != 0) {
            LOGW("m_base_strip_path: [%s] is not begin of m_filename_inzip: [%s]!",
                    m_base_strip_path.c_str(),
                    m_filename_inzip);
        }
        else {
            file_path = file_path.substr(m_base_strip_path.length());
            LOGD("Stripped file name: [%s]", file_path.c_str());
        }
    }
    else {
        LOGD("Not stripped file name: [%s]", file_path.c_str());
    }

    m_output_filepath = removeDuplicatedSlashesFromPath(m_extract_path + "/" + file_path);

    LOGD("Packed: [%s], uncompressed_size: %d, will extract to: [%s]",
            m_filename_inzip, m_file_info.uncompressed_size,
            m_output_filepath.c_str());

    std::string path, name;
    divideToPathAndName(file_path, path, name);
    m_new_dir_path = removeDuplicatedSlashesFromPath(m_extract_path + "/" + path);
    m_new_dir_status = getPathStatus(m_new_dir_path);
    m_is_directory_entry = name.empty();

    LOGD("New output dir: [%s] status: %d m_is_directory_entry: %d",
            m_new_dir_path.c_str(), m_new_dir_status, m_is_directory_entry);

    if(FPS_DIRECTORY != m_new_dir_status) {

        if(m_is_directory_entry) {

            std::string base_directories;
            const size_t len = m_new_dir_path.length();

            for(int i = static_cast<int>(len) - 2; i >= 0; i--) {  //skip last \, /
                const char& cur = m_new_dir_path[i];
                if('\\' == cur || '/' == cur) {
                    base_directories = m_new_dir_path.substr(0, static_cast<size_t>(i));
                    break;
                }
            }

            LOGD("Type: DIRECTORY checking base output directories: [%s]",
                    base_directories.c_str());
            createMissingDirectories(base_directories, false);
        } else {
            LOGD("Type: FILE checking output dir: [%s]", m_new_dir_path.c_str());
            createMissingDirectories(m_new_dir_path, false);
        }
    }
}

void UnZipExtractRequest::handleDirectoryEntry()
{
    LOGD("Entered");
    if(FPS_DIRECTORY != m_new_dir_status) {

        if(FPS_FILE == m_new_dir_status) {
            if(m_callback->getOverwrite()) {    //Is a file & overwrite is set:
                std::string fn = removeTrailingDirectorySlashFromPath(m_new_dir_path);
                if(std::remove(fn.c_str()) != 0) {
                    LOGE("std::remove(\"%s\") failed with errno:%s",
                            m_new_dir_path.c_str(), strerror(errno));
                    throw UnknownException(
                            "Could not overwrite file in output directory");
                }
            } else {                            //Is a file & overwrite is not set:
                LOGE("Failed to extract directory, "
                        "file with the same name exists in output directory");
                throw UnknownException("Failed to extract directory, "
                        "file with the same name exists in output directory");
            }
        }

        //Try to create new directory in output directory
        if(mkdir(m_new_dir_path.c_str(), 0775) == -1) {
            LOGE("Couldn't create new directory: %s errno:%s",
                    m_new_dir_path.c_str(), strerror(errno));
            throw UnknownException(
                    "Could not create new directory in extract output directory");
        }
    }

    LOGD("Set dir: [%s] access and modify to: %4d-%2d-%2d %2d:%2d:%2d", m_new_dir_path.c_str(),
            m_file_info.tmu_date.tm_year,
            m_file_info.tmu_date.tm_mon,
            m_file_info.tmu_date.tm_mday,
            m_file_info.tmu_date.tm_hour,
            m_file_info.tmu_date.tm_min,
            m_file_info.tmu_date.tm_sec);

    // Directory already exists we only need to update time
    changeFileAccessAndModifyDate(m_new_dir_path, m_file_info.tmu_date);

    LOGD("Extracted directory entry: [%s]", m_new_dir_path.c_str());
}

bool UnZipExtractRequest::prepareOutputSubdirectory()
{
    LOGD("Entered");
    //This zip entry points to file - verify that parent directory in output dir exists
    if(FPS_DIRECTORY != m_new_dir_status) {
        if(FPS_FILE == m_new_dir_status) {
            LOGE("Path: %s is pointing to file not directory!",
                    m_new_dir_path.c_str());
            throw UnknownException("Failed to extract file from zip archive, "
                    "output path is invalid");
        }

        //Try to create new directory in output directory
        //TODO investigate 0775 (mode) - Filesystem assumed that file should have parent mode
        if(mkdir(m_new_dir_path.c_str(), 0775) == -1) {
            LOGW("couldn't create new directory: %s errno:%s",
                    m_new_dir_path.c_str(), strerror(errno));
            //TODO check why mkdir return -1 but directory is successfully created
            //     throw UnknownException(
            //     "Could not create new directory in extract output directory");
        }
    }

    if(m_callback->isCanceled()) {
        LOGD("Operation cancelled");
        throw OperationCanceledException();
    }

    const FilePathStatus output_fstatus = getPathStatus(m_output_filepath);
    if(FPS_NOT_EXIST != output_fstatus) {
        if(!m_callback->getOverwrite()) {
            LOGW("%s exists at output path: [%s], overwrite is set to FALSE",
                    (FPS_DIRECTORY == output_fstatus ? "Directory" : "File"),
                    m_output_filepath.c_str());

            //Just skip this file - TODO: this should be documented in WIDL
            return false;
        } else {
            if(FPS_DIRECTORY == output_fstatus) {
                try {
                    Filesystem::PathPtr path = Filesystem::Path::create(m_output_filepath);
                    Filesystem::NodePtr node = Filesystem::Node::resolve(path);
                    node->remove(Filesystem::OPT_RECURSIVE);
                    LOGD("Removed directory: [%s]", m_output_filepath.c_str());
                } catch(BasePlatformException& ex) {
                    LOGE("Remove dir: [%s] failed with exception: %s:%s",
                            m_output_filepath.c_str(),
                            ex.getName().c_str(), ex.getMessage().c_str());
                    throw UnknownException("Could not overwrite existing directory");
                } catch (...) {
                    LOGE("Remove dir: [%s] failed", m_output_filepath.c_str());
                    throw UnknownException("Could not overwrite existing directory");
                }
            } //else {
                //We will overwrite it with fopen
            //}
        }
    }

    return true;
}

void UnZipExtractRequest::handleFileEntry()
{
    LOGD("Entered");
    if(!prepareOutputSubdirectory()) {
        LOGE("File exists but overwrite is false");
        throw InvalidModificationException("file already exists.");
    }

    int err = unzOpenCurrentFilePassword(m_owner.m_unzip,
        NULL); //password is not supported yet therefore passing NULL
    if (UNZ_OK != err) {
        LOGE("ret: %d", err);
        throwArchiveException(err, "unzOpenCurrentFilePassword()");
    }

    //We have successfully opened curent file, therefore we should close it later
    m_close_unz_current_file = true;

    const size_t buffer_size = m_owner.m_default_buffer_size;
    m_buffer = new(std::nothrow) char[buffer_size];
    if(!m_buffer) {
        LOGE("Couldn't allocate buffer with size: %s",
                bytesToReadableString(buffer_size).c_str());
        throw UnknownException("Memory allocation failed");
    }

    m_output_file = fopen(m_output_filepath.c_str(), "wb");
    if(!m_output_file) {
        LOGE("Couldn't open output file: %s", m_output_filepath.c_str());
        throw UnknownException("Could not create extracted file");
    }
    m_delete_output_file = true;

    int read_size = 0;
    bool marked_as_finished = false;

    LOGD("Started extracting: [%s] uncompressed size: %d - %s", m_filename_inzip,
            m_file_info.uncompressed_size,
            bytesToReadableString(m_file_info.uncompressed_size).c_str());

    ExtractAllProgressCallback* extract_callback = NULL;
    if(m_callback->getCallbackType() == EXTRACT_ALL_PROGRESS_CALLBACK ||
            m_callback->getCallbackType() == EXTRACT_ENTRY_PROGRESS_CALLBACK) {
        extract_callback = static_cast<ExtractAllProgressCallback*>(m_callback);
        extract_callback->startedExtractingFile(m_file_info.uncompressed_size);
    }

    ArchiveFileEntryPtrMapPtr entries = m_callback->getArchiveFile()->getEntryMap();
    auto it = entries->find(m_filename_inzip);
    if (it == entries->end()) {
        LOGE("Entry not found");
        throw Common::NotFoundException("Entry not found");
    }

    while(true) {
        if(m_callback->isCanceled()) {
            LOGD("Operation cancelled");
            throw OperationCanceledException();
        }

        read_size = unzReadCurrentFile(m_owner.m_unzip, m_buffer, buffer_size);
        if (read_size < 0) {
            LOGE("unzReadCurrentFile failed with error code:%d for file:%s", read_size,
                    m_filename_inzip);
            throw UnknownException("Failed to extract file from zip archive");
        }
        else if(0 == read_size) {

            if(extract_callback) {
                if(!marked_as_finished) {
                    LOGD("NOT marked_as_finished -> increment extracted files counter");
                    extract_callback->finishedExtractingFile();

                    //Call progress callback only if we expected empty file
                    if(m_file_info.uncompressed_size == 0) {
                        LOGD("Calling progress callback(%f, %s)",
                                extract_callback->getOverallProgress(), m_filename_inzip);
                        extract_callback->callProgressCallbackOnMainThread(
                                extract_callback->getOverallProgress(), it->second);
                    }
                }
            }
            //Finished writing file, we should not delete extracted file
            m_delete_output_file = false;
            break;
        }

        if (fwrite(m_buffer, read_size, 1, m_output_file) != 1) {
            LOGE("Couldn't write extracted data to output file:%s",
                    m_output_filepath.c_str());
            throw UnknownException("Could not write extract file into output file");
        }

        if(extract_callback) {
            extract_callback->extractedPartOfFile(read_size);
            LOGD("File: [%s] extracted: %s - %f%%; total progress %f%%", m_filename_inzip,
                    bytesToReadableString(read_size).c_str(),
                    100.0f * extract_callback->getCurrentFileProgress(),
                    100.0f * extract_callback->getOverallProgress());

            // It is better to update number of extracted entries so we will have
            // overal progres: 1.0 if all files are extracted
            //
            if(extract_callback->getCurrentFileProgress() >= 1.0) {
                LOGD("Current file: [%s] progress: %f >= 1.0 -> "
                        "marked_as_finished = true and increment extracted files counter",
                        m_filename_inzip, extract_callback->getCurrentFileProgress());
                marked_as_finished = true;
                extract_callback->finishedExtractingFile();
            }

            LOGD("Calling progress callback(%f, %s)",
                    extract_callback->getOverallProgress(), m_filename_inzip);
            extract_callback->callProgressCallbackOnMainThread(
                    extract_callback->getOverallProgress(), it->second);
        }
    }

    if(m_output_file) {
        fclose(m_output_file);
        m_output_file = NULL;
    }

    changeFileAccessAndModifyDate(m_output_filepath, m_file_info.tmu_date);
}

} //namespace Archive
} //namespace DeviceAPI
