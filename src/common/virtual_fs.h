// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_VIRTUAL_FS_H_
#define COMMON_VIRTUAL_FS_H_

#include <string>
#include <vector>

#include <storage.h>

#include "common/optional.h"
#include "common/picojson.h"

namespace common {

std::string to_string(storage_type_e type);
std::string to_string(storage_state_e state);

struct VirtualRoot {
  VirtualRoot(const std::string& name, const std::string& path);
  picojson::value ToJson() const;
  std::string name_;
  std::string path_;
};

struct VirtualStorage {
  VirtualStorage(int id, storage_type_e type, storage_state_e state,
                 const std::string& path = "");
  picojson::value ToJson() const;
  int id_;
  storage_type_e type_;
  storage_state_e state_;
  std::string path_;
  std::string name_;
};

class VirtualFs {
 public:
  /**
   * Get absolute path of Virtual Roots
   *
   * @return absolute path on success
   */
  optional<std::string> GetVirtualRootDirectory(const std::string& name) const;

  /**
   * Get root directory of current application.
   *
   * @return absolute path on success
   */
  optional<std::string> GetApplicationDirectory() const;

  /**
   * Get real path (absolute path) from virtual path
   *
   * @remark
   * this function never fail.
   * if you want virtual path is not exist, check with getVirtualRoot()
   *
   * @return real path
   */
  std::string GetRealPath(const std::string& path_or_uri) const;

  /**
   * Get virtual path from real path
   *
   * @return virtual path
   */
  std::string GetVirtualPath(const std::string& real_path) const;

  std::vector<VirtualRoot> GetVirtualRoots() const;

  std::vector<VirtualStorage> GetStorages() const;

  // TODO: add stat method

  static VirtualFs& GetInstance();

 private:
  VirtualFs();
  VirtualFs(const VirtualFs&) = delete;
  VirtualFs& operator =(const VirtualFs&) = delete;
  ~VirtualFs();

  optional<std::string> app_root_;
};

}  // namespace common

#endif  // COMMON_VIRTUAL_FS_H_
