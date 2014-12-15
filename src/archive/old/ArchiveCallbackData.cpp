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
 * @file        ArchiveCallbackData.cpp
 */

#include <Logger.h>
#include "ArchiveCallbackData.h"
#include <GlobalContextManager.h>
#include <JSUtil.h>
#include <Path.h>
#include <FilesystemExternalUtils.h>
#include "ArchiveFile.h"
#include "JSArchiveFileEntry.h"
#include "ArchiveUtils.h"
#include "JSArchiveFileEntry.h"
#include "UnZip.h"
#include "Zip.h"
#include "Path.h"

namespace DeviceAPI {
namespace Archive {

using namespace DeviceAPI::Common;

namespace {
const char* CALLBACK_SUCCESS = "success";
const char* CALLBACK_ERROR = "error";
const char* CALLBACK_PROGRESS = "progress";
} //anonymous namespace

//----------------------------------------------------------------------------------------
//OperationCallbackData
//----------------------------------------------------------------------------------------

OperationCallbackData::OperationCallbackData(JSContextRef globalCtx,
        ArchiveCallbackType callback_type) :
    MultiCallbackUserData(globalCtx),
    m_callback_type(callback_type),
    m_op_id(0),
    m_is_error(false),
    m_is_canceled(false)
{
    LOGD("Entered");
}

OperationCallbackData::~OperationCallbackData()
{
    LOGD("Entered");
    if(m_op_id > 0){
        ArchiveManager::getInstance().eraseElementFromArchiveFileMap(m_op_id);
    }
}

void OperationCallbackData::setError(const std::string &err_name,
        const std::string &err_message)
{
    LOGD("Entered");
    //store only first error
    if (!m_is_error) {
        m_err_name = err_name;
        m_err_message = err_message;
        m_is_error = true;
    }
}

bool OperationCallbackData::isError() const
{
    LOGD("Entered");
    return m_is_error;
}

bool OperationCallbackData::isCanceled() const
{
    return m_is_canceled;
}

void OperationCallbackData::setOperationId(long op_id)
{
    LOGD("Entered");
    m_op_id = op_id;
}

long OperationCallbackData::getOperationId() const
{
    LOGD("Entered");
    return m_op_id;
}

void OperationCallbackData::setIsCanceled(bool canceled)
{
    m_is_canceled = canceled;
}

const std::string& OperationCallbackData::getErrorName() const
{
    LOGD("Entered");
    return m_err_name;
}

const std::string& OperationCallbackData::getErrorMessage() const
{
    LOGD("Entered");
    return m_err_message;
}

ArchiveCallbackType OperationCallbackData::getCallbackType() const
{
    LOGD("Entered");
    return m_callback_type;
}

ArchiveFilePtr OperationCallbackData::getArchiveFile() const
{
    return m_caller_instance;
}

void OperationCallbackData::setArchiveFile(ArchiveFilePtr caller)
{
    m_caller_instance = caller;
}

void OperationCallbackData::setSuccessCallback(JSValueRef on_success)
{
    LOGD("Entered");
    auto ctx = getContext();
    if(on_success && JSValueIsObject(ctx, on_success)) {
        JSObjectRef success = JSValueToObject(ctx, on_success, NULL);
        this->setCallback(CALLBACK_SUCCESS, success);
    }
}

void OperationCallbackData::setErrorCallback(JSValueRef on_error)
{
    LOGD("Entered");
    auto ctx = getContext();
    if(on_error && JSValueIsObject(ctx, on_error)) {
        JSObjectRef error = JSValueToObject(ctx, on_error, NULL);
        this->setCallback(CALLBACK_ERROR, error);
    }
}

void OperationCallbackData::callSuccessCallback()
{
    LOGD("Entered");
    this->invokeCallback(CALLBACK_SUCCESS, 0, NULL);
}

void OperationCallbackData::callSuccessCallback(JSValueRef success)
{
    LOGD("Entered");
    this->invokeCallback(CALLBACK_SUCCESS, success);
}

void OperationCallbackData::callErrorCallback(JSValueRef err)
{
    LOGD("Entered");
    this->invokeCallback(CALLBACK_ERROR, err);
}

//----------------------------------------------------------------------------------------
//OpenCallbackData
//----------------------------------------------------------------------------------------

OpenCallbackData::OpenCallbackData(JSContextRef globalCtx,
        ArchiveCallbackType callback_type):
    OperationCallbackData(globalCtx, callback_type)
{
    LOGD("Entered");
}

OpenCallbackData::~OpenCallbackData()
{
    LOGD("Entered");
}

void OpenCallbackData::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LOGE("Entered");

    Filesystem::FilePtr file = archive_file_ptr->getFile();
    if (!file) {
        LOGE("File is null");
        throw UnknownException("File is null");
    }
    Filesystem::NodePtr node = file->getNode();
    if(!node) {
        LOGE("Node is null");
        throw UnknownException("Node is null");
    }
    const FileMode fm = archive_file_ptr->m_file_mode;
    if (0 == node->getSize()) {
        if(FileMode::READ_WRITE == fm ||
                FileMode::WRITE == fm ||
                FileMode::ADD == fm) {
            LOGD("Empty file obtained for writing/appending");

            // Do not create empty archive with minizip library - it will not be loaded
            // by unzip.
            //
            // For explanation please see:
            //    ArchiveFile.h m_created_as_new_empty_archive description
            //
            archive_file_ptr->setCreatedAsNewEmptyArchive(true);
            archive_file_ptr->setEntryMap(ArchiveFileEntryPtrMapPtr(
                    new ArchiveFileEntryPtrMap()));
            archive_file_ptr->setIsOpen(true);
        }
        else {
            LOGE("The file is empty throwing: InvalidValuesException - Invalid ZIP archive");
            throw InvalidValuesException("Invalid ZIP archive");
        }
    }
    else {
        archive_file_ptr->setIsOpen(true);
        archive_file_ptr->updateListOfEntries();
    }

    guint id = g_idle_add(ArchiveFile::openTaskCompleteCB, this);
    if (!id) {
        LOGE("g_idle_add fails");
        throw UnknownException("g_idle_add fails");
    }
}

//----------------------------------------------------------------------------------------
//GetEntriesCallbackData
//----------------------------------------------------------------------------------------

GetEntriesCallbackData::GetEntriesCallbackData(JSContextRef globalCtx,
        ArchiveCallbackType callback_type):
    OperationCallbackData(globalCtx, callback_type)
{
    LOGD("Entered");
}
GetEntriesCallbackData::~GetEntriesCallbackData()
{
    LOGD("Entered");
}

ArchiveFileEntryPtrMapPtr GetEntriesCallbackData::getEntries() const
{
    return m_entries;
}

void GetEntriesCallbackData::setEntries(ArchiveFileEntryPtrMapPtr entries)
{
    m_entries = entries;
}

void GetEntriesCallbackData::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LOGD("Entered");

    setEntries(archive_file_ptr->getEntryMap());

    guint id = g_idle_add(ArchiveFile::getEntriesTaskCompleteCB, this);
    if (!id) {
        LOGE("g_idle_add fails");
        throw UnknownException("g_idle_add fails");
    }
}

//----------------------------------------------------------------------------------------
//GetEntryByNameCallbackData
//----------------------------------------------------------------------------------------

GetEntryByNameCallbackData::GetEntryByNameCallbackData(JSContextRef globalCtx,
        ArchiveCallbackType callback_type):
    OperationCallbackData(globalCtx, callback_type)
{
    LOGD("Entered");
}

GetEntryByNameCallbackData::~GetEntryByNameCallbackData()
{
    LOGD("Entered");
}

const std::string& GetEntryByNameCallbackData::getName() const
{
    LOGD("Entered");
    return m_name;
}

void GetEntryByNameCallbackData::setName(const std::string& name)
{
    LOGD("Entered");
    m_name = name;
}

ArchiveFileEntryPtr GetEntryByNameCallbackData::getFileEntry() const
{
    return m_file_entry;
}

void GetEntryByNameCallbackData::setFileEntry(ArchiveFileEntryPtr entry)
{
    m_file_entry = entry;
}

void GetEntryByNameCallbackData::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LOGD("Entered");

    ArchiveFileEntryPtrMapPtr entries = archive_file_ptr->getEntryMap();
    auto it = entries->find(getName());

    //Not found but if our name does not contain '/'
    //try looking for directory with such name
    //
    if (it == entries->end() && !isDirectoryPath(getName())) {
        const std::string try_directory = getName() + "/";
        LOGD("GetEntryByName Trying directory: [%s]", try_directory.c_str());
        it = entries->find(try_directory);
    }

    if (it == entries->end()) {
        LOGE("GetEntryByName Entry with name: [%s] not found", getName().c_str());
        LOGE("Throwing NotFoundException - Entry not found");
        throw Common::NotFoundException("Entry not found");
    }

    setFileEntry(it->second);

    guint id = g_idle_add(ArchiveFile::getEntryByNameTaskCompleteCB, this);
    if (!id) {
        LOGE("g_idle_add fails");
        throw UnknownException("g_idle_add fails");
    }
}

//----------------------------------------------------------------------------------------
//BaseProgressCallback
//----------------------------------------------------------------------------------------

BaseProgressCallback::BaseProgressCallback(JSContextRef globalCtx,
        ArchiveCallbackType callback_type):
    OperationCallbackData(globalCtx, callback_type),
    m_overwrite(false)
{
    LOGD("Entered");
}

BaseProgressCallback::~BaseProgressCallback()
{
    LOGD("Entered");
}

bool BaseProgressCallback::getOverwrite() const
{
    LOGD("Entered");
    return m_overwrite;
}

void BaseProgressCallback::setOverwrite(bool overwrite)
{
    LOGD("Entered");
    m_overwrite = overwrite;
}

struct ProgressHolder
{
    ProgressHolder() :
            callback(NULL)
    {
    };

    double overall_progress;
    ArchiveFileEntryPtr currently_processed_entry;
    BaseProgressCallback* callback;
};

void BaseProgressCallback::callSuccessCallbackOnMainThread()
{
    guint id = g_idle_add(BaseProgressCallback::callSuccessCallbackCB,
            static_cast<void*>(this));
    if (!id) {
        LOGE("g_idle_add fails - success callback will not be called");
    }
}

gboolean BaseProgressCallback::callSuccessCallbackCB(void* data)
{
    BaseProgressCallback* callback = static_cast<BaseProgressCallback*>(data);
    if (!callback) {
        LOGE("callback pointer is NULL");
        return false;
    }

    std::unique_ptr<BaseProgressCallback> cb_ptr(callback);

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context closed - unable to call success callback");
        return false;
    }

    try {
        callback->callSuccessCallback();
    }
    catch (const BasePlatformException &err) {
        LOGE("%s (%s)", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unknown error occurs");
    }

    callback->setArchiveFile(ArchiveFilePtr());
    ArchiveManager::getInstance().eraseElementFromArchiveFileMap(callback->m_op_id);

    return false;
}

void BaseProgressCallback::setProgressCallback(JSValueRef on_progress)
{
    LOGD("Entered");
    auto ctx = getContext();
    if(on_progress && JSValueIsObject(ctx, on_progress)) {
        JSObjectRef progress = JSValueToObject(ctx, on_progress, NULL);
        this->setCallback(CALLBACK_PROGRESS, progress);
    }
}

void BaseProgressCallback::callProgressCallback(long operationId,
        double value,
        const std::string& filename)
{
    LOGD("Entered");
    auto ctx = getContext();
    const int SIZE = 3;
    JSValueRef parameters[SIZE] = {
            JSUtil::toJSValueRef(ctx, operationId),
            JSUtil::toJSValueRef(ctx, value),
            JSUtil::toJSValueRef(ctx, filename)
        };

    this->invokeCallback(CALLBACK_PROGRESS, SIZE, parameters);
}

void BaseProgressCallback::callProgressCallbackOnMainThread(const double progress,
        ArchiveFileEntryPtr current_entry)
{
    ProgressHolder* ph = new(std::nothrow) ProgressHolder();

    if(ph) {
        ph->overall_progress = progress;
        ph->currently_processed_entry = current_entry;
        ph->callback = this;

        guint id = g_idle_add(BaseProgressCallback::callProgressCallbackCB,
                static_cast<void*>(ph));
        if (!id) {
            LOGE("g_idle_add fails");
            delete ph;
            ph = NULL;
        }
    } else {
        LOGE("Couldn't allocate ProgressHolder");
    }
}

gboolean BaseProgressCallback::callProgressCallbackCB(void* data)
{
    ProgressHolder* ph = static_cast<ProgressHolder*>(data);
    if (!ph) {
        LOGE("ph is null");
        return false;
    }

    std::unique_ptr<ProgressHolder> ph_ptr(ph);
    if (!ph->callback) {
        LOGE("ph->callback is null");
        return false;
    }

    JSContextRef context = ph->callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        return false;
    }
    try {
        //Error callback is being handled by ArchiveFile queue - see
        //ArchiveFile::taskManagerThread function
        //
        ph->callback->callProgressCallback(
                ph->callback->m_op_id,
                ph->overall_progress,
                ph->currently_processed_entry->getName());
    }
    catch (const BasePlatformException &err) {
        LOGE("%s (%s)", err.getName().c_str(), err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Unknown error occurs");
    }

    return false;
}

//----------------------------------------------------------------------------------------
//AddProgressCallback
//----------------------------------------------------------------------------------------

AddProgressCallback::AddProgressCallback(JSContextRef globalCtx,
        ArchiveCallbackType callback_type):
    BaseProgressCallback(globalCtx, callback_type)
{
    LOGD("Entered");
}

AddProgressCallback::~AddProgressCallback()
{
    LOGD("Entered");
}

ArchiveFileEntryPtr AddProgressCallback::getFileEntry() const
{
    LOGD("Entered");
    return m_file_entry;
}

void AddProgressCallback::setFileEntry(ArchiveFileEntryPtr file_entry)
{
    LOGD("Entered");
    m_file_entry = file_entry;
}

void AddProgressCallback::setBasePath(const std::string& path)
{
    LOGD("Entered");
    m_base_path = path;
    m_base_virt_path = Filesystem::External::toVirtualPath(m_base_path);
    std::string::size_type pos = m_base_virt_path.find(DeviceAPI::Filesystem::Path::getSeparator());
    if (pos != std::string::npos)
    {
	m_base_virt_path = m_base_virt_path.substr(pos + 1);
    } 
    else
    {
        m_base_virt_path = "";
    }
}

const std::string& AddProgressCallback::getBasePath()
{
    LOGD("Entered");
    return m_base_path;
}

const std::string& AddProgressCallback::getBaseVirtualPath()
{
    LOGD("Entered");
    return m_base_virt_path;
}

void AddProgressCallback::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LOGD("Entered");

    if(!m_file_entry) {
        LOGE("ArchiveFileEntry is not set in callback");
        throw UnknownException("Could not add file to archive");
    }

    if(!archive_file_ptr) {
        LOGE("archive_file_ptr is NULL");
        throw UnknownException("Could not extract archive file entry");
    }

    AddProgressCallback* callback = this;

    ZipPtr zip = archive_file_ptr->createZipObject();
    zip->addFile(callback);
    // Zip is no more needed but it locks file opening while
    // it is needed to read entries from file
    zip->close();

    //We have just finished adding file to archive so now
    //this archive file should be valid .zip archive and
    //we can remove CreatedAsNewEmptyArchive flag
    archive_file_ptr->setCreatedAsNewEmptyArchive(false);

    LOGD("Update decompressed size and entry list");
    // update informations about decompressed size and entry list
    // TODO FIXME need to resolve problem with access to file by
    // more than one thread
    try{
        archive_file_ptr->updateListOfEntries();
    } catch(...){
        LOGD("Unknown error during updating entries list inside archive");
    }
}

//----------------------------------------------------------------------------------------
//ExtractAllProgressCallback
//----------------------------------------------------------------------------------------

ExtractAllProgressCallback::ExtractAllProgressCallback(JSContextRef globalCtx,
        ArchiveCallbackType callback_type):
    BaseProgressCallback(globalCtx, callback_type),
    m_files_to_extract(0),
    m_files_extracted(0),
    m_current_file_size(0),
    m_current_file_extracted_bytes(0),
    m_progress_overall(0),
    m_overall_decompressed(0)
{
    LOGD("Entered");
}

ExtractAllProgressCallback::~ExtractAllProgressCallback()
{
    LOGD("Entered");
}

Filesystem::FilePtr ExtractAllProgressCallback::getDirectory() const
{
    return m_directory;
}

void ExtractAllProgressCallback::setDirectory(Filesystem::FilePtr directory)
{
    m_directory = directory;
}

void ExtractAllProgressCallback::startedExtractingFile(unsigned long current_file_size)
{
    m_current_file_size = current_file_size;
    m_current_file_extracted_bytes = 0;
}

void ExtractAllProgressCallback::extractedPartOfFile(unsigned long bytes_decompressed)
{
    m_current_file_extracted_bytes += bytes_decompressed;
    updateOverallProgress(bytes_decompressed);
}

void ExtractAllProgressCallback::finishedExtractingFile()
{
    m_current_file_size = 0;
    m_current_file_extracted_bytes = 0;
    ++m_files_extracted;
    updateOverallProgress(0);
}

void ExtractAllProgressCallback::updateOverallProgress(unsigned long bytes_decompressed)
{
    m_overall_decompressed += bytes_decompressed;
    m_progress_overall =
            static_cast<double>(m_overall_decompressed + m_files_extracted) /
            static_cast<double>(m_expected_decompressed_size + m_files_to_extract);

    LOGD("%s of %s - %f%% (%d/%d files)",
            bytesToReadableString(m_overall_decompressed).c_str(),
            bytesToReadableString(m_expected_decompressed_size).c_str(),
            m_progress_overall * 100.0,
            m_files_extracted, m_files_to_extract);
}

double ExtractAllProgressCallback::getCurrentFileProgress() const
{
    if(m_current_file_size > 0) {
        return static_cast<double>(m_current_file_extracted_bytes) /
                static_cast<double>(m_current_file_size);
    }
    else {
        return 1.0;
    }
}

double ExtractAllProgressCallback::getOverallProgress() const
{
    return m_progress_overall;
}

void ExtractAllProgressCallback::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LOGD("Entered");
    archive_file_ptr->extractAllTask(this);
}

void ExtractAllProgressCallback::setExpectedDecompressedSize(unsigned long exp_dec_size)
{
    m_expected_decompressed_size = exp_dec_size;
}

unsigned long ExtractAllProgressCallback::getExpectedDecompressedSize() const
{
    return m_expected_decompressed_size;
}

void ExtractAllProgressCallback::setNumberOfFilesToExtract(unsigned long files_count)
{
    m_files_to_extract = files_count;
}

unsigned long ExtractAllProgressCallback::getNumberOfFilesToExtract() const
{
    return m_files_to_extract;
}

//----------------------------------------------------------------------------------------
//OperationCanceledException
//----------------------------------------------------------------------------------------

const char* OPERATION_CANCELED_EXCEPTION = "OperationCanceledException";

OperationCanceledException::OperationCanceledException(const char* message):
        BasePlatformException(OPERATION_CANCELED_EXCEPTION, message)
{
    LOGD("Entered");
}

OperationCanceledException::OperationCanceledException(JSContextRef ctx,
        JSValueRef exception):
    BasePlatformException(ctx, exception)
{
    mName = OPERATION_CANCELED_EXCEPTION;
}

//----------------------------------------------------------------------------------------
//ExtractEntryProgressCallback
//----------------------------------------------------------------------------------------

ExtractEntryProgressCallback::ExtractEntryProgressCallback(JSContextRef globalCtx):
        ExtractAllProgressCallback(globalCtx),
        m_strip_name(false)
{
    LOGD("Entered");
    m_callback_type = EXTRACT_ENTRY_PROGRESS_CALLBACK;
}

ExtractEntryProgressCallback::~ExtractEntryProgressCallback()
{
    LOGD("Entered");
}

ArchiveFileEntryPtr ExtractEntryProgressCallback::getArchiveFileEntry()
{
    return m_archive_file_entry;
}

void ExtractEntryProgressCallback::setArchiveFileEntry(ArchiveFileEntryPtr afentry)
{
    m_archive_file_entry = afentry;
}

void ExtractEntryProgressCallback::setStripName(bool strip_name)
{
    m_strip_name = strip_name;
}

bool ExtractEntryProgressCallback::getStripName() const
{
    return m_strip_name;
}

void ExtractEntryProgressCallback::setStripBasePath(
        const std::string& strip_base_path)
{
    m_strip_base_path = strip_base_path;
}

const std::string& ExtractEntryProgressCallback::getStripBasePath() const
{
    return m_strip_base_path;
}

void ExtractEntryProgressCallback::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LOGD("Entered");

    if(!m_archive_file_entry) {
        LOGE("ArchiveFileEntry is not set in callback");
        throw UnknownException("Could not extract archive file entry");
    }

    if(!archive_file_ptr) {
        LOGE("archive_file_ptr is NULL");
        throw UnknownException("Could not extract archive file entry");
    }

    UnZipPtr unzip = archive_file_ptr->createUnZipObject();
    unzip->extractTo(this);
}

}
}
