// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_INSTANCE_H_
#define FILESYSTEM_FILESYSTEM_INSTANCE_H_

#include "common/extension.h"
#include "filesystem_utils.h"

namespace extension {
namespace filesystem {

class FilesystemInstance : public common::ParsedInstance {
 public:
  FilesystemInstance();
  virtual ~FilesystemInstance();

 private:
  void FileStat(const picojson::value& args, picojson::object& out);
  void FileStatSync(const picojson::value& args, picojson::object& out);
  void FilesystemGetWidgetPaths(const picojson::value& args,
                                picojson::object& out);
  void FileSystemManagerFetchStorages(const picojson::value& args,
                                      picojson::object& out);

  void PrepareError(const FilesystemError& error, picojson::object& out);
};

}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_INSTANCE_H_
