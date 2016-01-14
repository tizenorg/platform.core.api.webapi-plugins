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

#include "common/filesystem/filesystem_storage.h"
#include <string>
#include "common/logger.h"

namespace common {

VirtualRoot::VirtualRoot(std::string const& name, std::string const& path,
                         StorageType type, StorageState state)
    : name_(name),
      path_(path),
      type_(type),
      state_(state) {
  LoggerD("Entered");
}

Storage::Storage(int id, StorageType type, StorageState state,
                 std::string const& path, std::string const& name)
    : VirtualRoot(name, path, type, state),
      id_(id) {
  LoggerD("Enter");
  if (name_ == "") {
    switch (type) {
      case StorageType::kInternal:
        name_ = "internal";
        break;
      case StorageType::kUsbDevice:
      case StorageType::kUsbHost:
      case StorageType::kMmc:
        name_ = "removable";
        break;
      default:
        name_ = "unknown";
        LoggerE("Unknown storage type: %d", type);
        break;
    }
    name_ += std::to_string(id);
  }
}

picojson::value VirtualRoot::ToJson() const {
  LoggerD("Entered");
  picojson::value v { picojson::object { } };
  picojson::object& obj = v.get<picojson::object>();

  obj["type"] = picojson::value(ToString(type_));
  obj["state"] = picojson::value(ToString(state_));
  obj["path"] = picojson::value(path_);
  obj["name"] = picojson::value(name_);

  return v;
}

picojson::value Storage::ToJson() const {
  LoggerD("Entered");
  picojson::value value = VirtualRoot::ToJson();
  picojson::object& obj = value.get<picojson::object>();
  obj["storage_id"] = picojson::value(static_cast<double>(id_));
  return value;
}

Storage::Storage(Storage const& other)
    : VirtualRoot(other) {
  LoggerD("Entered");
  this->id_ = other.id_;
}

VirtualRoot::VirtualRoot(VirtualRoot const& other) {
  LoggerD("Entered");
  this->path_ = other.path_;
  this->name_ = other.name_;
  this->state_ = other.state_;
  this->type_ = other.type_;
}

std::string VirtualRoot::ToString(StorageType type) {
  LoggerD("Entered");
  switch (type) {
    case StorageType::kInternal:
      return "INTERNAL";
    case StorageType::kUsbDevice:
    case StorageType::kUsbHost:
    case StorageType::kMmc:
      return "EXTERNAL";
    default:
      LoggerE("Unknown storage type: %d", type);
      return "UNKNOWN";
  }
}

std::string VirtualRoot::ToString(StorageState state) {
  LoggerD("Entered");
  switch (state) {
    case StorageState::kUnmounted:
      return "REMOVED";
    case StorageState::kUnmountable:
      return "UNMOUNTABLE";
    case StorageState::kMounted:
      return "MOUNTED";
    default:
      LoggerE("Unknown storage state: %d", state);
      return "UNKNOWN";
  }
}

}  // namespace common
