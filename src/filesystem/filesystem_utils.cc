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
#include "filesystem_utils.h"

#include <glib.h>
#include <libgen.h>
#include "common/logger.h"

namespace FilesystemUtils {
std::string get_storage_dir_path(int id, storage_directory_e typeToCheck) {
  LoggerD("Enter");
  char* platformPath = NULL;
  int result = storage_get_directory(id, typeToCheck, &platformPath);
  if (result != STORAGE_ERROR_NONE) {
    LoggerD("Cannot retrieve path for type %i", typeToCheck);
    return std::string();
  }
  std::string path = std::string(platformPath);
  free(platformPath);
  return path;
}

std::string get_dirname(const std::string& path) {
  char* dir = g_path_get_dirname(path.c_str());
  if (dir) {
    std::string dir_result(dir);
    g_free(dir);
    return dir_result;
  } else {
    return std::string(".");
  }
}

std::string get_basename(const std::string& path) {
  // basename will modify content: pass a copy
  std::string buf = path.c_str();
  return std::string(basename(const_cast<char*>(buf.c_str())));
}
}
