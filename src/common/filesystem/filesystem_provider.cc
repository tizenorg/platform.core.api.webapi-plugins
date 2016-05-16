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

#include "common/filesystem/filesystem_provider.h"
#include "common/filesystem/filesystem_provider_deviced.h"
#include "common/filesystem/filesystem_provider_storage.h"

namespace common {

IFilesystemProvider::IFilesystemProvider() {
  LoggerD("enter");

}

IFilesystemProvider::~IFilesystemProvider() {
  LoggerD("enter");
}


FilesystemProvider::FilesystemProvider() :
    provider_ (FilesystemProviderDeviced::Create())
{
}

FilesystemProvider& FilesystemProvider::Create() {
  LoggerD("Entered");
  static FilesystemProvider instance;
  return instance;
}

FilesystemProvider::~FilesystemProvider() {
  LoggerD("Entered");
}

void FilesystemProvider::RegisterDeviceChangeState(
    DeviceChangeStateFun callback) {
  LoggerD("Entered");
  provider_.RegisterDeviceChangeState(callback);
}

void FilesystemProvider::UnregisterDeviceChangeState() {
  LoggerD("Entered");
  provider_.UnregisterDeviceChangeState();
}

Storages FilesystemProvider::GetStorages() {
  LoggerD("Entered");
  return provider_.GetStorages();
}

VirtualRoots FilesystemProvider::GetVirtualPaths() {
  LoggerD("Entered");
  return provider_.GetVirtualPaths();
}

VirtualStorages FilesystemProvider::GetAllStorages() {
  LoggerD("Entered");
  return provider_.GetAllStorages();
}

std::shared_ptr< Storage > FilesystemProvider::GetInternalStorage(){
  LoggerD("Entered");
  return provider_.GetInternalStorage();
}

std::string FilesystemProvider::GetRealPath(
    const std::string& path_or_uri) {
  LoggerD("Entered");
  return FilesystemProviderStorage::Create().GetRealPath(path_or_uri);
}

std::string FilesystemProvider::GetVirtualPath(
    const std::string& real_path) const {
  LoggerD("Entered");
  return FilesystemProviderStorage::Create().GetVirtualPath(real_path);
}

}  // namespace common
