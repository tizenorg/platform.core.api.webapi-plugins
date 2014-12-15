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
#ifndef _TIZEN_ARCHIVE_ARCHIVE_FILE_H_
#define _TIZEN_ARCHIVE_ARCHIVE_FILE_H_

#include <memory>
#include <string>
#include <deque>
#include <File.h> // from filesystem

#include "ArchiveCallbackData.h"
#include "UnZip.h"
#include "Zip.h"
#include "ArchiveManager.h"


namespace DeviceAPI {
namespace Archive {

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

    long getEntries(GetEntriesCallbackData* callback);
    long getEntryByName(GetEntryByNameCallbackData* callback);
    long extractAll(ExtractAllProgressCallback *callback);
    long add(AddProgressCallback *callback);
    void close();

    Filesystem::FilePtr getFile() const;
    void setFile(Filesystem::FilePtr file);
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
    long extractEntryTo(ExtractEntryProgressCallback* callback);

    bool isAllowedOperation(const std::string& method_name);
    FileMode getFileMode() const;
    void setFileMode(FileMode file_mode);

    /**
     *  \brief Throw InvalidStateError in case when ArchiveFile is closed
     */
    void throwInvalidStateErrorIfArchiveFileIsClosed() const;

    void setCreatedAsNewEmptyArchive(bool new_and_empty);
    bool isCreatedAsNewEmptyArchive() const;


    void updateListOfEntries();
private:
    UnZipPtr createUnZipObject();
    ZipPtr createZipObject();

    std::deque<CallbackPair> m_task_queue;
    std::mutex m_mutex;

    Filesystem::FilePtr m_file;

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
    long addOperation(OperationCallbackData* callback);
    static gboolean callErrorCallback(void* data);

    void extractAllTask(ExtractAllProgressCallback* callback);

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

} // Archive
} // DeviceAPI

#endif /* _TIZEN_ARCHIVE_FILE_ENTRY_H_ */
