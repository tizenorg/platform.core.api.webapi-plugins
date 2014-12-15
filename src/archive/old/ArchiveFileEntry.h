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

#ifndef _TIZEN_ARCHIVE_ARCHIVE_FILE_ENTRY_H_
#define _TIZEN_ARCHIVE_ARCHIVE_FILE_ENTRY_H_

#include <memory>
#include <File.h> // from filesystem



namespace DeviceAPI {
namespace Archive {

class ArchiveFile;

class ArchiveFileEntry;
typedef std::shared_ptr<ArchiveFileEntry> ArchiveFileEntryPtr;

class ExtractEntryProgressCallback;

typedef std::map<std::string, ArchiveFileEntryPtr> ArchiveFileEntryPtrMap;
typedef std::shared_ptr<ArchiveFileEntryPtrMap> ArchiveFileEntryPtrMapPtr;

class ArchiveFileEntry :  public std::enable_shared_from_this<ArchiveFileEntry> {
public:
    ArchiveFileEntry(Filesystem::FilePtr file = Filesystem::FilePtr());
    ~ArchiveFileEntry();

    unsigned long getCompressedSize() const;
    void setCompressedSize(unsigned long compressedSize);
    Filesystem::FilePtr getFile() const;
    void setFile(Filesystem::FilePtr file);
    unsigned long getSize() const;
    void setSize(unsigned long size);
    const std::string& getName() const;
    void setName(const std::string& name);
    void setModified(time_t time);
    time_t getModified() const;
    const std::string& getDestination() const;
    void setDestination(const std::string& destination);
    bool getStriped() const;
    void setStriped(bool striped);
    void setCompressionLevel(unsigned int level);
    unsigned int getCompressionLevel() const;

    void setArchiveFileNonProtectPtr(ArchiveFile* ptr);
    ArchiveFile* getArchiveFileNonProtectPtr();

    long extractTo(ExtractEntryProgressCallback* callback);

private:
    Filesystem::FilePtr m_file;
    ArchiveFile* m_archive;
    std::string m_name;
    std::string m_destination;
    bool m_striped;
    unsigned long m_size;
    unsigned long m_compressed_size;
    time_t m_modified;
    unsigned int m_compression_level;
};

} // Archive
} // DeviceAPI

#endif /* _TIZEN_ARCHIVE_FILE_ENTRY_H_ */
