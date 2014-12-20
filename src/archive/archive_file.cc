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
#include "archive_file.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

#include "archive_manager.h"
#include "archive_utils.h"
#include "defs.h"
#include "un_zip.h"
#include "zip.h"
#include "archive_instance.h"

using namespace common;

namespace extension {
namespace archive {

Permission::Permission(bool r, bool w, bool rw, bool a){
    permission[0] = r;
    permission[1] = w;
    permission[2] = rw;
    permission[3] = a;
}

PermissionMap ArchiveFile::s_permission_map = {
        {ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD,
                Permission(NOT_ALLOWED, ALLOWED, ALLOWED, ALLOWED)},
        {ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL,
                Permission(ALLOWED, NOT_ALLOWED, ALLOWED, NOT_ALLOWED)},
        {ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES,
                Permission(ALLOWED, NOT_ALLOWED, ALLOWED, NOT_ALLOWED)},
        {ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME,
                Permission(ALLOWED, NOT_ALLOWED, ALLOWED, NOT_ALLOWED)}
};

ArchiveFile::ArchiveFile() :
        enable_shared_from_this<ArchiveFile>(),
        m_decompressed_size(0),
        m_is_open(false),
        m_overwrite(false),
        m_created_as_new_empty_archive(false)
{
    LoggerD("Entered");
}

ArchiveFile::ArchiveFile(FileMode file_mode) :
        enable_shared_from_this<ArchiveFile>(),
        m_decompressed_size(0),
        m_is_open(false),
        m_overwrite(false)
{
    m_file_mode = file_mode;
}

ArchiveFile::~ArchiveFile()
{
    LoggerD("Entered");

    if(m_entry_map) {
        LoggerD("Unlinking old m_entry_map: %d ArchiveFileEntries", m_entry_map->size());
        for(auto it = m_entry_map->begin(); it != m_entry_map->end(); ++it) {
            if(it->second) {
                it->second->setArchiveFileNonProtectPtr(NULL);
            }
        }
    }
}

gboolean ArchiveFile::openTaskCompleteCB(void *data)
{
    LoggerD("Entered");

    auto callback = static_cast<OperationCallbackData*>(data);
    if (!callback) {
        LoggerE("callback is null");
        return false;
    }

    try {
        auto archive_file = callback->getArchiveFile();

        picojson::value val = picojson::value(picojson::object());
        picojson::object& obj = val.get<picojson::object>();
        obj[JSON_CALLBACK_ID] = picojson::value(callback->getCallbackId());
        obj[JSON_DATA] = picojson::value(picojson::object());

        picojson::object& args = obj[JSON_DATA].get<picojson::object>();

        if (!callback->isError()) {
            long handle = ArchiveManager::getInstance().addPrivData(archive_file);

            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            args[ARCHIVE_FILE_ATTR_MODE] = picojson::value(
                    fileModeToString(archive_file->getFileMode()));
            args[ARCHIVE_FILE_ATTR_DECOMPRESSED_SIZE] = picojson::value();
            args[ARCHIVE_FILE_HANDLE] = picojson::value(static_cast<double>(handle));
        } else {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

            args[ERROR_CALLBACK_NAME] = picojson::value(callback->getErrorName());
            args[ERROR_CALLBACK_MESSAGE] = picojson::value(callback->getErrorMessage());
        }

        LoggerD("%s", val.serialize().c_str());

        ArchiveInstance::getInstance().PostMessage(val.serialize().c_str());
    }
    catch (const PlatformException& ex) {
        LoggerE("%s (%s)", ex.name().c_str(), ex.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occurred");
    }

    delete callback;
    callback = NULL;

    return false;
}

gboolean ArchiveFile::callErrorCallback(void* data)
{
    LoggerD("Entered");

    auto callback = static_cast<OperationCallbackData*>(data);
    if (!callback) {
        LoggerE("callback is null");
        return false;
    }

    try {
        picojson::value val = picojson::value(picojson::object());
        picojson::object& obj = val.get<picojson::object>();
        obj[JSON_CALLBACK_ID] = picojson::value(callback->getCallbackId());
        obj[JSON_DATA] = picojson::value(picojson::object());

        picojson::object& args = obj[JSON_DATA].get<picojson::object>();

        if (!callback->isError()) {
            LoggerW("The success callback should be not be called in this case");
        } else {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

            args[ERROR_CALLBACK_NAME] = picojson::value(callback->getErrorName());
            args[ERROR_CALLBACK_MESSAGE] = picojson::value(callback->getErrorMessage());
        }

        LoggerD("%s", val.serialize().c_str());

        ArchiveInstance::getInstance().PostMessage(val.serialize().c_str());
    }
    catch (const PlatformException& ex) {
        LoggerE("%s (%s)", ex.name().c_str(), ex.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occured");
    }

    delete callback;
    callback = NULL;

    return false;
}

void* ArchiveFile::taskManagerThread(void *data)
{
    LoggerD("Entered");
    ArchiveFileHolder* archive_file_holder = static_cast<ArchiveFileHolder*>(data);
    if (!archive_file_holder) {
        LoggerE("archive_file_holder is null");
        return NULL;
    }

    if (!archive_file_holder->ptr){
        LoggerE("archive_file is null");
        delete archive_file_holder;
        archive_file_holder = NULL;
        return NULL;
    }

    while(true){
        OperationCallbackData* callback = NULL;
        bool call_error_callback = false;
        try{
            {
                std::lock_guard<std::mutex> lock(archive_file_holder->ptr->m_mutex);
                if(archive_file_holder->ptr->m_task_queue.empty()){
                    break;
                }
                callback = archive_file_holder->ptr->m_task_queue.back().second;
            }
            if(callback && !callback->isCanceled()){
                callback->executeOperation(archive_file_holder->ptr);
            }
            {
                std::lock_guard<std::mutex> lock(archive_file_holder->ptr->m_mutex);
                archive_file_holder->ptr->m_task_queue.pop_back();
            }
        } catch (const OperationCanceledException &err) {
            {
                std::lock_guard<std::mutex> lock(archive_file_holder->ptr->m_mutex);
                archive_file_holder->ptr->m_task_queue.pop_back();
            }
            delete callback;
            callback = NULL;
        } catch (const PlatformException &err) {
            LoggerE("taskManagerThread fails, %s: %s", err.name().c_str(),
                    err.message().c_str());
            callback->setError(err.name().c_str(), err.message().c_str());
            call_error_callback = true;
        } catch (...) {
            LoggerE("taskManagerThread fails");
            callback->setError("UnknownError", "UnknownError");
            call_error_callback = true;
        }
        if(call_error_callback) {
            {
                std::lock_guard<std::mutex> lock(archive_file_holder->ptr->m_mutex);
                archive_file_holder->ptr->m_task_queue.pop_back();
            }
            if (!g_idle_add(callErrorCallback, static_cast<void*>(callback))) {
                LoggerE("g_idle_add fails");
                delete callback;
                callback = NULL;
            }
        }
    }

    delete archive_file_holder;
    archive_file_holder = NULL;

    return NULL;
}

void ArchiveFile::addOperation(OperationCallbackData* callback)
{
    LoggerD("Entered callback type:%d", callback->getCallbackType());

    const long operation_id = callback->getOperationId();
    callback->setArchiveFile(shared_from_this());
    std::size_t size = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task_queue.push_front(CallbackPair(operation_id, callback));
        size = m_task_queue.size();
    }
    if(1 == size) {
        pthread_t thread;
        ArchiveFileHolder* holder = new(std::nothrow) ArchiveFileHolder();
        if(!holder) {
            LoggerE("Memory allocation error");
            throw UnknownException("Memory allocation error");
        }
        holder->ptr = shared_from_this();
        if (pthread_create(&thread, NULL, taskManagerThread,
                static_cast<void*>(holder))) {
            LoggerE("Thread creation failed");
            delete holder;
            holder = NULL;
            throw UnknownException("Thread creation failed");
        }

        if (pthread_detach(thread)) {
            LoggerE("Thread detachment failed");
        }
    }
}

void ArchiveFile::extractAllTask(ExtractAllProgressCallback* callback)
{
    filesystem::FilePtr directory = callback->getDirectory();

    if(!directory) {
        LoggerE("Directory is null");
        throw UnknownException("Directory is null");
    } else {
        if(!directory->getNode()){
            LoggerE("Node in directory is null");
            throw UnknownException("Node is null");
        }
    }

    if(!m_file) {
        LoggerE("File is null");
        throw UnknownException("File is null");
    } else {
        if(!m_file->getNode()){
            LoggerE("Node in file is null");
            throw UnknownException("Node in file is null");
        }
    }

    // For explanation please see:
    //    ArchiveFile.h m_created_as_new_empty_archive description
    //
    if(m_file->getNode()->getSize() == 0) {
        LoggerD("Zip file: %s is empty",
                m_file->getNode()->getPath()->getFullPath().c_str());

        if(m_created_as_new_empty_archive) {
            //We do not call progress callback since we do not have any ArchiveFileEntry
            callback->callSuccessCallbackOnMainThread();
            callback = NULL;
            return;
        }
        else {
            LoggerW("m_created_as_new_empty_archive is false");
            LoggerE("Throwing InvalidStateException: File is not valid ZIP archive");
            throw InvalidStateException("File is not valid ZIP archive");
        }
    }

    UnZipPtr unzip = createUnZipObject();
    unzip->extractAllFilesTo(directory->getNode()->getPath()->getFullPath(), callback);
}

void ArchiveFile::getEntries(GetEntriesCallbackData* callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("callback is NULL");
        throw UnknownException("Could not get list of files in archive");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    addOperation(callback);
}

gboolean ArchiveFile::getEntriesTaskCompleteCB(void *data)
{
    LoggerD("Entered");
    LoggerW("STUB Not calling success/error callback");

    auto callback = static_cast<GetEntriesCallbackData*>(data);
    if (!callback) {
        LoggerE("callback is null");
        return false;
    }

    try {
        picojson::value val = picojson::value(picojson::object());
        picojson::object& obj = val.get<picojson::object>();
        obj[JSON_CALLBACK_ID] = picojson::value(callback->getCallbackId());

        if (!callback->isError()) {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);
            obj[JSON_DATA] = picojson::value(picojson::array());
            picojson::array &arr = obj[JSON_DATA].get<picojson::array>();

            ArchiveFileEntryPtrMapPtr entries = callback->getEntries();
            for(auto it = entries->begin(); it != entries->end(); it++) {
                picojson::value val = picojson::value(picojson::object());
                picojson::object& obj = val.get<picojson::object>();

                obj[ARCHIVE_FILE_ENTRY_ATTR_NAME] = picojson::value(
                        it->second->getName());
                obj[ARCHIVE_FILE_ENTRY_ATTR_SIZE] = picojson::value(
                        static_cast<double>(it->second->getSize()));
                obj[ARCHIVE_FILE_ENTRY_ATTR_MODIFIED] = picojson::value(
                        static_cast<double>(it->second->getModified()));
                obj[ARCHIVE_FILE_ENTRY_ATTR_COMPRESSED_SIZE] = picojson::value(
                        static_cast<double>(it->second->getCompressedSize()));
                obj[ARCHIVE_FILE_HANDLE] = picojson::value(
                        static_cast<double>(callback->getHandle()));

                arr.push_back(val);
            }
        } else {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
            obj[JSON_DATA] = picojson::value(picojson::object());
            picojson::object& args = obj[JSON_DATA].get<picojson::object>();

            args[ERROR_CALLBACK_NAME] = picojson::value(callback->getErrorName());
            args[ERROR_CALLBACK_MESSAGE] = picojson::value(callback->getErrorMessage());
        }

        LoggerD("%s", val.serialize().c_str());

        ArchiveInstance::getInstance().PostMessage(val.serialize().c_str());
    }
    catch (const PlatformException& ex) {
        LoggerE("%s (%s)", ex.name().c_str(), ex.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occured");
    }

    delete callback;
    callback = NULL;

    return false;
}

void ArchiveFile::extractAll(ExtractAllProgressCallback *callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("callback is NULL");
        throw UnknownException("Could not extract all files from archive");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    addOperation(callback);
}

void ArchiveFile::extractEntryTo(ExtractEntryProgressCallback* callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("callback is NULL");
        throw UnknownException("Could not extract archive file entry");
    }

    // FIXME according to documentation:
    // if archive was closed, any further operation attempt will make InvalidStateError
    // but method extract() from ArchiveFileEntryObject is not permitted to throw above exception

    // uncomment in case when this method have permission to throwing InvalidStateError
    // throwInvalidStateErrorIfArchiveFileisClosed();
    if(!m_is_open) {
        LoggerE("Archive is not opened");
        throw UnknownException("Archive is not opened");
    }

    addOperation(callback);
}


void ArchiveFile::add(AddProgressCallback *callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("callback is NULL");
        throw UnknownException("Could not add file to archive");
    }
    if(FileMode::READ == m_file_mode) {
        LoggerE("Trying to add file when READ access mode selected");
        throw InvalidAccessException("Add not allowed for \"r\" access mode");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    addOperation(callback);
}

void ArchiveFile::close()
{
    LoggerD("Entered");

    if(!m_is_open){
        LoggerD("Archive already closed");
    }
    m_is_open = false;

    return;
}

void ArchiveFile::getEntryByName(GetEntryByNameCallbackData* callback)
{
    LoggerD("Entered");
    if(!callback) {
        LoggerE("callback is NULL");
        throw UnknownException("Could not get archive file entries by name");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    addOperation(callback);
}

gboolean ArchiveFile::getEntryByNameTaskCompleteCB(void *data)
{
    LoggerD("Entered");

    auto callback = static_cast<GetEntryByNameCallbackData*>(data);
    if (!callback) {
        LoggerE("callback is null");
        return false;
    }

    try {
        picojson::value val = picojson::value(picojson::object());
        picojson::object& obj = val.get<picojson::object>();
        obj[JSON_CALLBACK_ID] = picojson::value(callback->getCallbackId());
        obj[JSON_DATA] = picojson::value(picojson::object());
        picojson::object& args = obj[JSON_DATA].get<picojson::object>();

        if (!callback->isError()) {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            ArchiveFileEntryPtr ent = callback->getFileEntry();

            args[ARCHIVE_FILE_ENTRY_ATTR_NAME] = picojson::value(ent->getName());
            args[ARCHIVE_FILE_ENTRY_ATTR_SIZE] = picojson::value(
                    static_cast<double>(ent->getSize()));
            args[ARCHIVE_FILE_ENTRY_ATTR_MODIFIED] = picojson::value(
                    static_cast<double>(ent->getModified()));
            args[ARCHIVE_FILE_ENTRY_ATTR_COMPRESSED_SIZE] = picojson::value(
                    static_cast<double>(ent->getCompressedSize()));
            args[ARCHIVE_FILE_HANDLE] = picojson::value(
                    static_cast<double>(callback->getHandle()));
        } else {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

            args[ERROR_CALLBACK_NAME] = picojson::value(callback->getErrorName());
            args[ERROR_CALLBACK_MESSAGE] = picojson::value(callback->getErrorMessage());
        }

        LoggerD("%s", val.serialize().c_str());

        ArchiveInstance::getInstance().PostMessage(val.serialize().c_str());
    }
    catch (const PlatformException& ex) {
        LoggerE("%s (%s)", ex.name().c_str(), ex.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occured");
    }

    delete callback;
    callback = NULL;

    return false;
}

filesystem::FilePtr ArchiveFile::getFile() const
{
    LoggerD("Entered");
    return m_file;
}

void ArchiveFile::setFile(filesystem::FilePtr file)
{
    LoggerD("Entered");
    m_file = file;
}

bool ArchiveFile::isOverwrite() const
{
    return m_overwrite;
}

void ArchiveFile::setOverwrite(bool overwrite)
{
    LoggerD("Entered");
    m_overwrite = overwrite;
}

unsigned long ArchiveFile::getDecompressedSize() const
{
    LoggerD("Entered");
    return m_decompressed_size;
}

void ArchiveFile::setDecompressedSize(unsigned long decompressed_size)
{
    LoggerD("Entered");
    m_decompressed_size = decompressed_size;
}

bool ArchiveFile::isOpen() const
{
    LoggerD("Entered");
    return m_is_open;
}

void ArchiveFile::setIsOpen(bool is_open)
{
    LoggerD("Entered");
    m_is_open = is_open;
}

ArchiveFileEntryPtrMapPtr ArchiveFile::getEntryMap() const
{
    return m_entry_map;
}

void ArchiveFile::setEntryMap(ArchiveFileEntryPtrMapPtr entries)
{
    LoggerD("Entered");

    if(m_entry_map) {
        LoggerD("Unlinking old m_entry_map: %d ArchiveFileEntries", m_entry_map->size());
        for(auto it = m_entry_map->begin(); it != m_entry_map->end(); ++it) {
            if(it->second) {
                it->second->setArchiveFileNonProtectPtr(NULL);
            }
        }
    }

    m_entry_map = entries;

    LoggerD("Linking new m_entry_map ArchiveFileEntries (%d) with ArchiveFile object",
            m_entry_map->size());
    for(auto it = m_entry_map->begin(); it != m_entry_map->end(); ++it) {
        if(it->second) {
            it->second->setArchiveFileNonProtectPtr(this);
        }
    }
}

UnZipPtr ArchiveFile::createUnZipObject()
{
    LoggerD("Entered");
    if(!m_is_open) {
        LoggerE("File is not opened");
        throw UnknownException("File is not opened");
    }

    if (!m_file) {
        LoggerE("m_file is null");
        throw UnknownException("File is null");
    }

    filesystem::NodePtr node = m_file->getNode();
    if(!node) {
        LoggerE("Node is null");
        throw UnknownException("Node is null");
    }

    UnZipPtr unzip = UnZip::open(m_file->getNode()->getPath()->getFullPath());
    return unzip;
}

ZipPtr ArchiveFile::createZipObject()
{
    LoggerD("Entered");
    if(!m_is_open) {
        LoggerE("File is not opened");
        throw UnknownException("File is not opened");
    }

    if (!m_file) {
        LoggerE("m_file is null");
        throw UnknownException("File is null");
    }

    filesystem::NodePtr node = m_file->getNode();
    if(!node) {
        LoggerE("Node is null");
        throw UnknownException("Node is null");
    }

    ZipPtr zip = Zip::open(m_file->getNode()->getPath()->getFullPath());
    return zip;

}

bool ArchiveFile::isAllowedOperation(const std::string& method_name)
{
    LoggerD("Entered");
    PermissionMap::iterator it = s_permission_map.find(method_name);
    if (it != s_permission_map.end()) {
        return it->second.permission[m_file_mode];
    }
    return false;
}

FileMode ArchiveFile::getFileMode() const
{
   LoggerD("Entered");
   return m_file_mode;
}

void ArchiveFile::setFileMode(FileMode file_mode)
{
    LoggerD("Entered");
    m_file_mode = file_mode;
}

void ArchiveFile::throwInvalidStateErrorIfArchiveFileIsClosed() const
{
    if(!m_is_open){
        LoggerE("ArchiveFile closed - operation not permitted");
        throw InvalidStateException(
            "ArchiveFile closed - operation not permitted");
    }
}

void ArchiveFile::setCreatedAsNewEmptyArchive(bool new_and_empty)
{
    m_created_as_new_empty_archive = new_and_empty;
}

bool ArchiveFile::isCreatedAsNewEmptyArchive() const
{
    return m_created_as_new_empty_archive;
}

void ArchiveFile::updateListOfEntries()
{
    // For explanation please see:
    //    ArchiveFile.h m_created_as_new_empty_archive description
    //
    if(m_file->getNode()->getSize() == 0) {
        LoggerD("Zip file: %s is empty",
                m_file->getNode()->getPath()->getFullPath().c_str());

        if(m_created_as_new_empty_archive) {
            LoggerD("OK this is empty archive = nothing to do yet");
            return;
        }
        else {
            LoggerW("m_created_as_new_empty_archive is false");
            LoggerE("Throwing InvalidStateException: File is not valid ZIP archive");
            throw InvalidStateException("File is not valid ZIP archive");
        }
    }

    UnZipPtr unzip = createUnZipObject();
    unsigned long decompressedSize = 0;
    ArchiveFileEntryPtrMapPtr emap = unzip->listEntries(&decompressedSize);
    setEntryMap(emap);
    setDecompressedSize(decompressedSize);
}

bool ArchiveFile::isEntryWithNameInArchive(const std::string& name_in_zip,
        bool* out_is_directory,
        std::string* out_matching_name)
{
    if(!m_entry_map) {
        LoggerW("m_entry_map is NULL");
        return false;
    }

    const bool name_in_zip_is_dir = isDirectoryPath(name_in_zip);
    bool set_is_directory = false;
    bool set_name_exists = false;

    //Try exact name:
    auto it = m_entry_map->find(name_in_zip);
    if(it != m_entry_map->end()) {
        set_is_directory = name_in_zip_is_dir;
        set_name_exists = true;
    }
    else {
        if(name_in_zip_is_dir) {
            //If name_in_zip is pointing at directory try file
            it = m_entry_map->find(removeTrailingDirectorySlashFromPath(name_in_zip));
            set_is_directory = false;
        } else {
            //If name_in_zip is pointing at file try directory
            it = m_entry_map->find(name_in_zip + "/");
            set_is_directory = true;
        }

        if(it != m_entry_map->end()) {
            set_name_exists = true;
        }
    }

    if(!set_name_exists) {
        return false;
    }

    if(out_is_directory) {
        *out_is_directory = set_is_directory;
    }
    if(out_matching_name) {
        *out_matching_name = it->first;
    }

    return true;
}

} // archive
} // extension
