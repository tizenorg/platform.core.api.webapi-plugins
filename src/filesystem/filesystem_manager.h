// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_MANAGER_H
#define FILESYSTEM_FILESYSTEM_MANAGER_H

#include <functional>
#include <string>
#include <vector>

#include "filesystem_stat.h"
#include "filesystem_storage.h"
#include "filesystem_utils.h"

namespace extension {
namespace filesystem {

enum class FilesystemError {
  None,
  NotFound,
  Other
};

class FilesystemManager {
 private:
  FilesystemManager();

 public:
  static FilesystemManager& GetInstance();

  void StatPath(const std::string& path,
                const std::function<void(const FilesystemStat&)>& success_cb,
                const std::function<void(FilesystemError)>& error_cb);

  void FetchStorages(const std::function<void(
                         const std::vector<FilesystemStorage>&)>& success_cb,
                     const std::function<void(FilesystemError)>& error_cb);

  void GetWidgetPaths(
      const std::function<void(const std::map<std::string, std::string>&)>&
          success_cb,
      const std::function<void(FilesystemError)>& error_cb);
};
}
}

#endif  // FILESYSTEM_FILESYSTEM_MANAGER_H
