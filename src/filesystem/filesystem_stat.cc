// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem_stat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace extension {
namespace filesystem {

FilesystemStat::FilesystemStat() : valid(false) {}

picojson::value FilesystemStat::toJSON() const {
  picojson::value retval = picojson::value(picojson::object());
  picojson::object& obj = retval.get<picojson::object>();

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
  if (0 != stat(path.c_str(), &aStatObj)) {
    return FilesystemStat();
  }

  FilesystemStat _result;

  _result.readOnly = true;
  if (getuid() == aStatObj.st_uid && (aStatObj.st_mode & S_IWUSR) == S_IWUSR) {
    _result.readOnly = false;
  } else if (getgid() == aStatObj.st_gid &&
             (aStatObj.st_mode & S_IWGRP) == S_IWGRP) {
    _result.readOnly = false;
  } else if (aStatObj.st_mode & S_IWOTH == S_IWOTH) {
    _result.readOnly = false;
  }

  _result.isDirectory = S_ISDIR(aStatObj.st_mode);
  _result.isFile = S_ISREG(aStatObj.st_mode);
  _result.ctime = aStatObj.st_ctim.tv_sec;
  _result.mtime = aStatObj.st_mtim.tv_sec;
  _result.size = aStatObj.st_size;
  _result.nlink = aStatObj.st_nlink;

  _result.valid = true;
  return _result;
}
}
}
