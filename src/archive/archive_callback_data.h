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

/**
 * @file        ArchiveCallbackData.h
 */

#ifndef __ARCHIVE_CALLBACK_DATA_H__
#define __ARCHIVE_CALLBACK_DATA_H__

#include <string>
#include <memory>
#include <glib.h>

#include "common/platform_exception.h"
#include "filesystem_file.h"
#include "archive_file_entry.h"


namespace extension {
namespace archive {

using namespace common;
using namespace filesystem;

class ArchiveFileEntry;
typedef std::shared_ptr<ArchiveFileEntry> ArchiveFileEntryPtr;

enum ArchiveCallbackType {
    OPERATION_CALLBACK_DATA = 0,
    OPEN_CALLBACK_DATA = 1,
    GET_ENTRIES_CALLBACK_DATA = 2,
    GET_ENTRY_BY_NAME_CALLBACK_DATA = 3,
    BASE_PROGRESS_CALLBACK = 4,
    EXTRACT_ALL_PROGRESS_CALLBACK = 5,
    EXTRACT_ENTRY_PROGRESS_CALLBACK = 6,
    ADD_PROGRESS_CALLBACK = 7
};

class ArchiveFile;
typedef std::shared_ptr<ArchiveFile> ArchiveFilePtr;

class OperationCallbackData
{
public:
    OperationCallbackData(ArchiveCallbackType callback_type);
    virtual ~OperationCallbackData();

    void setError(const std::string &err_name,
            const std::string &err_message);
    bool isError() const;
    const std::string& getErrorName() const;
    const std::string& getErrorMessage() const;

    void setOperationId(long op_id);
    long getOperationId() const;
    void setCallbackId(double cid);
    double getCallbackId() const;
    void setHandle(long handle);
    long getHandle() const;

    ArchiveCallbackType getCallbackType() const;

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr) = 0;

    ArchiveFilePtr getArchiveFile() const;
    void setArchiveFile(ArchiveFilePtr caller);

    bool isCanceled() const;
    void setIsCanceled(bool canceled);

protected:
    ArchiveCallbackType m_callback_type;
    long m_op_id;
    double m_cid;
    long m_handle;

private:
    bool m_is_error;
    bool m_is_canceled;
    std::string m_err_name;
    std::string m_err_message;

    ArchiveFilePtr m_caller_instance;
};

class OpenCallbackData : public OperationCallbackData
{
public:
    OpenCallbackData(ArchiveCallbackType callback_type = OPEN_CALLBACK_DATA);
    virtual ~OpenCallbackData();

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr);

private:
    std::string m_filename;
};

class GetEntriesCallbackData : public OperationCallbackData
{
public:
    GetEntriesCallbackData(ArchiveCallbackType callback_type = GET_ENTRIES_CALLBACK_DATA);
    virtual ~GetEntriesCallbackData();

    ArchiveFileEntryPtrMapPtr getEntries() const;
    void setEntries(ArchiveFileEntryPtrMapPtr entries);

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr);

private:
    ArchiveFileEntryPtrMapPtr m_entries;
};

class GetEntryByNameCallbackData : public OperationCallbackData
{
public:
    GetEntryByNameCallbackData(ArchiveCallbackType callback_type = GET_ENTRY_BY_NAME_CALLBACK_DATA);
    virtual ~GetEntryByNameCallbackData();

    const std::string& getName() const;
    void setName(const std::string& name);

    ArchiveFileEntryPtr getFileEntry() const;
    void setFileEntry(ArchiveFileEntryPtr entry);

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr);
private:
    std::string m_name;
    ArchiveFileEntryPtr m_file_entry;

};

class BaseProgressCallback : public OperationCallbackData
{
public:
    BaseProgressCallback(ArchiveCallbackType callback_type = BASE_PROGRESS_CALLBACK);
    virtual ~BaseProgressCallback();

    //void setProgressCallback(JSValueRef on_progress);

    bool getOverwrite() const;
    void setOverwrite(bool overwrite);

    void callProgressCallbackOnMainThread(const double progress,
            ArchiveFileEntryPtr current_entry);
    void callSuccessCallbackOnMainThread();

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr) = 0;

protected:
    void callProgressCallback(long operationId,
            double value,
            const std::string& filename,
            double callbackId);

private:
    static gboolean callProgressCallbackCB(void* data);
    static gboolean callSuccessCallbackCB(void* data);

    bool m_overwrite;
};

class AddProgressCallback : public BaseProgressCallback
{
public:
    AddProgressCallback(ArchiveCallbackType callback_type = ADD_PROGRESS_CALLBACK);
    virtual ~AddProgressCallback();

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr);

    void setBasePath(const std::string& path);
    const std::string& getBasePath();
    const std::string& getBaseVirtualPath();

    ArchiveFileEntryPtr getFileEntry() const;
    void setFileEntry(ArchiveFileEntryPtr file_entry);

private:
    ArchiveFileEntryPtr m_file_entry;
    ArchiveFileEntryPtrMapPtr m_entry_map;
    std::string m_base_path;
    std::string m_base_virt_path;
};

class ExtractAllProgressCallback : public BaseProgressCallback
{
public:
    ExtractAllProgressCallback(ArchiveCallbackType callback_type = EXTRACT_ALL_PROGRESS_CALLBACK);
    virtual ~ExtractAllProgressCallback();

    filesystem::FilePtr getDirectory() const;
    void setDirectory(filesystem::FilePtr directory);

    void startedExtractingFile(unsigned long current_file_size);
    void extractedPartOfFile(unsigned long bytes_decompressed);
    void finishedExtractingFile();

    double getCurrentFileProgress() const;
    double getOverallProgress() const;

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr);

    void setExpectedDecompressedSize(unsigned long exp_dec_size);
    unsigned long getExpectedDecompressedSize() const;

    void setNumberOfFilesToExtract(unsigned long files_count);
    unsigned long getNumberOfFilesToExtract() const;

private:
    void updateOverallProgress(unsigned long bytes_decompressed);

    filesystem::FilePtr m_directory;

    //
    // Constant values set before extracting entries:
    //
    unsigned long m_files_to_extract;
    unsigned long m_expected_decompressed_size;

    //
    // Values updated during extraction
    //

    unsigned long m_current_file_size;
    unsigned long m_current_file_extracted_bytes;
    unsigned long m_files_extracted;

    double m_progress_overall;
    unsigned long m_overall_decompressed;
};

class ExtractEntryProgressCallback : public ExtractAllProgressCallback
{
public:
    ExtractEntryProgressCallback();
    virtual ~ExtractEntryProgressCallback();

    ArchiveFileEntryPtr getArchiveFileEntry();
    void setArchiveFileEntry(ArchiveFileEntryPtr afentry);

    void setStripName(bool strip_name);
    bool getStripName() const;

    void setStripBasePath(const std::string& strip_base_path);
    const std::string& getStripBasePath() const;

    virtual void executeOperation(ArchiveFilePtr archive_file_ptr);

private:
    ArchiveFileEntryPtr m_archive_file_entry;
    bool m_strip_name;
    std::string m_strip_base_path;
};

class OperationCanceledException {
public:
    OperationCanceledException(const char* message = "Operation Canceled");
};

}
}

#endif //__ARCHIVE_CALLBACK_DATA_H__
