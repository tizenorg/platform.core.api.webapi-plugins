// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILESYSTEM_FILESYSTEM_MANAGER_H
#define FILESYSTEM_FILESYSTEM_MANAGER_H

#include <functional>
#include <string>
#include <vector>
#include <set>

#include "common/virtual_fs.h"

#include "filesystem_stat.h"
#include "filesystem_utils.h"

namespace extension {
namespace filesystem {

class FilesystemStateChangeListener {
 public:
  virtual ~FilesystemStateChangeListener() {}
  virtual void onFilesystemStateChangeSuccessCallback(
      const common::VirtualStorage& storage) = 0;
  virtual void onFilesystemStateChangeErrorCallback() = 0;
};

class FilesystemManager {
 private:
  FilesystemManager();
  ~FilesystemManager();
  FilesystemStateChangeListener* listener_;

  bool is_listener_registered_;
  std::set<int> ids_;

 public:
  static FilesystemManager& GetInstance();

  void UnlinkFile(
          const std::string& path,
          const std::function<void()>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);

  void StatPath(const std::string& path,
                const std::function<void(const FilesystemStat&)>& success_cb,
                const std::function<void(FilesystemError)>& error_cb);

  void FetchStorages(
      const std::function<void(const std::vector<common::VirtualStorage>&)>& success_cb,
      const std::function<void(FilesystemError)>& error_cb);

  void GetVirtualRoots(
      const std::function<void(const std::vector<common::VirtualRoot>&)>& success_cb,
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

  void FileRead(const std::string& path,
                size_t offset,
                size_t length,
                const std::function<void(const std::string&)>& success_cb,
                const std::function<void(FilesystemError)>& error_cb);

  void FileWrite(const std::string& path,
                 const std::string& data,
                 size_t offset,
                 const std::function<void()>& success_cb,
                 const std::function<void(FilesystemError)>& error_cb);

void CopyTo(const std::string& originFilePath,
          const std::string& destinationFilePath,
          const bool overwrite,
          const std::function<void()>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);

  void StartListening();
  void StopListening();
  void AddListener(FilesystemStateChangeListener* listener);
};
}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_MANAGER_H
