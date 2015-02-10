// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_STORAGE_H
#define FILESYSTEM_FILESYSTEM_STORAGE_H

#include <string>
#include <common/picojson.h>
#include <storage-expand.h>

namespace extension {
namespace filesystem {

struct StoragePaths {
  std::string images;
  std::string sounds;
  std::string videos;
  std::string camera;
  std::string downloads;
  std::string music;
  std::string documents;
  std::string others;
  std::string ringtones;
};

class FilesystemStorage {
 public:
  FilesystemStorage(int storage_id,
                    storage_type_e type,
                    storage_state_e,
                    const char* path);

  int storage_id;
  std::string type;
  std::string state;
  std::string path;
  std::string name;
  StoragePaths paths;

  picojson::value toJSON() const;
};
}
}

#endif  // FILESYSTEM_FILESYSTEM_STORAGE_H
