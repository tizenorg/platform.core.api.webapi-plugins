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

#ifndef FILESYSTEM_FILESYSTEM_MANAGER_H
#define FILESYSTEM_FILESYSTEM_MANAGER_H

#include <functional>
#include <string>
#include <vector>
#include <set>
#include <memory>

#include "common/filesystem/filesystem_provider_storage.h"

#include "filesystem/filesystem_stat.h"
#include "filesystem/filesystem_utils.h"

#include "common/filesystem/filesystem_storage.h"
#include "common/filesystem/filesystem_provider_storage.h"

namespace extension {
namespace filesystem {

class FilesystemStateChangeListener {
 public:
  virtual ~FilesystemStateChangeListener() {}
  virtual void onFilesystemStateChangeSuccessCallback(
      const common::Storage& storage) = 0;
  virtual void onFilesystemStateChangeErrorCallback() = 0;
};

class FilesystemManager {
 private:
  FilesystemManager();
  FilesystemStateChangeListener* listener_;

  common::FilesystemProviderStorage& fs_provider_;

 public:

  virtual ~FilesystemManager();

  static FilesystemManager& GetInstance();

  void UnlinkFile(
          const std::string& path,
          const std::function<void()>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);

  void StatPath(const std::string& path,
                const std::function<void(const FilesystemStat&)>& success_cb,
                const std::function<void(FilesystemError)>& error_cb);

  void FetchStorages(
      const std::function<void(const common::Storages&)>& success_cb,
      const std::function<void(FilesystemError)>& error_cb);

  void GetVirtualRoots(
      const std::function<void(const common::VirtualRoots&)>& success_cb,
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
  void OnStorageDeviceChanged(common::Storage const& _virtualStorage,
                          common::StorageState _old, common::StorageState _new);
  void AddListener(FilesystemStateChangeListener* listener);
  void RemoveListener();
};
}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_MANAGER_H
