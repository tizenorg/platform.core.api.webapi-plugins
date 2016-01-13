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

#ifndef COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_H_
#define COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_H_

#include <functional>
#include <vector>
#include <memory>
#include "common/filesystem/filesystem_storage.h"
#include "common/filesystem/filesystem_provider_types.h"

namespace common {

class IFilesystemProvider {
 public:
  IFilesystemProvider();
  virtual ~IFilesystemProvider();

  virtual void RegisterDeviceChangeState(DeviceChangeStateFun _callback) = 0;
  virtual void UnregisterDeviceChangeState() = 0;

  virtual Storages GetStorages() = 0;
  virtual VirtualRoots GetVirtualPaths() = 0;
  virtual VirtualStorages GetAllStorages() = 0;
  virtual std::shared_ptr<Storage> GetInternalStorage() = 0;
};

typedef IFilesystemProvider& FilesystemProviderRef;

class FilesystemProvider {
 public:
  static FilesystemProvider& Create();
  virtual ~FilesystemProvider();

  void RegisterDeviceChangeState(DeviceChangeStateFun _callback);
  void UnregisterDeviceChangeState();

  Storages GetStorages();
  VirtualRoots GetVirtualPaths();
  VirtualStorages GetAllStorages();
  std::shared_ptr<Storage> GetInternalStorage();

  std::string GetRealPath(const std::string& path_or_uri);
  std::string GetVirtualPath(const std::string& real_path) const;
 private:
  FilesystemProvider();
  FilesystemProvider(const FilesystemProvider&) = delete;
  FilesystemProvider& operator=(const FilesystemProvider&) = delete;
  FilesystemProvider(FilesystemProvider&&) = delete;
  FilesystemProvider& operator=(FilesystemProvider&&) = delete;
  FilesystemProviderRef provider_;
};

}  // namespace common

#endif  // COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_H_
