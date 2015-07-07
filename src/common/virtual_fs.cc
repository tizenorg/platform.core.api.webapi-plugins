// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/virtual_fs.h"

#include <map>

#include <app_manager.h>
#include <package_manager.h>

#include "common/extension.h"
#include "common/logger.h"
#include "common/optional.h"
#include "common/scope_exit.h"
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

const std::string kFileUriPrefix = "file://";

const std::map<storage_directory_e, const std::string*> kStorageDirectories = {
  {STORAGE_DIRECTORY_CAMERA, &kVirtualRootCamera},
  {STORAGE_DIRECTORY_DOCUMENTS, &kVirtualRootDocuments},
  {STORAGE_DIRECTORY_DOWNLOADS, &kVirtualRootDownloads},
  {STORAGE_DIRECTORY_IMAGES, &kVirtualRootImages},
  {STORAGE_DIRECTORY_MUSIC, &kVirtualRootMusic},
  {STORAGE_DIRECTORY_SYSTEM_RINGTONES, &kVirtualRootRingtones},
  {STORAGE_DIRECTORY_VIDEOS, &kVirtualRootVideos}
};

std::map<int, common::VirtualStorage> g_storages;

std::map<const std::string, common::VirtualRoot> g_virtual_roots;

void AddVirtualRoot(const std::string& name, const std::string& path) {
  LoggerD("Enter");
  g_virtual_roots.insert(std::make_pair(name, common::VirtualRoot(name, path)));
}

void OnStorageStateChanged(int storage_id, storage_state_e state, void *user_data) {
  LoggerD("Enter");
  const auto it = g_storages.find(storage_id);

  if (it != g_storages.end()) {
    it->second.state_ = state;
  } else {
    LoggerE("Unknown storage: %d", storage_id);
  }
}

bool OnStorageDeviceSupported(int storage_id, storage_type_e type,
                              storage_state_e state, const char *path,
                              void *user_data) {
  LoggerD("Enter");
  g_storages.insert(std::make_pair(storage_id, common::VirtualStorage(storage_id, type, state, path)));

  storage_set_state_changed_cb(storage_id, OnStorageStateChanged, nullptr);

  if (STORAGE_TYPE_INTERNAL == type) {
    // virtual roots are only on internal storage
    for (const auto& kv : kStorageDirectories) {
      char* directory = nullptr;
      storage_directory_e dir = kv.first;

      int err = storage_get_directory(storage_id, dir, &directory);

      if (STORAGE_ERROR_NONE == err  && nullptr != directory) {
        AddVirtualRoot(*kv.second, directory);
        free(directory);
      }
    }
  }

  return true;
}

common::optional<std::string> GetRootDir() {
  LoggerD("Enter");
  std::string app_id = common::CurrentApplication::GetInstance().GetApplicationId();

  app_info_h app_info;
  int err = app_info_create(app_id.c_str(), &app_info);
  if (APP_MANAGER_ERROR_NONE != err) {
    LoggerE("Can't create app info handle from appId (%s)", app_id.c_str());
    return nullptr;
  }
  SCOPE_EXIT {
    app_info_destroy(app_info);
  };

  char* package = nullptr;
  err = app_info_get_package(app_info, &package);
  if (APP_MANAGER_ERROR_NONE != err) {
    LoggerE("Can't get package name from app info (%s)", get_error_message(err));
    return nullptr;
  }
  SCOPE_EXIT {
    free(package);
  };

  package_info_h pkg_info;
  err = package_info_create(package, &pkg_info);
  if (PACKAGE_MANAGER_ERROR_NONE != err) {
    LoggerE("Can't create package info handle from pkg (%s)", get_error_message(err));
    return nullptr;
  }
  SCOPE_EXIT {
    package_info_destroy(pkg_info);
  };

  char* root = nullptr;
  err = package_info_get_root_path(pkg_info, &root);
  if (PACKAGE_MANAGER_ERROR_NONE != err || nullptr == root) {
    LoggerE("Can't get root path from package info (%s)", get_error_message(err));
    return nullptr;
  }

  std::string ret{root};
  free(root);

  return ret;
}

}  // namespace

namespace common {

std::string to_string(storage_type_e type) {
  switch (type) {
    case STORAGE_TYPE_INTERNAL:
      return "INTERNAL";

    case STORAGE_TYPE_EXTERNAL:
      return "EXTERNAL";

    default:
      LoggerE("Unknown storage type: %d", type);
      return "UNKNOWN";
  }
}

std::string to_string(storage_state_e state) {
  switch (state) {
    case STORAGE_STATE_UNMOUNTABLE:
      return "UNMOUNTABLE";

    case STORAGE_STATE_REMOVED:
      return "REMOVED";

    case STORAGE_STATE_MOUNTED:
    case STORAGE_STATE_MOUNTED_READ_ONLY:
      return "MOUNTED";

    default:
      LoggerE("Unknown storage state: %d", state);
      return "UNKNOWN";
  }
}

VirtualRoot::VirtualRoot(const std::string& name, const std::string& path)
    : name_(name),
      path_(path) {
}

picojson::value VirtualRoot::ToJson() const {
  picojson::value v{picojson::object{}};
  picojson::object& obj = v.get<picojson::object>();

  obj["name"] = picojson::value(name_);
  obj["path"] = picojson::value(path_);

  return v;
}

VirtualStorage::VirtualStorage(int id, storage_type_e type,
                               storage_state_e state, const std::string& path)
    : id_(id),
      type_(type),
      state_(state),
      path_(path) {
  LoggerD("Enter");
  switch (type_) {
    case STORAGE_TYPE_INTERNAL:
      name_ = "internal";
      break;

    case STORAGE_TYPE_EXTERNAL:
      name_ = "removable";
      break;

    default:
      name_ = "unknown";
      LoggerE("Unknown storage type: %d", type);
      break;
  }
  name_ += std::to_string(id_);
}

picojson::value VirtualStorage::ToJson() const {
  picojson::value v{picojson::object{}};
  picojson::object& obj = v.get<picojson::object>();

  obj["storage_id"] = picojson::value(static_cast<double>(id_));
  obj["type"] = picojson::value(to_string(type_));
  obj["state"] = picojson::value(to_string(state_));
  obj["path"] = picojson::value(path_);
  obj["name"] = picojson::value(name_);

  return v;
}

VirtualFs::VirtualFs() : app_root_(GetRootDir()) {
  LoggerD("Enter");
  if (app_root_) {
    AddVirtualRoot(kVirtualRootWgtPackage, *app_root_ + "/res/wgt");
    AddVirtualRoot(kVirtualRootWgtPrivate, *app_root_ + "/data");
    AddVirtualRoot(kVirtualRootWgtPrivateTmp, *app_root_ + "/tmp");
  }

  int err = storage_foreach_device_supported(OnStorageDeviceSupported, nullptr);

  if (err != STORAGE_ERROR_NONE) {
    LoggerE("Unknown Error on getting storage paths");
  }

  int id = -1;
  // add virtual roots to storages
  for (const auto kw : g_virtual_roots) {
    const int storage_id = id--;
    VirtualStorage vs{storage_id, STORAGE_TYPE_INTERNAL, STORAGE_STATE_MOUNTED, kw.second.path_};
    vs.name_ = kw.second.name_;
    g_storages.insert(std::make_pair(storage_id, vs));
  }
}

VirtualFs::~VirtualFs() {
  LoggerD("Enter");
  for (const auto kv : g_storages) {
    storage_unset_state_changed_cb(kv.second.id_, OnStorageStateChanged);
  }
}

VirtualFs& VirtualFs::GetInstance() {
  static VirtualFs vfs;
  return vfs;
}

optional<std::string> VirtualFs::GetVirtualRootDirectory(const std::string& name) const {
  LoggerD("Enter");
  const auto it = g_virtual_roots.find(name);
  if (it != g_virtual_roots.end()) {
    return it->second.path_;
  }
  return nullptr;
}
optional<std::string> VirtualFs::GetApplicationDirectory() const {
  return app_root_;
}

std::string VirtualFs::GetRealPath(const std::string& path_or_uri) const {
  LoggerD("Enter");
  SLoggerD("Input: [%s]", path_or_uri.c_str());
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
    const auto it = g_virtual_roots.find(prefix);
    if (it != g_virtual_roots.end()) {
      realpath.replace(0, prefix.size(), it->second.path_);
    } else {
      LoggerE("Unknown virtual root");
    }
  }
  SLoggerD("Exit: [%s]", realpath.c_str());
  return realpath;
}

std::string VirtualFs::GetVirtualPath(const std::string& real_path) const {
  LoggerD("Enter");
  for (const auto& kv : g_virtual_roots) {
    if (0 == real_path.compare(0, kv.second.path_.size(), kv.second.path_)) {
      return std::string(real_path).replace(0, kv.second.path_.size(), kv.first);
    }
  }
  return real_path;
}

std::vector<VirtualRoot> VirtualFs::GetVirtualRoots() const {
  LoggerD("Enter");
  std::vector<VirtualRoot> r;

  for (const auto kv : g_virtual_roots) {
    r.push_back(kv.second);
  }

  return r;
}

std::vector<VirtualStorage> VirtualFs::GetStorages() const {
  LoggerD("Enter");
  std::vector<VirtualStorage> r;

  for (const auto kv : g_storages) {
    r.push_back(kv.second);
  }

  return r;
}

}  // namespace common
