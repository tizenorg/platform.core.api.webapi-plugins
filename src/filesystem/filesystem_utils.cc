// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem_utils.h"

#include <libgen.h>
#include "common/logger.h"

namespace FilesystemUtils {
std::string get_storage_dir_path(int id, storage_directory_e typeToCheck) {
  int result = STORAGE_ERROR_NONE;
  char* platformPath = NULL;
  result = storage_get_directory(id, typeToCheck, &platformPath);
  if (result != STORAGE_ERROR_NONE) {
    LoggerD("Cannot retrieve path for type %i", typeToCheck);
    return std::string();
  }
  std::string path = std::string(platformPath);
  free(platformPath);
  return path;
}

std::string get_dirname(const std::string& path) {
  // dirname will modify content: pass a copy
  std::string buf = path.c_str();
  return std::string(dirname(const_cast<char*>(buf.c_str())));
}

std::string get_basename(const std::string& path) {
  // basename will modify content: pass a copy
  std::string buf = path.c_str();
  return std::string(basename(const_cast<char*>(buf.c_str())));
}
}
