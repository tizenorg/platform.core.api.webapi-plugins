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
 
#include "archive_manager.h"
#include <mutex>
#include "common/logger.h"
#include "filesystem_file.h"

namespace extension {
namespace archive {

using namespace filesystem;

ArchiveManager::ArchiveManager():
        m_next_unique_id(0)
{
    LoggerD("Initialize ArchiveManager");
}

ArchiveManager::~ArchiveManager()
{
    LoggerD("Deinitialize ArchiveManager");
}

ArchiveManager& ArchiveManager::getInstance()
{
    LoggerD("Entered");
    static ArchiveManager instance;
    return instance;
}

void ArchiveManager::abort(long operation_id)
{
    LoggerD("Entered");

    ArchiveFileMap::iterator it = m_archive_file_map.find(operation_id);
    if (it != m_archive_file_map.end()) {
        ArchiveFilePtr archive_file_ptr = it->second;
        std::lock_guard<std::mutex> lock(archive_file_ptr->m_mutex);

        std::size_t size = archive_file_ptr->m_task_queue.size();
        for(std::size_t i = 0; i < size; ++i){
            if(operation_id == archive_file_ptr->m_task_queue[i].first){
                archive_file_ptr->m_task_queue[i].second->setIsCanceled(true);
                return;
            }
        }
    }
    LoggerD("The Operation Identifier not found");
}

void ArchiveManager::erasePrivData(long handle)
{
    LoggerD("Entered");

    ArchiveFileMap::iterator it = m_priv_map.find(handle);
    if (it != m_priv_map.end()) {
        m_priv_map.erase(it);
    }
}

long ArchiveManager::addPrivData(ArchiveFilePtr archive_file_ptr)
{
    LoggerD("Entered");

    long handle = ++m_next_unique_id;
    m_priv_map.insert(ArchiveFilePair(handle, archive_file_ptr));
    return handle;
}

PlatformResult ArchiveManager::getPrivData(long handle, ArchiveFilePtr* archive_file)
{
    ArchiveFileMap::iterator it = m_priv_map.find(handle);
    if (it != m_priv_map.end()) {
        *archive_file = it->second;
        return PlatformResult(ErrorCode::NO_ERROR);
    }
    LoggerE("Failed: Priv is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Priv is null");
}

PlatformResult ArchiveManager::open(OpenCallbackData* callback)
{
    LoggerD("Entered");

    //ArchiveFilePtr a_ptr = callback->getArchiveFile();
//    std::string filename = callback->getFile();
//
//    NodePtr node = Node::resolve(Path::create(filename));
//    FilePtr file_ptr = FilePtr(new File(node, std::vector<int>(), filename));
//    ArchiveFilePtr a_ptr = ArchiveFilePtr(new ArchiveFile(FileMode::READ));

    ArchiveFilePtr a_ptr = callback->getArchiveFile();
    return a_ptr->addOperation(callback);
}

void ArchiveManager::eraseElementFromArchiveFileMap(long operation_id)
{
    LoggerD("Entered");
    ArchiveFileMap::iterator it = m_archive_file_map.find(operation_id);
    if (it != m_archive_file_map.end()) {
        m_archive_file_map.erase(it);
    }
}

} // archive
} // extension
