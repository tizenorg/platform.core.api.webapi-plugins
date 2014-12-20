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

#include "archive_callback_data.h"

#include "common/logger.h"

//#include <filesystemExternalUtils.h>
#include "archive_file.h"
#include "archive_utils.h"
#include "un_zip.h"
#include "zip.h"
#include "archive_manager.h"
#include "defs.h"
#include "archive_instance.h"

namespace extension {
namespace archive {

using namespace common;

//----------------------------------------------------------------------------------------
//OperationCallbackData
//----------------------------------------------------------------------------------------

OperationCallbackData::OperationCallbackData(ArchiveCallbackType callback_type) :
    m_callback_type(callback_type),
    m_op_id(-1),
    m_cid(-1),
    m_handle(-1),
    m_is_error(false),
    m_is_canceled(false)
{
    LoggerD("Entered");
}

OperationCallbackData::~OperationCallbackData()
{
    LoggerD("Entered");
    if(m_op_id > -1){
        ArchiveManager::getInstance().eraseElementFromArchiveFileMap(m_op_id);
    }
}

void OperationCallbackData::setError(const std::string &err_name,
        const std::string &err_message)
{
    LoggerD("Entered");
    //store only first error
    if (!m_is_error) {
        m_err_name = err_name;
        m_err_message = err_message;
        m_is_error = true;
    }
}

bool OperationCallbackData::isError() const
{
    LoggerD("Entered");
    return m_is_error;
}

bool OperationCallbackData::isCanceled() const
{
    return m_is_canceled;
}

void OperationCallbackData::setOperationId(long op_id)
{
    LoggerD("Entered");
    m_op_id = op_id;
}

long OperationCallbackData::getOperationId() const
{
    LoggerD("Entered");
    return m_op_id;
}

void OperationCallbackData::setCallbackId(double cid)
{
    m_cid = cid;
}

double OperationCallbackData::getCallbackId() const
{
    return m_cid;
}

void OperationCallbackData::setHandle(long handle)
{
    m_handle = handle;
}

long OperationCallbackData::getHandle() const
{
    return m_handle;
}

void OperationCallbackData::setIsCanceled(bool canceled)
{
    m_is_canceled = canceled;
}

const std::string& OperationCallbackData::getErrorName() const
{
    LoggerD("Entered");
    return m_err_name;
}

const std::string& OperationCallbackData::getErrorMessage() const
{
    LoggerD("Entered");
    return m_err_message;
}

ArchiveCallbackType OperationCallbackData::getCallbackType() const
{
    LoggerD("Entered");
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

//----------------------------------------------------------------------------------------
//OpenCallbackData
//----------------------------------------------------------------------------------------

OpenCallbackData::OpenCallbackData(ArchiveCallbackType callback_type):
    OperationCallbackData(callback_type)
{
    LoggerD("Entered");
}

OpenCallbackData::~OpenCallbackData()
{
    LoggerD("Entered");
}

void OpenCallbackData::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LoggerE("Entered");

    filesystem::FilePtr file = archive_file_ptr->getFile();
    if (!file) {
        LoggerE("File is null");
        throw UnknownException("File is null");
    }
    filesystem::NodePtr node = file->getNode();
    if(!node) {
        LoggerE("Node is null");
        throw UnknownException("Node is null");
    }
    const FileMode fm = archive_file_ptr->m_file_mode;
    if (0 == node->getSize()) {
        if(FileMode::READ_WRITE == fm ||
                FileMode::WRITE == fm ||
                FileMode::ADD == fm) {
            LoggerD("Empty file obtained for writing/appending");

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
            LoggerE("The file is empty throwing: InvalidValuesException - Invalid ZIP archive");
            throw InvalidValuesException("Invalid ZIP archive");
        }
    }
    else {
        archive_file_ptr->setIsOpen(true);
        archive_file_ptr->updateListOfEntries();
    }

    guint id = g_idle_add(ArchiveFile::openTaskCompleteCB, this);
    if (!id) {
        LoggerE("g_idle_add fails");
        throw UnknownException("g_idle_add fails");
    }
}

//----------------------------------------------------------------------------------------
//GetEntriesCallbackData
//----------------------------------------------------------------------------------------

GetEntriesCallbackData::GetEntriesCallbackData(ArchiveCallbackType callback_type):
    OperationCallbackData(callback_type)
{
    LoggerD("Entered");
}

GetEntriesCallbackData::~GetEntriesCallbackData()
{
    LoggerD("Entered");
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
    LoggerD("Entered");

    setEntries(archive_file_ptr->getEntryMap());

    guint id = g_idle_add(ArchiveFile::getEntriesTaskCompleteCB, this);
    if (!id) {
        LoggerE("g_idle_add fails");
        throw UnknownException("g_idle_add fails");
    }
}

//----------------------------------------------------------------------------------------
//GetEntryByNameCallbackData
//----------------------------------------------------------------------------------------

GetEntryByNameCallbackData::GetEntryByNameCallbackData(ArchiveCallbackType callback_type):
    OperationCallbackData(callback_type)
{
    LoggerD("Entered");
}

GetEntryByNameCallbackData::~GetEntryByNameCallbackData()
{
    LoggerD("Entered");
}

const std::string& GetEntryByNameCallbackData::getName() const
{
    LoggerD("Entered");
    return m_name;
}

void GetEntryByNameCallbackData::setName(const std::string& name)
{
    LoggerD("Entered");
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
    LoggerD("Entered");

    ArchiveFileEntryPtrMapPtr entries = archive_file_ptr->getEntryMap();
    auto it = entries->find(getName());

    //Not found but if our name does not contain '/'
    //try looking for directory with such name
    //
    if (it == entries->end() && !isDirectoryPath(getName())) {
        const std::string try_directory = getName() + "/";
        LoggerD("GetEntryByName Trying directory: [%s]", try_directory.c_str());
        it = entries->find(try_directory);
    }

    if (it == entries->end()) {
        LoggerE("GetEntryByName Entry with name: [%s] not found", getName().c_str());
        LoggerE("Throwing NotFoundException - Entry not found");
        throw NotFoundException("Entry not found");
    }

    setFileEntry(it->second);

    guint id = g_idle_add(ArchiveFile::getEntryByNameTaskCompleteCB, this);
    if (!id) {
        LoggerE("g_idle_add fails");
        throw UnknownException("g_idle_add fails");
    }
}

//----------------------------------------------------------------------------------------
//BaseProgressCallback
//----------------------------------------------------------------------------------------

BaseProgressCallback::BaseProgressCallback(ArchiveCallbackType callback_type):
    OperationCallbackData(callback_type),
    m_overwrite(false)
{
    LoggerD("Entered");
}

BaseProgressCallback::~BaseProgressCallback()
{
    LoggerD("Entered");
}

bool BaseProgressCallback::getOverwrite() const
{
    LoggerD("Entered");
    return m_overwrite;
}

void BaseProgressCallback::setOverwrite(bool overwrite)
{
    LoggerD("Entered");
    m_overwrite = overwrite;
}

struct ProgressHolder
{
    ProgressHolder() :
        overall_progress(0.0),
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
        LoggerE("g_idle_add fails - success callback will not be called");
    }
}

gboolean BaseProgressCallback::callSuccessCallbackCB(void* data)
{
    BaseProgressCallback* callback = static_cast<BaseProgressCallback*>(data);
    if (!callback) {
        LoggerE("callback pointer is NULL");
        return false;
    }

    std::unique_ptr<BaseProgressCallback> cb_ptr(callback);

    try {
        picojson::value val = picojson::value(picojson::object());
        picojson::object& obj = val.get<picojson::object>();
        obj[JSON_CALLBACK_ID] = picojson::value(callback->getCallbackId());

        if (!callback->isError()) {
            obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_SUCCCESS);

            LoggerD("%s", val.serialize().c_str());

            ArchiveInstance::getInstance().PostMessage(val.serialize().c_str());
        } else {
            LoggerW("Not calling error callback in such case");
        }
    }
    catch (const PlatformException& ex) {
        LoggerE("%s (%s)", ex.name().c_str(), ex.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occurred");
    }

    callback->setArchiveFile(ArchiveFilePtr());
    ArchiveManager::getInstance().eraseElementFromArchiveFileMap(callback->m_op_id);

    return false;
}

void BaseProgressCallback::callProgressCallback(long operationId,
        double value,
        const std::string& filename,
        double callbackId)
{
    LoggerD("Entered");

    try {
        picojson::value val = picojson::value(picojson::object());
        picojson::object& obj = val.get<picojson::object>();
        obj[JSON_CALLBACK_ID] = picojson::value(callbackId);
        obj[JSON_DATA] = picojson::value(picojson::object());
        picojson::object& args = obj[JSON_DATA].get<picojson::object>();

        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_PROGRESS);
        obj[JSON_CALLBACK_KEEP] = picojson::value(true);

        args[PARAM_OPERATION_ID] = picojson::value(static_cast<double>(operationId));
        args[PARAM_VALUE] = picojson::value(value);
        args[PARAM_FILENAME] = picojson::value(filename);

        LoggerD("%s", val.serialize().c_str());

        ArchiveInstance::getInstance().PostMessage(val.serialize().c_str());
    }
    catch (const PlatformException& ex) {
        LoggerE("%s (%s)", ex.name().c_str(), ex.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occurred");
    }
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
            LoggerE("g_idle_add fails");
            delete ph;
            ph = NULL;
        }
    } else {
        LoggerE("Couldn't allocate ProgressHolder");
    }
}

gboolean BaseProgressCallback::callProgressCallbackCB(void* data)
{
    ProgressHolder* ph = static_cast<ProgressHolder*>(data);
    if (!ph) {
        LoggerE("ph is null");
        return false;
    }

    std::unique_ptr<ProgressHolder> ph_ptr(ph);
    if (!ph->callback) {
        LoggerE("ph->callback is null");
        return false;
    }

    LoggerW("STUB Not checking if context is still alive");

    try {
        //Error callback is being handled by ArchiveFile queue - see
        //ArchiveFile::taskManagerThread function
        //
        ph->callback->callProgressCallback(
                ph->callback->m_op_id,
                ph->overall_progress,
                ph->currently_processed_entry->getName(),
                ph->callback->m_cid);
    }
    catch (const PlatformException &err) {
        LoggerE("%s (%s)", err.name().c_str(), err.message().c_str());
    }
    catch (...) {
        LoggerE("Unknown error occurs");
    }

    return false;
}

//----------------------------------------------------------------------------------------
//AddProgressCallback
//----------------------------------------------------------------------------------------

AddProgressCallback::AddProgressCallback(ArchiveCallbackType callback_type):
    BaseProgressCallback(callback_type)
{
    LoggerD("Entered");
}

AddProgressCallback::~AddProgressCallback()
{
    LoggerD("Entered");
}

ArchiveFileEntryPtr AddProgressCallback::getFileEntry() const
{
    LoggerD("Entered");
    return m_file_entry;
}

void AddProgressCallback::setFileEntry(ArchiveFileEntryPtr file_entry)
{
    LoggerD("Entered");
    m_file_entry = file_entry;
}

void AddProgressCallback::setBasePath(const std::string& path)
{
    LoggerD("Entered");
    m_base_path = path;
    m_base_virt_path = filesystem::External::toVirtualPath(m_base_path);
    std::string::size_type pos = m_base_virt_path.find(filesystem::Path::getSeparator());
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
    LoggerD("Entered");
    return m_base_path;
}

const std::string& AddProgressCallback::getBaseVirtualPath()
{
    LoggerD("Entered");
    return m_base_virt_path;
}

void AddProgressCallback::executeOperation(ArchiveFilePtr archive_file_ptr)
{
    LoggerD("Entered");

    if(!m_file_entry) {
        LoggerE("ArchiveFileEntry is not set in callback");
        throw UnknownException("Could not add file to archive");
    }

    if(!archive_file_ptr) {
        LoggerE("archive_file_ptr is NULL");
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

    LoggerD("Update decompressed size and entry list");
    // update informations about decompressed size and entry list
    // TODO FIXME need to resolve problem with access to file by
    // more than one thread
    try{
        archive_file_ptr->updateListOfEntries();
    } catch(...){
        LoggerD("Unknown error during updating entries list inside archive");
    }
}

//----------------------------------------------------------------------------------------
//ExtractAllProgressCallback
//----------------------------------------------------------------------------------------

ExtractAllProgressCallback::ExtractAllProgressCallback(ArchiveCallbackType callback_type):
    BaseProgressCallback(callback_type),
    m_files_to_extract(0),
    m_expected_decompressed_size(0),
    m_files_extracted(0),
    m_current_file_size(0),
    m_current_file_extracted_bytes(0),
    m_progress_overall(0),
    m_overall_decompressed(0)
{
    LoggerD("Entered");
}

ExtractAllProgressCallback::~ExtractAllProgressCallback()
{
    LoggerD("Entered");
}

filesystem::FilePtr ExtractAllProgressCallback::getDirectory() const
{
    return m_directory;
}

void ExtractAllProgressCallback::setDirectory(filesystem::FilePtr directory)
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

    LoggerD("%s of %s - %f%% (%d/%d files)",
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
    LoggerD("Entered");
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

OperationCanceledException::OperationCanceledException(const char* message)
{
    LoggerD("Entered");
}

//----------------------------------------------------------------------------------------
//ExtractEntryProgressCallback
//----------------------------------------------------------------------------------------

ExtractEntryProgressCallback::ExtractEntryProgressCallback():
        ExtractAllProgressCallback(),
        m_strip_name(false)
{
    LoggerD("Entered");
    m_callback_type = EXTRACT_ENTRY_PROGRESS_CALLBACK;
}

ExtractEntryProgressCallback::~ExtractEntryProgressCallback()
{
    LoggerD("Entered");
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
    LoggerD("Entered");

    if(!m_archive_file_entry) {
        LoggerE("ArchiveFileEntry is not set in callback");
        throw UnknownException("Could not extract archive file entry");
    }

    if(!archive_file_ptr) {
        LoggerE("archive_file_ptr is NULL");
        throw UnknownException("Could not extract archive file entry");
    }

    UnZipPtr unzip = archive_file_ptr->createUnZipObject();
    unzip->extractTo(this);
}

} //namespace archive
} //namespace extension
