// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_FILE_H
#define FILESYSTEM_FILESYSTEM_FILE_H

#include <string>
#include <vector>
#include <stdint.h>

namespace extension {
namespace filesystem {

class FilesystemBuffer : public std::vector<uint8_t> {
 public:
  bool DecodeData(const std::string& data);
  std::string EncodeData() const;

 private:
  inline uint8_t safe_get(size_t i) const {
    if (i >= size()) {
      return 0;
    }
    return at(i);
  }
};

class FilesystemFile {
  const std::string path;

 public:
  FilesystemFile(const std::string& path_);

  bool Read(FilesystemBuffer* data, size_t offset, size_t length);
  bool Write(const FilesystemBuffer& data, size_t offset);
};

}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_FILE_H
