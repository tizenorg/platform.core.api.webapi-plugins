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
#ifndef FILESYSTEM_FILESYSTEM_STAT_H
#define FILESYSTEM_FILESYSTEM_STAT_H

#include <string>

#include "common/picojson.h"

#include "filesystem/filesystem_utils.h"

namespace extension {
namespace filesystem {

class FilesystemStat {
  FilesystemStat();

 public:
  FilesystemError error;
  bool valid;

  std::string path;
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
}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_STAT_H
