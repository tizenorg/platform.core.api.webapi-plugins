// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_UTILS_H
#define FILESYSTEM_FILESYSTEM_UTILS_H

#include <string>
#include <storage-expand.h>
#include "common/picojson.h"

namespace extension {
namespace filesystem {

enum class FilesystemError {
  None,
  NotFound,
  FileExists,
  DirectoryExists,
  PermissionDenied,
  IOError,
  Other
};
}  // namespace filesystem
}  // namespace extension

namespace std {

std::string to_string(storage_type_e);
std::string to_string(storage_state_e);
}

namespace FilesystemUtils {

/**
 * @brief get_storage_dir_path attempts to get path from storage.
 * If path cannot be retrieved then an empty string is returned.
 *
 */
std::string get_storage_dir_path(int id, storage_directory_e typeToCheck);

std::string get_dirname(const std::string& path);
std::string get_basename(const std::string& path);
}

#endif  // FILESYSTEM_FILESYSTEM_UTILS_H
