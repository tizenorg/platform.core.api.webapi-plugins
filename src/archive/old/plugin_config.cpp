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
#include <map>
#include <Commons/FunctionDefinition.h>

#include "plugin_config.h"

using namespace WrtDeviceApis::Commons;

namespace DeviceAPI {
namespace Archive {

namespace{
const char* ARCHIVE_FEATURE_API_WRITE = "http://tizen.org/privilege/filesystem.write";
const char* ARCHIVE_FEATURE_API_READ = "http://tizen.org/privilege/filesystem.read";

const char* ARCHIVE_DEVICE_CAP_WRITE = "filesystem.write";
const char* ARCHIVE_DEVICE_CAP_READ = "filesystem.read";
}

static FunctionMapping createArchiveFunctions();

static FunctionMapping ArchiveFunctions = createArchiveFunctions();

#pragma GCC visibility push(default)
DEFINE_FUNCTION_GETTER(Archive, ArchiveFunctions);
#pragma GCC visibility pop

static FunctionMapping createArchiveFunctions()
{
    /**
     * Device capabilities
     */
    ACE_CREATE_DEVICE_CAP(DEVICE_CAP_ARCHIVE_WRITE, ARCHIVE_DEVICE_CAP_WRITE);
    ACE_CREATE_DEVICE_CAP(DEVICE_CAP_ARCHIVE_READ, ARCHIVE_DEVICE_CAP_READ);

    ACE_CREATE_DEVICE_CAPS_LIST(DEVICE_LIST_ARCHIVE_WRITE);
    ACE_ADD_DEVICE_CAP(DEVICE_LIST_ARCHIVE_WRITE, DEVICE_CAP_ARCHIVE_WRITE);

    ACE_CREATE_DEVICE_CAPS_LIST(DEVICE_LIST_ARCHIVE_READ);
    ACE_ADD_DEVICE_CAP(DEVICE_LIST_ARCHIVE_READ, DEVICE_CAP_ARCHIVE_READ);

    /**
     * Api Features
     */
    ACE_CREATE_FEATURE(FEATURE_ARCHIVE_READ, ARCHIVE_FEATURE_API_READ);
    ACE_CREATE_FEATURE(FEATURE_ARCHIVE_WRITE, ARCHIVE_FEATURE_API_WRITE);

    ACE_CREATE_FEATURE_LIST(ARCHIVE_FEATURES_READ);
    ACE_ADD_API_FEATURE(ARCHIVE_FEATURES_READ, FEATURE_ARCHIVE_READ);

    ACE_CREATE_FEATURE_LIST(ARCHIVE_FEATURES_WRITE);
    ACE_ADD_API_FEATURE(ARCHIVE_FEATURES_WRITE, FEATURE_ARCHIVE_WRITE);

    /**
     * Functions
     */
    FunctionMapping archiveMapping;

    // open
    AceFunction archiveFileOpenFunc = ACE_CREATE_FUNCTION(
            FUNCTION_ARCHIVE_FILE_OPEN,
            ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_OPEN,
            ARCHIVE_FEATURES_WRITE,
            DEVICE_LIST_ARCHIVE_WRITE);

    archiveMapping.insert(std::make_pair(
                                ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_OPEN,
                                archiveFileOpenFunc));

    // add
    AceFunction archiveFileAddFunc = ACE_CREATE_FUNCTION(
            FUNCTION_ARCHIVE_FILE_ADD,
            ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD,
            ARCHIVE_FEATURES_WRITE,
            DEVICE_LIST_ARCHIVE_WRITE);

    archiveMapping.insert(std::make_pair(
                                ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD,
                                archiveFileAddFunc));

    // extractAll
    AceFunction archiveFileExtractAllFunc = ACE_CREATE_FUNCTION(
            FUNCTION_ARCHIVE_FILE_EXTRACT_ALL,
            ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL,
            ARCHIVE_FEATURES_WRITE,
            DEVICE_LIST_ARCHIVE_WRITE);

    archiveMapping.insert(std::make_pair(
                                ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL,
                                archiveFileExtractAllFunc));

    // getEntries
    AceFunction archiveFileGetEntriesFunc = ACE_CREATE_FUNCTION(
            FUNCTION_ARCHIVE_FILE_GET_ENTRIES,
            ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES,
            ARCHIVE_FEATURES_READ,
            DEVICE_LIST_ARCHIVE_READ);

    archiveMapping.insert(std::make_pair(
                                ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES,
                                archiveFileGetEntriesFunc));

    // getEntryByName
    AceFunction archiveFileGetEntryByNameFunc = ACE_CREATE_FUNCTION(
            FUNCTION_ARCHIVE_FILE_GET_ENTRY_BY_NAME,
            ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME,
            ARCHIVE_FEATURES_READ,
            DEVICE_LIST_ARCHIVE_READ);

    archiveMapping.insert(std::make_pair(
                                ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME,
                                archiveFileGetEntryByNameFunc));

    // extract
    AceFunction archiveFileEntryExtractFunc = ACE_CREATE_FUNCTION(
            FUNCTION_ARCHIVE_FILE_ENTRY_EXTRACT,
            ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ENTRY_EXTRACT,
            ARCHIVE_FEATURES_WRITE,
            DEVICE_LIST_ARCHIVE_WRITE);

    archiveMapping.insert(std::make_pair(
                                ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ENTRY_EXTRACT,
                                archiveFileEntryExtractFunc));

    return archiveMapping;
}

}
}
