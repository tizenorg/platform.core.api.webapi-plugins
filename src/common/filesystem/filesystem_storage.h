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

#ifndef COMMON_FILESYSTEM_STORAGE_H_
#define COMMON_FILESYSTEM_STORAGE_H_

#include <string>
#include "common/picojson.h"

namespace common {

enum class StorageState {
  kMounted,
  kUnmounted,
  kUnmountable,
  kUnknown
};

enum class StorageType {
  kUnknown,
  kInternal,
  kUsbDevice,
  kUsbHost,
  kMmc
};

class VirtualStorage {
 public:
  virtual picojson::value ToJson() const = 0;
  virtual ~VirtualStorage() {
  }

  virtual std::string path() const = 0;
  virtual std::string name() const = 0;
  virtual StorageType type() const = 0;
  virtual StorageState state() const = 0;
};

class VirtualRoot : public VirtualStorage {
 public:
  VirtualRoot(std::string const& name, std::string const & path,
              StorageType type = StorageType::kInternal, StorageState state = StorageState::kMounted);
  VirtualRoot(VirtualRoot const& other);

  std::string name_;
  std::string path_;
  StorageType type_;
  StorageState state_;
  virtual picojson::value ToJson() const;
  virtual std::string path() const{
    return path_;
  }
  virtual std::string name() const {
    return name_;
  }
  virtual StorageType type() const{
    return type_;
  }
  virtual StorageState state() const{
    return state_;
  }

  static std::string ToString(StorageType type);
  static std::string ToString(StorageState state);
};

class Storage : public VirtualRoot {
 public:
  virtual ~Storage() {
  }

  Storage(int id, StorageType type, StorageState state,
          std::string const& path = "", std::string const& name = "");
  Storage(Storage const& other);

  int id_;
  virtual picojson::value ToJson() const;
};

}  // namespace common

#endif  // COMMON_FILESYSTEM_STORAGE_H_
