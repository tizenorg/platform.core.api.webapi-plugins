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
 
#ifndef FILESYSTEM_FILESYSTEM_INSTANCE_H_
#define FILESYSTEM_FILESYSTEM_INSTANCE_H_

#include "common/extension.h"
#include "filesystem_utils.h"
#include <thread>
#include <mutex>
#include "filesystem_manager.h"

namespace extension {
namespace filesystem {

class FilesystemInstance : public common::ParsedDataInstance,
                           FilesystemStateChangeListener {
 public:
  FilesystemInstance();
  virtual ~FilesystemInstance();

 private:
  void FileCreateSync(const picojson::value& args, picojson::object& out);
  void FileRename(const picojson::value& args, picojson::object& out);
  void FileStat(const picojson::value& args, picojson::object& out);
  void FileStatSync(const picojson::value& args, picojson::object& out);
  void FileRead(const picojson::value& args, picojson::object& out);
  void FileReadSync(const common::ParsedDataRequest& req, common::ParsedDataResponse& res);
  void FileWrite(const picojson::value& args, picojson::object& out);
  void FileWriteSync(const common::ParsedDataRequest& req, common::ParsedDataResponse& res);
  void FilesystemFetchVirtualRoots(const picojson::value& args,
                                   picojson::object& out);
  void FileSystemManagerFetchStorages(const picojson::value& args,
                                      picojson::object& out);
  void FileSystemManagerMakeDirectory(const picojson::value& args,
                                      picojson::object& out);
  void FileSystemManagerMakeDirectorySync(const picojson::value& args,
                                          picojson::object& out);
  void ReadDir(const picojson::value& args, picojson::object& out);
  void UnlinkFile(const picojson::value& args, picojson::object& out);
  void RemoveDirectory(const picojson::value& args, picojson::object& out);
  void StartListening(const picojson::value& args, picojson::object& out);
  void StopListening(const picojson::value& args, picojson::object& out);
  void CopyTo(const picojson::value& args, picojson::object& out);
  void onFilesystemStateChangeErrorCallback();
  void onFilesystemStateChangeSuccessCallback(const common::VirtualStorage& storage);
  void PrepareError(const FilesystemError& error, picojson::object& out);
};

}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_INSTANCE_H_
