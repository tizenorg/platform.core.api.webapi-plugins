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

#ifndef _TIZEN_ARCHIVE_ARCHIVE_FILE_H_
#define _TIZEN_ARCHIVE_ARCHIVE_FILE_H_

#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <map>

#include "archive_callback_data.h"
#include "filesystem_file.h"
#include "un_zip.h"
#include "zip.h"
#include "archive_file_entry.h"

namespace extension {
namespace archive {

class ArchiveFile;
class ArchiveManager;
class OperationCallbackData;
class OpenCallbackData;
class GetEntriesCallbackData;
class GetEntryByNameCallbackData;
class BaseProgressCallback;
class AddProgressCallback;
class ExtractAllProgressCallback;
class UnZipExtractRequest;
class ExtractEntryProgressCallback;
class Zip;
class ZipAddRequest;

enum FileMode {
    READ = 0,
    WRITE,
    READ_WRITE,
    ADD
};

enum IsAllowed {
    NOT_ALLOWED = false,
    ALLOWED = true
};

struct Permission {
    Permission(bool r, bool w, bool rw, bool a);
    bool permission[4];
};

typedef std::shared_ptr<ArchiveFile> ArchiveFilePtr;
typedef std::pair<long, OperationCallbackData*> CallbackPair;
typedef std::vector<ArchiveFileEntryPtr> ArchiveFileEntryPtrVector;
typedef std::map<std::string, Permission> PermissionMap;
typedef std::pair<std::string, Permission> PermissionPair;

struct ArchiveFileHolder{
    ArchiveFilePtr ptr;
};

class ArchiveFile : public std::enable_shared_from_this<ArchiveFile> {
public:
    ArchiveFile();
    ArchiveFile(FileMode file_mode);
    virtual ~ArchiveFile();

    common::PlatformResult getEntries(GetEntriesCallbackData* callback);
    common::PlatformResult getEntryByName(GetEntryByNameCallbackData* callback);
    common::PlatformResult extractAll(ExtractAllProgressCallback *callback);
    common::PlatformResult add(AddProgressCallback *callback);
    void close();

    filesystem::FilePtr getFile() const;
    void setFile(filesystem::FilePtr file);
    bool isOverwrite() const;
    void setOverwrite(bool overwrite);
    unsigned long getDecompressedSize() const;
    void setDecompressedSize(unsigned long decompressed_size);
    bool isOpen() const;
    void setIsOpen(bool is_open);

    ArchiveFileEntryPtrMapPtr getEntryMap() const;
    void setEntryMap(ArchiveFileEntryPtrMapPtr entries);
    bool isEntryWithNameInArchive(const std::string& name_in_zip,
            bool* out_is_directory = NULL,
            std::string* out_matching_name = NULL);

    //Used by ArchiveFileEntry
    common::PlatformResult extractEntryTo(ExtractEntryProgressCallback* callback);

    bool isAllowedOperation(const std::string& method_name);
    FileMode getFileMode() const;
    void setFileMode(FileMode file_mode);

    void setCreatedAsNewEmptyArchive(bool new_and_empty);
    bool isCreatedAsNewEmptyArchive() const;

    PlatformResult updateListOfEntries();
private:
    PlatformResult createUnZipObject(UnZipPtr* unzip);
    PlatformResult createZipObject(ZipPtr* zip);

    std::deque<CallbackPair> m_task_queue;
    std::mutex m_mutex;

    filesystem::FilePtr m_file;

    FileMode m_file_mode;
    static PermissionMap s_permission_map;

    unsigned long m_decompressed_size;
    bool m_is_open;
    /**
     * If set to true, during decompression archive will had permission to overwriting files.
     * Warning: If decompressing file have got the same name as existing directory
     * in place where file should be decompressed, directory will be deleted.
    */
    bool m_overwrite;
    ArchiveFileEntryPtrMapPtr m_entry_map;

    /**
     * If we execute tizen.open(destFile , "w"/"rw"/ "a", ..., {overwrite: true}),
     * destFile will be empty file with size = 0, which cannot be
     * opened with minizip library.
     *
     * Zip file format restricts that at least one file / folder is compressed,
     * threfore after creating new empty archive we cannot save it.
     * Until we execute archive.add destFile is empty file with size 0 bytes.
     *
     * Unfortunately if we try to execute archive.getEntries or archive.extractAll
     * minizip library will fail to open empty archive. To fix this issue we will
     * use flag "created_as_new_empty_archive" which informs that this is new and
     * empty archive threfore we should not try to open it.
     *
     * In extractAll we will just call success callback - there was nothing to extract
     * but no error occured. In get entries we just should return empty list of entries.
     */
    bool m_created_as_new_empty_archive;

    static gboolean openTaskCompleteCB(void *data);
    static gboolean getEntriesTaskCompleteCB(void *data);
    static gboolean getEntryByNameTaskCompleteCB(void *data);

    static void* taskManagerThread(void *data);
    common::PlatformResult addOperation(OperationCallbackData* callback);
    static gboolean callErrorCallback(void* data);

    PlatformResult extractAllTask(ExtractAllProgressCallback* callback);

    friend class ExtractAllProgressCallback;
    friend class UnZipExtractRequest;
    friend class OpenCallbackData;
    friend class GetEntriesCallbackData;
    friend class GetEntryByNameCallbackData;
    friend class ExtractEntryProgressCallback;
    friend class ArchiveManager;
    friend class AddProgressCallback;
    friend class Zip;
    friend class ZipAddRequest;
    friend class BaseProgressCallback;
};

} // archive
} // extension

#endif /* _TIZEN_ARCHIVE_FILE_ENTRY_H_ */
