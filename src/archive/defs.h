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

#ifndef _ARCHIVE_PLUGIN_DEFS_H_
#define _ARCHIVE_PLUGIN_DEFS_H_

#define TIZEN_ARCHIVE_ARCHIVE_CLASS                             "archive"

#define ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_ABORT              "abort"
#define ARCHIVE_FUNCTION_API_ARCHIVE_MANAGER_OPEN               "open"

#define ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ADD                   "add"
#define ARCHIVE_FUNCTION_API_ARCHIVE_FILE_EXTRACT_ALL           "extractAll"
#define ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRIES           "getEntries"
#define ARCHIVE_FUNCTION_API_ARCHIVE_FILE_GET_ENTRY_BY_NAME     "getEntryByName"
#define ARCHIVE_FUNCTION_API_ARCHIVE_FILE_CLOSE                 "close"

#define ARCHIVE_FUNCTION_API_ARCHIVE_FILE_ENTRY_EXTRACT         "extract"

#define JSON_CMD                                                "cmd"
#define JSON_ACTION                                             "action"
#define JSON_CALLBACK_ID                                        "cid"
#define JSON_CALLBACK_SUCCCESS                                  "success"
#define JSON_CALLBACK_ERROR                                     "error"
#define JSON_CALLBACK_PROGRESS                                  "progress"
#define JSON_CALLBACK_KEEP                                      "keep"
#define JSON_DATA                                               "args"

#define PARAM_FILE                                              "file"
#define PARAM_MODE                                              "mode"
#define PARAM_OPTIONS                                           "options"
#define PARAM_SOURCE_FILE                                       "sourceFile"
#define PARAM_DESTINATION_DIR                                   "destinationDirectory"
#define PARAM_OVERWRITE                                         "overwrite"
#define PARAM_NAME                                              "name"
#define PARAM_STRIP_NAME                                        "stripName"
#define PARAM_OPERATION_ID                                      "opId"
#define PARAM_VALUE                                             "value"
#define PARAM_FILENAME                                          "filename"

#define ARCHIVE_FILE_ATTR_MODE                                  "mode"
#define ARCHIVE_FILE_ATTR_DECOMPRESSED_SIZE                     "decompressedSize"
#define ARCHIVE_FILE_HANDLE                                     "handle"

#define ARCHIVE_FILE_ENTRY_ATTR_NAME                            "name"
#define ARCHIVE_FILE_ENTRY_ATTR_SIZE                            "size"
#define ARCHIVE_FILE_ENTRY_ATTR_COMPRESSED_SIZE                 "compressedSize"
#define ARCHIVE_FILE_ENTRY_ATTR_MODIFIED                        "modified"

#define ERROR_CALLBACK_CODE                                     "code"
#define ERROR_CALLBACK_MESSAGE                                  "message"

#endif // _ARCHIVE_PLUGIN_DEFS_H_
