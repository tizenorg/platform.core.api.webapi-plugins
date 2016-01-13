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

#include "common/filesystem/filesystem_provider_deviced.h"

#include <gio/gio.h>
#include <functional>
#include <list>
#include <map>
#include <algorithm>
#include <memory>
#include <utility>

#include "common/filesystem/filesystem_storage.h"
#include "common/logger.h"

namespace {
static const char* kIface = "org.tizen.system.deviced.BlockManager";
static const char* kPath = "/Org/Tizen/System/DeviceD/Block/Manager";
}  // namespace

namespace common {

struct DeviceListElem {
    DeviceListElem()
        : block_type(0),
          devnode(nullptr),
          syspath(nullptr),
          fs_usage(nullptr),
          fs_type(nullptr),
          fs_version(nullptr),
          fs_uuid_enc(nullptr),
          readonly(0),
          mount_point(nullptr),
          state(0),
          primary(false) {}

    int block_type;
    char* devnode;
    char* syspath;
    char* fs_usage;
    char* fs_type;
    char* fs_version;
    char* fs_uuid_enc;
    int readonly;
    char* mount_point;
    int state;
    bool primary;
};

FilesystemProviderDeviced::~FilesystemProviderDeviced() {
  LoggerD("Entered");
  UnregisterDeviceChangeState();
}

FilesystemProviderDeviced::FilesystemProviderDeviced() :
  dbus_(nullptr),
  proxy_(nullptr),
  device_changed_callback_(nullptr),
  block_signal_subscribe_id_(0),
  virtual_roots_provider_(FilesystemProviderStorage::Create()),
  is_initialized_(false) {
  LoggerD("Entered");

  GError* error = nullptr;

  dbus_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
  if (error != NULL) {
    LoggerE("Could not get dbus connection: %s", error->message);
  } else {
    if (!dbus_) {
      LoggerE("Dbus handle is null");
    } else {
      LoggerE("Dbus connection set");
      const gchar* unique_name = g_dbus_connection_get_unique_name(dbus_);
      proxy_ = g_dbus_proxy_new_sync(dbus_, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                     nullptr, unique_name, kPath, kIface, nullptr,
                                     &error);
      if (!proxy_ || error) {
        LoggerE("Could not get dbus proxy. %s", error->message);
      } else {
        is_initialized_ = true;
      }
    }
  }
}

FilesystemProviderDeviced& FilesystemProviderDeviced::Create() {
  LoggerD("Entered");

  static FilesystemProviderDeviced instance;

  return instance;
}

void FilesystemProviderDeviced::BlockSignalProxy(
    GDBusConnection* connection, const gchar* sender_name,
    const gchar* object_path, const gchar* interface_name,
    const gchar* signal_name, GVariant* parameters, gpointer user_data) {
  LoggerD("Entered");

  FilesystemProviderDeviced* instance =
    static_cast<FilesystemProviderDeviced*>(user_data);
  DeviceListElem elem;
  g_variant_get(parameters, "(issssssisib)", &elem.block_type, &elem.devnode,
                &elem.syspath, &elem.fs_usage, &elem.fs_type, &elem.fs_version,
                &elem.fs_uuid_enc, &elem.readonly, &elem.mount_point,
                &elem.state, &elem.primary);
  instance->BlockSignalCallback(elem);
}

void FilesystemProviderDeviced::BlockSignalCallback(DeviceListElem elem) {
  LoggerD("Entered");
  StorageState previous_state = StorageState::kUnmounted;
  auto it = previous_device_state_map_.find(elem.syspath);
  if (it == previous_device_state_map_.end()) {
    previous_device_state_map_[elem.syspath] = previous_state =
        (elem.state ? StorageState::kMounted : StorageState::kUnmounted);
  } else {
    previous_state = it->second;
  }
  if (device_changed_callback_ != nullptr) {
    std::shared_ptr<Storage> storage = GetStorage(elem);
    device_changed_callback_(*storage, previous_state, storage->state_);
  }
}

void FilesystemProviderDeviced::RegisterDeviceChangeState(
    DeviceChangeStateFun _callback) {
  LoggerD("Entered");

  if(!is_initialized_) {
    LoggerE("DeviceD Core api not initialized");
    return;
  }

  if (device_changed_callback_ == nullptr) {
    LoggerD("Registering dbus signal subscription");
    block_signal_subscribe_id_ = g_dbus_connection_signal_subscribe(
        dbus_, nullptr, "org.tizen.system.deviced.Block", "DeviceChanged",
        nullptr, nullptr, G_DBUS_SIGNAL_FLAGS_NONE, BlockSignalProxy, this,
        nullptr);
  }
  device_changed_callback_ = _callback;
}

void FilesystemProviderDeviced::UnregisterDeviceChangeState() {
  LoggerD("Entered");

  if(!is_initialized_) {
    LoggerE("DeviceD Core api not initialized");
    return;
  }

  if (0 != block_signal_subscribe_id_) {
    LoggerD("Dbus signal handling unsubscription");
    g_dbus_connection_signal_unsubscribe (dbus_, block_signal_subscribe_id_);
  }
  device_changed_callback_ = nullptr;
}

std::shared_ptr<Storage> FilesystemProviderDeviced::GetInternalStorage() {
  return virtual_roots_provider_.GetInternalStorage();
}

std::shared_ptr<Storage> FilesystemProviderDeviced::GetStorage(const DeviceListElem& elem) {
  LoggerD("Entered");
  return std::make_shared<Storage>(Storage(GetIdFromUUID(elem.fs_uuid_enc), StorageType::kExternal,
                 (elem.state ? StorageState::kMounted : StorageState::kUnmounted),
                 elem.syspath, GetNameFromPath(elem.devnode)));
}

std::string FilesystemProviderDeviced::GetNameFromPath(
    const char* const char_path) {
  LoggerD("Entered");
  std::string path = char_path;
  std::string name = "removable_";
  std::size_t last_slash_pos = path.find_last_of("/");
  if (last_slash_pos + 1 >= path.size()) {
    name += path;
    LoggerW("Failed to get device name from device syspath");
  } else {
    name += path.substr(last_slash_pos + 1);
  }
  return name;
}

int FilesystemProviderDeviced::GetIdFromUUID(const char* const char_uuid) {
  LoggerD("Entered");
  std::string uuid = char_uuid;
  size_t hyphen = uuid.find("-");
  std::string clear_uuid = uuid.substr(0, hyphen) + uuid.substr(hyphen + 1);
  return static_cast<int>(std::stoul(clear_uuid, 0, 16));
}

Storages FilesystemProviderDeviced::GetStorages() {
  LoggerD("Entered");

  if(!is_initialized_) {
    LoggerE("DeviceD Core api not initialized");
    return Storages();
  }

  GError* error = nullptr;
  GVariant* variant = g_dbus_proxy_call_sync(
      proxy_, "GetDeviceList", g_variant_new("(s)", "all"),
      G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
  if (!variant || error) {
    LoggerE("Failed to call GetDeviceList method - %s", error->message);
    return Storages();
  }
  LoggerD("GetDeviceList called");
  return GetStoragesFromGVariant(variant);
}

Storages FilesystemProviderDeviced::GetStoragesFromGVariant(GVariant* variant) {
  LoggerD("Entered");
  Storages storages;
  GVariantIter iter;
  GVariant* tuple = nullptr;
  g_variant_iter_init(&iter, variant);
  while ((tuple = g_variant_iter_next_value(&iter))) {
    DeviceListElem elem;
    g_variant_get(tuple, "(issssssisib)", &elem.block_type, &elem.devnode,
                  &elem.syspath, &elem.fs_usage, &elem.fs_type,
                  &elem.fs_version, &elem.fs_uuid_enc, &elem.readonly,
                  &elem.mount_point, &elem.state, &elem.primary);
    storages.push_back(GetStorage(elem));
    g_variant_unref(tuple);
  }
  return storages;
}

VirtualRoots FilesystemProviderDeviced::GetVirtualPaths() {
  LoggerD("Entered");

  if(!is_initialized_) {
    LoggerE("DeviceD Core api not initialized");
    return VirtualRoots();
  }

  return virtual_roots_provider_.GetVirtualPaths();
}

VirtualStorages FilesystemProviderDeviced::GetAllStorages() {
  LoggerD("Entered");

  if(!is_initialized_) {
    LoggerE("DeviceD Core api not initialized");
    return VirtualStorages();
  }

  std::lock_guard<std::mutex> lock(mutex_);
  VirtualStorages vs;
  for (auto storage : GetStorages()) {
    vs.push_back(storage);
  }

  for (auto virtual_root : virtual_roots_provider_.GetVirtualPaths()) {
    vs.push_back(std::make_shared<VirtualRoot>(virtual_root));
  }

  return vs;
}

}  // namespace common
