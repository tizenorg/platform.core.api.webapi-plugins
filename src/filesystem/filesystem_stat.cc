// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem_stat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <common/logger.h>
#include <common/scope_exit.h>

namespace extension {
namespace filesystem {

FilesystemStat::FilesystemStat() : error(FilesystemError::None), valid(false) {}

picojson::value FilesystemStat::toJSON() const {
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
  struct stat aStatObj;
  FilesystemStat _result;

  LoggerD("enter");

  if (0 != stat(path.c_str(), &aStatObj)) {
    LoggerE("Failed to stat: (%d) %s", errno, strerror(errno));
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
      LoggerE("Cannot open directory: %s", strerror(errno));
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
      LoggerE("Cannot count files in directory: %s", strerror(errno));
      return _result;
    }
  }

  _result.valid = true;
  return _result;
}
}  // namespace filesystem
}  // namespace extension
