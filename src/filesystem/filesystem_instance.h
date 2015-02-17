// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_INSTANCE_H_
#define FILESYSTEM_FILESYSTEM_INSTANCE_H_

#include "common/extension.h"
#include "filesystem_utils.h"
#include "filesystem_manager.h"

namespace extension {
namespace filesystem {

class FilesystemInstance : public common::ParsedInstance,
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
  void FileReadSync(const picojson::value& args, picojson::object& out);
  void FileWrite(const picojson::value& args, picojson::object& out);
  void FileWriteSync(const picojson::value& args, picojson::object& out);
  void FilesystemGetWidgetPaths(const picojson::value& args,
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
  void onFilesystemStateChangeErrorCallback();
  void onFilesystemStateChangeSuccessCallback(const std::string& label,
                                              const std::string& state,
                                              const std::string& type);
  void PrepareError(const FilesystemError& error, picojson::object& out);
};

}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_INSTANCE_H_
