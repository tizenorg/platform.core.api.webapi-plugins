// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_STAT_H
#define FILESYSTEM_FILESYSTEM_STAT_H

#include <string>
#include <common/picojson.h>

namespace extension {
namespace filesystem {

class FilesystemStat {
  FilesystemStat();

 public:
  bool valid;

  bool isFile;
  bool isDirectory;
  bool readOnly;
  uint64_t ctime;
  uint64_t mtime;
  size_t size;
  size_t nlink;

  picojson::value toJSON() const;

  static FilesystemStat getStat(const std::string& path);
};
}
}

#endif  // FILESYSTEM_FILESYSTEM_STAT_H
