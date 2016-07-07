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

#ifndef COMMON_FILESYSTEM_PROVIDER_DEVICED_H
#define COMMON_FILESYSTEM_PROVIDER_DEVICED_H

#include <gio/gio.h>

#include <functional>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <mutex>

#include "common/filesystem/filesystem_storage.h"
#include "common/filesystem/filesystem_provider.h"
#include "common/filesystem/filesystem_provider_storage.h"

namespace common {

struct DeviceListElem;
struct UsbListElem;

class FilesystemProviderDeviced : public IFilesystemProvider {
 public:

  virtual ~FilesystemProviderDeviced();
  virtual void RegisterDeviceChangeState(DeviceChangeStateFun _callback);
  virtual void UnregisterDeviceChangeState();
  virtual VirtualRoots GetVirtualPaths();
  virtual Storages GetStorages();
  virtual VirtualStorages GetAllStorages();
  static FilesystemProviderDeviced& Create();

 private:
  FilesystemProviderDeviced();

  static void BlockSignalProxy(GDBusConnection* connection,
                               const gchar* sender_name, const gchar* object_path,
                               const gchar* interface_name,
                               const gchar* signal_name, GVariant* parameters,
                               gpointer user_data);
  void BlockSignalCallback(DeviceListElem elem);

  static std::string GetNameFromPath(const char* const char_path);
  static int GetIdFromUUID(const char* const char_uuid);
  static std::shared_ptr<Storage> GetStorage(const DeviceListElem& elem);
  static std::shared_ptr<Storage> GetStorageUsb(const UsbListElem& elem);
  static Storages GetStoragesFromGVariant(GVariant* variant);
  GDBusConnection* dbus_;
  GDBusProxy* proxy_;

  DeviceChangeStateFun device_changed_callback_;
  guint block_signal_subscribe_id_;
  std::map<std::string, StorageState> previous_device_state_map_;
  FilesystemProviderRef virtual_roots_provider_;
  std::mutex mutex_;

  bool is_initialized_;
};

}  // namespace common

#endif /* DEVICED_H */
