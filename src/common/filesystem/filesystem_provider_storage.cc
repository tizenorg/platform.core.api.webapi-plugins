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

#include "common/filesystem/filesystem_provider_storage.h"
#include <algorithm>
#include <map>
#include <storage.h>
#include "common/logger.h"
#include "common/current_application.h"

namespace {

const std::string kVirtualRootCamera = "camera";
const std::string kVirtualRootDocuments = "documents";
const std::string kVirtualRootDownloads = "downloads";
const std::string kVirtualRootImages = "images";
const std::string kVirtualRootMusic = "music";
const std::string kVirtualRootRingtones = "ringtones";
const std::string kVirtualRootVideos = "videos";
const std::string kVirtualRootWgtPackage = "wgt-package";
const std::string kVirtualRootWgtPrivate = "wgt-private";
const std::string kVirtualRootWgtPrivateTmp = "wgt-private-tmp";
const std::string kVirtualRootMedia = "internal0";

const std::map<storage_directory_e, const std::string*> kStorageDirectories = {
    { STORAGE_DIRECTORY_CAMERA, &kVirtualRootCamera },
    { STORAGE_DIRECTORY_DOCUMENTS, &kVirtualRootDocuments },
    { STORAGE_DIRECTORY_DOWNLOADS, &kVirtualRootDownloads },
    { STORAGE_DIRECTORY_IMAGES, &kVirtualRootImages },
    { STORAGE_DIRECTORY_MUSIC, &kVirtualRootMusic },
    { STORAGE_DIRECTORY_SYSTEM_RINGTONES, &kVirtualRootRingtones },
    { STORAGE_DIRECTORY_VIDEOS, &kVirtualRootVideos } };

const std::string kFileUriPrefix = "file://";

}  // namespace

namespace common {

StorageState TranslateCoreStorageState(
    storage_state_e coreStorageState) {
  LoggerD("Entered");
  StorageState state = StorageState::kUnknown;
  if (coreStorageState == STORAGE_STATE_REMOVED) {
    state = StorageState::kUnmounted;
  } else if (coreStorageState == STORAGE_STATE_MOUNTED
      || coreStorageState == STORAGE_STATE_MOUNTED_READ_ONLY) {
    state = StorageState::kMounted;
  } else {
    state = StorageState::kUnmountable;
  }

  return state;
}

void OnStorageChange(int storage_id, storage_state_e state, void* user_data) {
  LoggerD("Entered, id: %d", storage_id);

  FilesystemProviderStorage* provider =
      static_cast<FilesystemProviderStorage*>(user_data);
  for (auto &storage : provider->GetStorages()) {
    if (storage->id_ == storage_id) {
      StorageState previous_state = storage->state_;
      storage->state_ = TranslateCoreStorageState(state);
      if (provider->GetListener()) {
        provider->GetListener()(*storage, previous_state, storage->state_);
      }
      break;
    }
  }
}

bool OnForeachStorage(int storage_id, storage_type_e type,
                      storage_state_e state, const char* path,
                      void* user_data) {
  LoggerD("Entered, id: %d", storage_id);
  FilesystemProviderStorage* provider =
      static_cast<FilesystemProviderStorage*>(user_data);

  int err = storage_set_state_changed_cb(storage_id, OnStorageChange, provider);
  if (STORAGE_ERROR_NONE != err) {
    LoggerW("Failed to add listener");
  }

  StorageType type_ =
      type == STORAGE_TYPE_INTERNAL ?
          StorageType::kInternal : StorageType::kUnknown;

  provider->GetStorages().push_back(
      std::make_shared<Storage>(storage_id, type_, TranslateCoreStorageState(state), path));
  if (type_ == StorageType::kInternal) {
    // TODO check internal storage
    //provider->internal_storage_ = std::make_shared<Storage>(Storage(storage_id, type_, state_, path, kVirtualRootMedia));
    // if internal storage is supported, we can add also virtual paths:
    // downloads, documents etc
    provider->FillVirtualPaths(storage_id);
  }
  return true;
}

DeviceChangeStateFun FilesystemProviderStorage::GetListener() {
  return listener_;
}

FilesystemProviderStorage::FilesystemProviderStorage() :
    internal_storage_(nullptr) {
  LoggerD("Entered");
  int err = storage_foreach_device_supported(OnForeachStorage, this);
  if (err != STORAGE_ERROR_NONE) {
    LoggerE("Unknown Error on getting storage paths");
  }
}

FilesystemProviderStorage& FilesystemProviderStorage::Create() {
  LoggerD("Entered");
  static FilesystemProviderStorage fs;
  return fs;
}

FilesystemProviderStorage::~FilesystemProviderStorage() {
  LoggerD("Entered");
}

void FilesystemProviderStorage::FillVirtualPaths(int storage_id) {
  LoggerD("Creating virtual paths for storage: %d", storage_id);
  for (auto& item : kStorageDirectories) {
    char* dir_path = nullptr;
    storage_directory_e dir_enum = item.first;
    int err = storage_get_directory(storage_id, dir_enum, &dir_path);
    if (STORAGE_ERROR_NONE == err && nullptr != dir_path) {
      virtual_paths_.push_back(VirtualRoot(*(item.second), dir_path));
      free(dir_path);
    }
  }

  // fill also virtual paths based on current application install dir
  std::string root = common::CurrentApplication::GetInstance().GetRoot();
  if (!root.empty()) {
    virtual_paths_.push_back(
        VirtualRoot(kVirtualRootWgtPackage, root + "/res/wgt"));
    virtual_paths_.push_back(
        VirtualRoot(kVirtualRootWgtPrivate, root + "/data"));
    virtual_paths_.push_back(
        VirtualRoot(kVirtualRootWgtPrivateTmp, root + "/tmp"));
  }
}

void FilesystemProviderStorage::RegisterDeviceChangeState(
    DeviceChangeStateFun callback) {
  LoggerD("Entered");
  listener_ = callback;
}

void FilesystemProviderStorage::UnregisterDeviceChangeState() {
  LoggerD("Entered");
  listener_ = nullptr;
}

Storages FilesystemProviderStorage::GetStorages() {
  LoggerD("Entered");
  return storages_;
}

VirtualRoots FilesystemProviderStorage::GetVirtualPaths() {
  LoggerD("Entered");
  return virtual_paths_;
}

std::string FilesystemProviderStorage::GetRealPath(
    const std::string& path_or_uri) {
  LoggerD("Enter");
  std::string realpath;
  std::size_t pos = path_or_uri.find(kFileUriPrefix);
  if (pos != std::string::npos) {
    realpath = path_or_uri.substr(pos + kFileUriPrefix.size());
  } else {
    realpath = path_or_uri;
  }
  pos = realpath.find('/');
  if (pos != 0) {
    const std::string prefix = realpath.substr(0, pos);
    const auto it = std::find_if(virtual_paths_.begin(), virtual_paths_.end(),
                                 [prefix](const common::VirtualRoot & vr) {
                                   return vr.name_ == prefix;
                                 });
    if (it != virtual_paths_.end()) {
      realpath.replace(0, prefix.size(), it->path_);
    } else {
      LoggerE("Unknown virtual root");
    }
  }
  return realpath;
}

std::shared_ptr< Storage > FilesystemProviderStorage::GetInternalStorage(){
  LoggerD("Entered");
  return internal_storage_;
}

std::string FilesystemProviderStorage::GetVirtualPath(
    const std::string& real_path) const {
  LoggerD("Enter");
  for (const auto& kv : virtual_paths_) {
    if (0 == real_path.compare(0, kv.path_.size(), kv.path_)) {
      return std::string(real_path).replace(0, kv.path_.size(), kv.name_);
    }
  }
  return real_path;
}

VirtualStorages FilesystemProviderStorage::GetAllStorages() {
  LoggerD("Entered");
  VirtualStorages vs;
  for (auto storage : storages_) {
    vs.push_back(storage);
  }

  for (auto virtualRoot : virtual_paths_) {
    vs.push_back(std::make_shared < VirtualRoot > (virtualRoot));
  }

  return vs;
}

}  // namespace common
