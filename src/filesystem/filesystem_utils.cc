// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem_utils.h"

#include "common/logger.h"

namespace std {

std::string to_string(storage_type_e type) {
  LoggerD("enter");
  switch (type) {
    case STORAGE_TYPE_INTERNAL:
      return "INTERNAL";
    case STORAGE_TYPE_EXTERNAL:
      return "EXTERNAL";
    default:
      return "";
  }
}

std::string to_string(storage_state_e state) {
  LoggerD("enter");
  switch (state) {
    case STORAGE_STATE_UNMOUNTABLE:
      return "UNMOUNTABLE";
    case STORAGE_STATE_REMOVED:
      return "REMOVED";
    case STORAGE_STATE_MOUNTED:
      return "MOUNTED";
    case STORAGE_STATE_MOUNTED_READ_ONLY:
      return "MOUNTED";
    default:
      return "";
  }
}
}

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
}
