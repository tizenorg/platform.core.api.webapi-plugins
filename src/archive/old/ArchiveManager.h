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

#ifndef _TIZEN_ARCHIVE_ARCHIVE_MANAGER_H_
#define _TIZEN_ARCHIVE_ARCHIVE_MANAGER_H_

#include <string>
#include "ArchiveFile.h"
#include "ArchiveCallbackData.h"


namespace DeviceAPI {
namespace Archive {

typedef std::map<long, ArchiveFilePtr> ArchiveFileMap;
typedef std::pair<long, ArchiveFilePtr> ArchiveFilePair;

class ArchiveManager {
public:
    static ArchiveManager& getInstance();
    ~ArchiveManager();

    void abort(long operation_id);
    long getNextOperationId(ArchiveFilePtr archive_file_ptr);
    void eraseElementFromArchiveFileMap(long operation_id);
    long open(OpenCallbackData* callback);

private:
    ArchiveManager();
    ArchiveManager(ArchiveManager const&);
    void operator=(ArchiveManager const&);

    ArchiveFileMap m_archive_file_map;

    long m_next_unique_id;

};

} // Archive
} // DeviceAPI

#endif /* _TIZEN_ARCHIVE_ARCHIVE_MANAGER_H_ */
