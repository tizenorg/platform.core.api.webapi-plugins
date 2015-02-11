// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_INSTANCE_H_
#define FILESYSTEM_FILESYSTEM_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace filesystem {

class FilesystemInstance : public common::ParsedInstance {
 public:
  FilesystemInstance();
  virtual ~FilesystemInstance();

 private:
  void FileMoveTo(const picojson::value& args, picojson::object& out);
  void FileSystemManagerListStorages(const picojson::value& args, picojson::object& out);
  void FileSystemManagerResolve(const picojson::value& args, picojson::object& out);
  void FileSystemManagerAddStorageStateChangeListener(const picojson::value& args, picojson::object& out);
  void FileSystemManagerRemoveStorageStateChangeListener(const picojson::value& args, picojson::object& out);
  void FileToURI(const picojson::value& args, picojson::object& out);
  void FileResolve(const picojson::value& args, picojson::object& out);
  void FileListFiles(const picojson::value& args, picojson::object& out);
  void FileDeleteDirectory(const picojson::value& args, picojson::object& out);
  void FileOpenStream(const picojson::value& args, picojson::object& out);
  void FileCreateDirectory(const picojson::value& args, picojson::object& out);
  void FileCreateFile(const picojson::value& args, picojson::object& out);
  void FileDeleteFile(const picojson::value& args, picojson::object& out);
  void FileReadAsText(const picojson::value& args, picojson::object& out);
  void FileCopyTo(const picojson::value& args, picojson::object& out);
  void FileSystemManagerGetStorage(const picojson::value& args, picojson::object& out);
};

} // namespace filesystem
} // namespace extension

#endif // FILESYSTEM_FILESYSTEM_INSTANCE_H_
