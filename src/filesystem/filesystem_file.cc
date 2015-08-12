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

#include "filesystem_file.h"
#include <stdio.h>
#include <unistd.h>
#include <common/scope_exit.h>
#include <common/logger.h>

namespace extension {
namespace filesystem {

namespace {
uint8_t characterToNumber(char c) {
  if (c == '+') {
    return 62;
  }
  if (c == '/') {
    return 63;
  }
  if (c <= '9') {
    return c + 0x04;
  }
  if (c <= 'Z') {
    return c - 0x41;
  }
  if (c <= 'z') {
    return c - 0x47;
  }
  return 0;
}

char numberToCharacter(uint8_t i) {
  if (i <= 25) {
    return 'A' + i;
  }
  if (i <= 51) {
    return 'a' + i - 26;
  }
  if (i <= 61) {
    return '0' + i - 52;
  }
  if (i == 62) {
    return '+';
  }
  if (i == 63) {
    return '/';
  }
  return 0;
}

bool validateCharacter(char c) {
  if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') || (c == '=') || (c == '+') || (c == '/')) {
    return true;
  }
  return false;
}
}  // namespace

/**
 * Data is encoded using Base64 encoding.
 */

bool FilesystemBuffer::DecodeData(const std::string& data) {
  LoggerD("Enter");
  if (data.length() % 4) {
    LoggerE("Buffer has invalid length");
    return false;
  }

  for (auto c : data) {
    if (!validateCharacter(c)) {
      LoggerE("Buffer has invalid character");
      return false;
    }
  }

  // Validate padding
  for (size_t i = 0; i + 2 < data.length(); ++i) {
    if (data[i] == '=') {
      LoggerE("Unexpected padding character in string");
      return false;
    }
  }

  if (data[data.length() - 2] == '=' && data[data.length() - 1] != '=') {
    LoggerE("Unexpected padding character in string");
    return false;
  }

  clear();

  if (data.length() == 0) {
    return true;
  }

  int padding = 0;
  if (data[data.length() - 1] == '=') {
    padding++;
  }

  if (data[data.length() - 2] == '=') {
    padding++;
  }

  for (size_t i = 0; i < data.length(); i += 4) {
    uint8_t part[] = {
        characterToNumber(data[i + 0]), characterToNumber(data[i + 1]),
        characterToNumber(data[i + 2]), characterToNumber(data[i + 3])};
    push_back(uint8_t((part[0] << 2) | (part[1] >> 4)));
    if ((data.length() - i != 4) || (padding < 2)) {
      push_back(uint8_t((part[1] << 4) | (part[2] >> 2)));
    }
    if ((data.length() - i != 4) || (padding < 1)) {
      push_back(uint8_t((part[2] << 6) | (part[3])));
    }
  }
  return true;
}

std::string FilesystemBuffer::EncodeData() const {
  LoggerD("Enter");
  std::string out;

  for (size_t i = 0; i < size(); i += 3) {
    uint8_t part[] = {safe_get(i), safe_get(i + 1), safe_get(i + 2)};
    out.push_back(numberToCharacter(0x3F & (part[0] >> 2)));
    out.push_back(numberToCharacter(0x3F & ((part[0] << 4) | (part[1] >> 4))));
    out.push_back(numberToCharacter(0x3F & ((part[1] << 2) | (part[2] >> 6))));
    out.push_back(numberToCharacter(0x3F & (part[2])));
  }

  if (out.size() == 0)
    return out;

  // Add padding
  int fillup = (size() % 3);
  if (fillup == 1) {
    out[out.size() - 2] = '=';
  }

  if (fillup == 1 || fillup == 2) {
    out[out.size() - 1] = '=';
  }

  return out;
}

FilesystemFile::FilesystemFile(const std::string& path_)
    : path(path_) {}

bool FilesystemFile::Read(uint8_t* data_p,
                               size_t offset,
                               size_t length,
                               size_t* readed) {
  LoggerE("entered");
  size_t temp_read = 0;
  if (!readed) {
    readed = &temp_read;
  }
  FILE* file = fopen(path.c_str(), "r");
  if (!file) {
    LoggerE("Cannot open file %s to read!", path.c_str());
    return false;
  }
  SCOPE_EXIT {
    int status = fclose(file);
    if (status) {
      LoggerE("Cannot close file!");
    }
  };
  int status;
  status = fseek(file, offset, SEEK_SET);
  if (status) {
    LoggerE("Cannot perform seek!");
    return false;
  }

  size_t data_size = length;
  while (*readed < data_size) {
    size_t part = fread(data_p, 1, length, file);

    *readed += part;
    data_p += part;
    data_size -= part;

    LoggerD("Readed part %li bytes", *readed);

    if (ferror(file)) {
      LoggerE("Error during file write!");
      return false;
    }

    if (feof(file)) {
      LoggerD("File is at end before buffer is filled. Finish.");
      break;
    }
  }
  LoggerD("Readed %li bytes", *readed);
  return true;
}

bool FilesystemFile::Write(uint8_t* data_p, size_t data_size, size_t offset,
                           size_t* written) {
  LoggerD("Enter %s", path.c_str());
  FILE* file = fopen(path.c_str(), "r+");
  if (!file) {
    LoggerE("Cannot open file %s to write!", path.c_str());
    return false;
  }

  SCOPE_EXIT {
    int status = fclose(file);
    if (status) {
      LoggerE("Cannot close file!");
    }
  };

  int status;
  status = fseek(file, offset, SEEK_SET);
  LoggerD("Offset is %li, writing %i bytes", offset, data_size);
  if (status) {
    LoggerE("Cannot perform seek!");
    return false;
  }

  while (*written < data_size) {
    size_t part = fwrite(data_p, 1, data_size, file);

    if (ferror(file)) {
      LoggerE("Error during file write!");
      return false;
    }

    *written += part;
    data_p += part;
    data_size -= part;
  }

  status = fflush(file);
  if (status) {
    LoggerE("Cannot flush file!");
    return false;
  }

  status = fsync(fileno(file));
  if (status) {
    LoggerE("Cannot sync file!");
    return false;
  }
  LoggerD("Written %li bytes", *written);

  return true;
}


}  // namespace filesystem
}  // namespace extension
