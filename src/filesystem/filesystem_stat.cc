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

#include "filesystem_stat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <common/logger.h>
#include <common/tools.h>
#include <common/scope_exit.h>

namespace extension {
namespace filesystem {

using common::tools::GetErrorString;

FilesystemStat::FilesystemStat()
    : error(FilesystemError::None),
      valid(false),
      isFile(false),
      isDirectory(false),
      readOnly(false),
      ctime(0),
      mtime(0),
      size(0),
      nlink(0) {
}

picojson::value FilesystemStat::toJSON() const {
  LoggerD("Enter");
  picojson::value retval = picojson::value(picojson::object());
  picojson::object& obj = retval.get<picojson::object>();

  obj["path"] = picojson::value(path);
  obj["isFile"] = picojson::value(isFile);
  obj["isDirectory"] = picojson::value(isDirectory);
  obj["readOnly"] = picojson::value(readOnly);
  obj["ctime"] = picojson::value(static_cast<double>(ctime));
  obj["mtime"] = picojson::value(static_cast<double>(mtime));
  obj["size"] = picojson::value(static_cast<double>(size));
  obj["nlink"] = picojson::value(static_cast<double>(nlink));

  return retval;
}

FilesystemStat FilesystemStat::getStat(const std::string& path) {
  LoggerD("Enter");
  struct stat aStatObj;
  FilesystemStat _result;

  LoggerD("enter");

  if (0 != stat(path.c_str(), &aStatObj)) {
    LoggerE("Failed to stat: (%d) %s", errno, GetErrorString(errno).c_str());
    if (ENOENT == errno) {
      _result.error = FilesystemError::NotFound;
    } else {
      _result.error = FilesystemError::InvalidValue;
    }
    return _result;
  }

  _result.path = path;
  _result.readOnly = true;
  if (getuid() == aStatObj.st_uid && (aStatObj.st_mode & S_IWUSR) == S_IWUSR) {
    _result.readOnly = false;
  } else if (getgid() == aStatObj.st_gid &&
             (aStatObj.st_mode & S_IWGRP) == S_IWGRP) {
    _result.readOnly = false;
  } else if ((aStatObj.st_mode & S_IWOTH) == S_IWOTH) {
    _result.readOnly = false;
  }

  _result.isDirectory = S_ISDIR(aStatObj.st_mode);
  _result.isFile = S_ISREG(aStatObj.st_mode);
  _result.ctime = aStatObj.st_ctim.tv_sec;
  _result.mtime = aStatObj.st_mtim.tv_sec;
  _result.size = aStatObj.st_size;
  _result.nlink = 0;
  if (_result.isDirectory) {
    // Count entries in directory
    DIR* dir = opendir(path.c_str());
    if (!dir) {
      LoggerE("Cannot open directory: %s", GetErrorString(errno).c_str());
      return _result;
    }
    SCOPE_EXIT {
      (void) closedir(dir);
    };

    struct dirent entry;
    struct dirent *result = nullptr;
    int status;

    while ( (0 == (status = readdir_r(dir, &entry, &result) ) && result != nullptr) ) {
      std::string name = result->d_name;
      if (name == "." || name == "..") {
        continue;
      }
      _result.nlink++;
    }

    if (status != 0) {
      LoggerE("Cannot count files in directory: %s", GetErrorString(errno).c_str());
      return _result;
    }
  }

  _result.valid = true;
  return _result;
}
}  // namespace filesystem
}  // namespace extension
