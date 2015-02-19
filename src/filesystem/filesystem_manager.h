// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_MANAGER_H
#define FILESYSTEM_FILESYSTEM_MANAGER_H

#include <functional>
#include <string>
#include <vector>
#include <set>

#include "filesystem_stat.h"
#include "filesystem_storage.h"
#include "filesystem_utils.h"

namespace extension {
namespace filesystem {

class FilesystemStateChangeListener {
 public:
  virtual void onFilesystemStateChangeSuccessCallback(
      const std::string& label, const std::string& state,
      const std::string& type) = 0;
  virtual void onFilesystemStateChangeErrorCallback() = 0;
};

class FilesystemManager {
 private:
  FilesystemManager();
  ~FilesystemManager();
  FilesystemStateChangeListener* listener_;

 public:
  static FilesystemManager& GetInstance();

  void UnlinkFile(
          const std::string& path,
          const std::function<void()>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);

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

  void CreateFile(const std::string& path,
                  const std::function<void(const FilesystemStat&)>& success_cb,
                  const std::function<void(FilesystemError)>& error_cb);

  void Rename(const std::string& oldPath,
              const std::string& newPath,
              const std::function<void(const FilesystemStat&)>& success_cb,
              const std::function<void(FilesystemError)>& error_cb);

  void MakeDirectory(const std::string& path,
                     const std::function<void(FilesystemError)>& result_cb);
  
  void ReadDir(
          const std::string& path,
          const std::function<void(const std::vector<std::string>&)>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);

  void RemoveDirectory(
          const std::string& path,
          const std::function<void()>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);
  void StartListening();
  void StopListening();
  void AddListener(FilesystemStateChangeListener* listener);

  bool is_listener_registered_;
  static std::vector<FilesystemStorage> storages;
  std::set<int> ids_;
};
}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_MANAGER_H
