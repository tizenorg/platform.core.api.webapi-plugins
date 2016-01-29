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

#ifndef COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_STORAGE_H_
#define COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_STORAGE_H_

#include <string>
#include <memory>
#include "common/filesystem/filesystem_provider_types.h"
#include "common/filesystem/filesystem_provider.h"

namespace common {

class FilesystemProviderStorage : public IFilesystemProvider {
 public:
  static FilesystemProviderStorage& Create();
  virtual ~FilesystemProviderStorage();

  virtual void RegisterDeviceChangeState(DeviceChangeStateFun callback);
  virtual void UnregisterDeviceChangeState();

  virtual Storages GetStorages();
  virtual VirtualRoots GetVirtualPaths();
  virtual VirtualStorages GetAllStorages();
  virtual std::shared_ptr< Storage > GetInternalStorage();

  std::string GetRealPath(const std::string& path_or_uri);
  std::string GetVirtualPath(const std::string& real_path) const;

  DeviceChangeStateFun GetListener();
  void FillVirtualPaths(int storage_id);
  void AddStorage(std::shared_ptr<Storage> storage);

 private:
  FilesystemProviderStorage();
  /**
   * For given storage_id try to get paths for virtual paths.
   * For example for storage_id (which has type INTERNAL) it will
   * add to storages virtual paths: downloads with real path /opt/usr/media/Downloads
   */
  DeviceChangeStateFun listener_;
  Storages storages_;
  VirtualRoots virtual_paths_;
  std::shared_ptr<Storage> internal_storage_;
};

}  // namespace common

#endif  // COMMON_FILESYSTEM_FILESYSTEM_PROVIDER_STORAGE_H_
