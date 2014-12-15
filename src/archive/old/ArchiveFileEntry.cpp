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
#include "ArchiveFileEntry.h"

#include <GlobalContextManager.h>
#include <JSWebAPIErrorFactory.h>
#include <Logger.h>
#include <PlatformException.h>

#include "ArchiveCallbackData.h"
#include "ArchiveFile.h"
#include "ArchiveUtils.h"

namespace DeviceAPI {

using namespace DeviceAPI::Common;

namespace Archive {

static const unsigned int s_default_compression_level = 5;

ArchiveFileEntry::ArchiveFileEntry(Filesystem::FilePtr file) :
        std::enable_shared_from_this<ArchiveFileEntry>(),
        m_file(file),
        m_archive(NULL),
        m_striped(false),
        m_size(0),
        m_compressed_size(0),
        m_modified(0),
        m_compression_level(s_default_compression_level)
{
    LOGD("Entered");
}

ArchiveFileEntry::~ArchiveFileEntry()
{
    LOGD("Entered");
}

unsigned long ArchiveFileEntry::getCompressedSize() const
{
    return m_compressed_size;
}

void ArchiveFileEntry::setCompressedSize(unsigned long compressed_size)
{
    m_compressed_size = compressed_size;
}

Filesystem::FilePtr ArchiveFileEntry::getFile() const
{
    return m_file;
}

void ArchiveFileEntry::setFile(Filesystem::FilePtr file)
{
    m_file = file;
}

unsigned long ArchiveFileEntry::getSize() const
{
    return m_size;
}

void ArchiveFileEntry::setSize(unsigned long size)
{
    m_size = size;
}

const std::string& ArchiveFileEntry::getName() const
{
    return m_name;
}

void ArchiveFileEntry::setName(const std::string& name)
{
    m_name = name;
}

void ArchiveFileEntry::setModified(time_t time)
{
    m_modified = time;
}

time_t ArchiveFileEntry::getModified() const
{
    return m_modified;
}

const std::string& ArchiveFileEntry::getDestination() const
{
    return m_destination;
}

void ArchiveFileEntry::setDestination(const std::string& destination)
{
    m_destination = destination;
}

bool ArchiveFileEntry::getStriped() const
{
    return m_striped;
}

void ArchiveFileEntry::setStriped(bool striped)
{
    m_striped = striped;
}

void ArchiveFileEntry::setCompressionLevel(unsigned int level)
{
    m_compression_level = level;
}
unsigned int ArchiveFileEntry::getCompressionLevel() const
{
    return m_compression_level;
}

void ArchiveFileEntry::setArchiveFileNonProtectPtr(ArchiveFile* ptr)
{
    m_archive = ptr;
}

ArchiveFile* ArchiveFileEntry::getArchiveFileNonProtectPtr()
{
    return m_archive;
}

long ArchiveFileEntry::extractTo(ExtractEntryProgressCallback* callback)
{
    if(!m_archive) {
        LOGE("m_archive is NULL");
        throw UnknownException("Could not extract archive file entry");
    }

    //Callback should be associated with this instance of ArchiveFileEntry
    callback->setArchiveFileEntry(shared_from_this());

    //
    // If strip name was set in JS layer we need to generate srip base path
    //
    if(callback->getStripName()) {

        //Get base path - left side of last slash
        std::string base_path_name = getBasePathFromPath(m_name);
        if(!isDirectoryPath(base_path_name) && !base_path_name.empty()) {
            base_path_name += "/";
        }

        LOGD("strip name is: true; archive file entry name is: [%s]; "
                "stripBasePath will be: [%s]",
                m_name.c_str(), base_path_name.c_str());

        callback->setStripBasePath(base_path_name);
    }

    return m_archive->extractEntryTo(callback);
}

} // Archive
} // DeviceAPI
