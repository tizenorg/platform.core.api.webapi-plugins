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
#include "ArchiveFile.h"

#include <Logger.h>
#include <PlatformException.h>
#include <GlobalContextManager.h>
#include <Path.h>

#include "ArchiveManager.h"
#include "JSArchiveFileEntry.h"
#include "JSArchiveFile.h"
#include "ArchiveUtils.h"
#include "plugin_config_impl.h"
#include "UnZip.h"
#include "Zip.h"

namespace DeviceAPI {
namespace Archive {

using namespace Common;

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
    LOGD("Entered");
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
    LOGD("Entered");

    if(m_entry_map) {
        LOGD("Unlinking old m_entry_map: %d ArchiveFileEntries", m_entry_map->size());
        for(auto it = m_entry_map->begin(); it != m_entry_map->end(); ++it) {
            if(it->second) {
                it->second->setArchiveFileNonProtectPtr(NULL);
            }
        }
    }
}

gboolean ArchiveFile::openTaskCompleteCB(void *data)
{
    LOGD("Entered");
    auto callback = static_cast<OperationCallbackData*>(data);
    if (!callback) {
        LOGE("callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }
    try {
        if (callback->isError()) {
            JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(errobj);
        }
        else {
            JSObjectRef result = JSArchiveFile::makeJSObject(context,
                    callback->getArchiveFile());
            callback->callSuccessCallback(result);
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("%s (%s)", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unknown error occurs");
    }

    delete callback;
    callback = NULL;

    return false;
}

gboolean ArchiveFile::callErrorCallback(void* data)
{
    LOGD("Entered");
    auto callback = static_cast<OperationCallbackData*>(data);
    if (!callback) {
        LOGE("callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }

    try {
        if (callback->isError()) {
            JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(errobj);
        }
        else {
            LOGW("The success callback should be not be called in this case");
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("%s (%s)", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unknown error occurs");
    }

    delete callback;
    callback = NULL;

    return false;
}

void* ArchiveFile::taskManagerThread(void *data)
{
    LOGD("Entered");
    ArchiveFileHolder* archive_file_holder = static_cast<ArchiveFileHolder*>(data);
    if (!archive_file_holder) {
        LOGE("archive_file_holder is null");
        return NULL;
    }

    if (!archive_file_holder->ptr){
        LOGE("archive_file is null");
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
        } catch (const BasePlatformException &err) {
            LOGE("taskManagerThread fails, %s: %s", err.getName().c_str(),
                    err.getMessage().c_str());
            callback->setError(err.getName().c_str(), err.getMessage().c_str());
            call_error_callback = true;
        } catch (...) {
            LOGE("taskManagerThread fails");
            callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR, "UnknownError.");
            call_error_callback = true;
        }
        if(call_error_callback) {
            {
                std::lock_guard<std::mutex> lock(archive_file_holder->ptr->m_mutex);
                archive_file_holder->ptr->m_task_queue.pop_back();
            }
            if (!g_idle_add(callErrorCallback, static_cast<void*>(callback))) {
                LOGE("g_idle_add fails");
                delete callback;
                callback = NULL;
            }
        }
    }

    delete archive_file_holder;
    archive_file_holder = NULL;

    return NULL;
}

long ArchiveFile::addOperation(OperationCallbackData* callback)
{
    LOGD("Entered callback type:%d", callback->getCallbackType());

    const long operation_id =
            ArchiveManager::getInstance().getNextOperationId(shared_from_this());
    callback->setOperationId(operation_id);
    callback->setArchiveFile(shared_from_this());
    std::size_t size = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task_queue.push_front(CallbackPair(operation_id, callback));
        size = m_task_queue.size();
    }
    if(1 == size){
        pthread_t thread;
        ArchiveFileHolder* holder = new(std::nothrow) ArchiveFileHolder();
        if(!holder){
            LOGE("Memory allocation error");
            throw UnknownException("Memory allocation error");
        }
        holder->ptr = shared_from_this();
        if (pthread_create(&thread, NULL, taskManagerThread,
                static_cast<void*>(holder))) {
            LOGE("Thread creation failed");
            delete holder;
            holder = NULL;
            throw UnknownException("Thread creation failed");
        }

        if (pthread_detach(thread)) {
            LOGE("Thread detachment failed");
        }
    }
    return operation_id;
}

void ArchiveFile::extractAllTask(ExtractAllProgressCallback* callback)
{
    Filesystem::FilePtr directory = callback->getDirectory();

    if(!directory) {
        LOGE("Directory is null");
        throw UnknownException("Directory is null");
    } else {
        if(!directory->getNode()){
            LOGE("Node in directory is null");
            throw UnknownException("Node is null");
        }
    }

    if(!m_file) {
        LOGE("File is null");
        throw UnknownException("File is null");
    } else {
        if(!m_file->getNode()){
            LOGE("Node in file is null");
            throw UnknownException("Node in file is null");
        }
    }

    // For explanation please see:
    //    ArchiveFile.h m_created_as_new_empty_archive description
    //
    if(m_file->getNode()->getSize() == 0) {
        LOGD("Zip file: %s is empty",
                m_file->getNode()->getPath()->getFullPath().c_str());

        if(m_created_as_new_empty_archive) {
            //We do not call progress callback since we do not have any ArchiveFileEntry
            callback->callSuccessCallbackOnMainThread();
            callback = NULL;
            return;
        }
        else {
            LOGW("m_created_as_new_empty_archive is false");
            LOGE("Throwing InvalidStateException: File is not valid ZIP archive");
            throw InvalidStateException("File is not valid ZIP archive");
        }
    }

    UnZipPtr unzip = createUnZipObject();
    unzip->extractAllFilesTo(directory->getNode()->getPath()->getFullPath(), callback);
}

long ArchiveFile::getEntries(GetEntriesCallbackData* callback)
{
    LOGD("Entered");
    if(!callback) {
        LOGE("callback is NULL");
        throw UnknownException("Could not get list of files in archive");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    return addOperation(callback);
}

gboolean ArchiveFile::getEntriesTaskCompleteCB(void *data)
{
    auto callback = static_cast<GetEntriesCallbackData*>(data);
    if (!callback) {
        LOGE("callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }

    try {
        ArchiveFileEntryPtrMapPtr entries = callback->getEntries();
        unsigned int size = entries->size();

        JSObjectRef objArray[size];
        int i = 0;
        for(auto it = entries->begin(); it != entries->end(); it++) {
            objArray[i] = JSArchiveFileEntry::makeJSObject(context, it->second);
            i++;
        }

        JSValueRef exception = NULL;
        JSObjectRef jsResult = JSObjectMakeArray(context, size,
                size > 0 ? objArray : NULL, &exception);
        if (exception != NULL) {
            throw Common::UnknownException(context, exception);
        }

        callback->callSuccessCallback(jsResult);
    }
    catch (const BasePlatformException &err) {
        LOGE("%s (%s)", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unknown error occurs");
    }

    delete callback;
    callback = NULL;

    return false;
}

long ArchiveFile::extractAll(ExtractAllProgressCallback *callback)
{
    LOGD("Entered");
    if(!callback) {
        LOGE("callback is NULL");
        throw UnknownException("Could not extract all files from archive");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    return addOperation(callback);
}

long ArchiveFile::extractEntryTo(ExtractEntryProgressCallback* callback)
{
    LOGD("Entered");
    if(!callback) {
        LOGE("callback is NULL");
        throw UnknownException("Could not extract archive file entry");
    }

    // FIXME according to documentation:
    // if archive was closed, any further operation attempt will make InvalidStateError
    // but method extract() from ArchiveFileEntryObject is not permitted to throw above exception

    // uncomment in case when this method have permission to throwing InvalidStateError
    // throwInvalidStateErrorIfArchiveFileisClosed();
    if(!m_is_open) {
        LOGE("Archive is not opened");
        throw UnknownException("Archive is not opened");
    }

    return addOperation(callback);
}


long ArchiveFile::add(AddProgressCallback *callback)
{
    LOGD("Entered");
    if(!callback) {
        LOGE("callback is NULL");
        throw UnknownException("Could not add file to archive");
    }
    if(FileMode::READ == m_file_mode) {
        LOGE("Trying to add file when READ access mode selected");
        throw InvalidAccessException("Add not allowed for \"r\" access mode");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    return addOperation(callback);
}

void ArchiveFile::close()
{
    LOGD("Entered");

    if(!m_is_open){
        LOGD("Archive already closed");
    }
    m_is_open = false;

    return;
}

long ArchiveFile::getEntryByName(GetEntryByNameCallbackData* callback)
{
    LOGD("Entered");
    if(!callback) {
        LOGE("callback is NULL");
        throw UnknownException("Could not get archive file entries by name");
    }

    throwInvalidStateErrorIfArchiveFileIsClosed();

    return addOperation(callback);
}

gboolean ArchiveFile::getEntryByNameTaskCompleteCB(void *data)
{
    auto callback = static_cast<GetEntryByNameCallbackData*>(data);
    if (!callback) {
        LOGE("callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }
    try {
        if (callback->isError()) {
            JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(errobj);
        }
        else {
            JSObjectRef entry = JSArchiveFileEntry::makeJSObject(context,
                    callback->getFileEntry());
            callback->callSuccessCallback(entry);
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("%s (%s)", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unknown error occurs");
    }

    delete callback;
    callback = NULL;

    return false;
}

Filesystem::FilePtr ArchiveFile::getFile() const
{
    LOGD("Entered");
    return m_file;
}

void ArchiveFile::setFile(Filesystem::FilePtr file)
{
    LOGD("Entered");
    m_file = file;
}

bool ArchiveFile::isOverwrite() const
{
    return m_overwrite;
}

void ArchiveFile::setOverwrite(bool overwrite)
{
    LOGD("Entered");
    m_overwrite = overwrite;
}

unsigned long ArchiveFile::getDecompressedSize() const
{
    LOGD("Entered");
    return m_decompressed_size;
}

void ArchiveFile::setDecompressedSize(unsigned long decompressed_size)
{
    LOGD("Entered");
    m_decompressed_size = decompressed_size;
}

bool ArchiveFile::isOpen() const
{
    LOGD("Entered");
    return m_is_open;
}

void ArchiveFile::setIsOpen(bool is_open)
{
    LOGD("Entered");
    m_is_open = is_open;
}

ArchiveFileEntryPtrMapPtr ArchiveFile::getEntryMap() const
{
    return m_entry_map;
}

void ArchiveFile::setEntryMap(ArchiveFileEntryPtrMapPtr entries)
{
    LOGD("Entered");

    if(m_entry_map) {
        LOGD("Unlinking old m_entry_map: %d ArchiveFileEntries", m_entry_map->size());
        for(auto it = m_entry_map->begin(); it != m_entry_map->end(); ++it) {
            if(it->second) {
                it->second->setArchiveFileNonProtectPtr(NULL);
            }
        }
    }

    m_entry_map = entries;

    LOGD("Linking new m_entry_map ArchiveFileEntries (%d) with ArchiveFile object",
            m_entry_map->size());
    for(auto it = m_entry_map->begin(); it != m_entry_map->end(); ++it) {
        if(it->second) {
            it->second->setArchiveFileNonProtectPtr(this);
        }
    }
}

UnZipPtr ArchiveFile::createUnZipObject()
{
    LOGD("Entered");
    if(!m_is_open) {
        LOGE("File is not opened");
        throw UnknownException("File is not opened");
    }

    if (!m_file) {
        LOGE("m_file is null");
        throw UnknownException("File is null");
    }

    Filesystem::NodePtr node = m_file->getNode();
    if(!node) {
        LOGE("Node is null");
        throw UnknownException("Node is null");
    }

    UnZipPtr unzip = UnZip::open(node->getPath()->getFullPath());
    return unzip;
}

ZipPtr ArchiveFile::createZipObject()
{
    LOGD("Entered");
    if(!m_is_open) {
        LOGE("File is not opened");
        throw UnknownException("File is not opened");
    }

    if (!m_file) {
        LOGE("m_file is null");
        throw UnknownException("File is null");
    }

    Filesystem::NodePtr node = m_file->getNode();
    if(!node) {
        LOGE("Node is null");
        throw UnknownException("Node is null");
    }

    ZipPtr zip = Zip::open(node->getPath()->getFullPath());
    return zip;

}

bool ArchiveFile::isAllowedOperation(const std::string& method_name)
{
    LOGD("Entered");
    PermissionMap::iterator it = s_permission_map.find(method_name);
    if (it != s_permission_map.end()) {
        return it->second.permission[m_file_mode];
    }
    return false;
}

FileMode ArchiveFile::getFileMode() const
{
   LOGD("Entered");
   return m_file_mode;
}

void ArchiveFile::setFileMode(FileMode file_mode)
{
    LOGD("Entered");
    m_file_mode = file_mode;
}

void ArchiveFile::throwInvalidStateErrorIfArchiveFileIsClosed() const
{
    if(!m_is_open){
        LOGE("ArchiveFile closed - operation not permitted");
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
        LOGD("Zip file: %s is empty",
                m_file->getNode()->getPath()->getFullPath().c_str());

        if(m_created_as_new_empty_archive) {
            LOGD("OK this is empty archive = nothing to do yet");
            return;
        }
        else {
            LOGW("m_created_as_new_empty_archive is false");
            LOGE("Throwing InvalidStateException: File is not valid ZIP archive");
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
        LOGW("m_entry_map is NULL");
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

} // Archive
} // DeviceAPI
