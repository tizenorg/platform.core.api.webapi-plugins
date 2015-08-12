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

  std::vector<common::VirtualStorage> FillStorages();
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
                const std::function<void(const std::string&, uint8_t*, size_t)>& success_cb,
                const std::function<void(FilesystemError)>& error_cb);

  void FileWrite(const std::string& path,
                 uint8_t* data_p,
                 size_t data_size,
                 size_t offset,
                 const std::function<void(size_t data_size)>& success_cb,
                 const std::function<void(FilesystemError)>& error_cb);

void CopyTo(const std::string& originFilePath,
          const std::string& destinationFilePath,
          const bool overwrite,
          const std::function<void()>& success_cb,
          const std::function<void(FilesystemError)>& error_cb);

  void StartListening();
  void StopListening();
  void AddListener(FilesystemStateChangeListener* listener);
  void RemoveListener();
};
}  // namespace filesystem
}  // namespace extension

#endif  // FILESYSTEM_FILESYSTEM_MANAGER_H
