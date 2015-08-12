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

  bool Read(u_int8_t* data_p, size_t offset, size_t length,
            size_t* readed);
  bool Write(uint8_t* data_p, size_t data_size, size_t offset, size_t* written);
};

}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_FILE_H
