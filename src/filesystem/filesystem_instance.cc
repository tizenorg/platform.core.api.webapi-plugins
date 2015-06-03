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

#include "filesystem/filesystem_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"
#include "filesystem_manager.h"

namespace extension {
namespace filesystem {

namespace {
// The privileges that required in Filesystem API
const std::string kPrivilegeFilesystem = "";

}  // namespace

using namespace common;
using namespace extension::filesystem;

FilesystemInstance::FilesystemInstance() {
  LoggerD("Enter");
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&FilesystemInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&FilesystemInstance::x, this, _1, _2));

  REGISTER_ASYNC("File_stat", FileStat);
  REGISTER_SYNC("File_statSync", FileStatSync);
  REGISTER_SYNC("File_createSync", FileCreateSync);
  REGISTER_ASYNC("File_readDir", ReadDir);
  REGISTER_ASYNC("File_rename", FileRename);
  REGISTER_ASYNC("File_read", FileRead);
  REGISTER_SYNC("File_readSync", FileReadSync);
  REGISTER_ASYNC("File_write", FileWrite);
  REGISTER_SYNC("File_writeSync", FileWriteSync);
  REGISTER_SYNC("Filesystem_fetchVirtualRoots", FilesystemFetchVirtualRoots);
  REGISTER_SYNC("FileSystemManager_addStorageStateChangeListener",
                StartListening);
  REGISTER_SYNC("FileSystemManager_removeStorageStateChangeListener",
                StopListening);
  REGISTER_SYNC("FileSystemManager_fetchStorages",
                FileSystemManagerFetchStorages);
  REGISTER_ASYNC("FileSystemManager_mkdir", FileSystemManagerMakeDirectory);
  REGISTER_SYNC("FileSystemManager_mkdirSync",
                FileSystemManagerMakeDirectorySync);
  REGISTER_ASYNC("File_unlinkFile", UnlinkFile);
  REGISTER_ASYNC("File_removeDirectory", RemoveDirectory);
  REGISTER_ASYNC("File_copyTo", CopyTo);
#undef REGISTER_SYNC
#undef REGISTER_ASYNC
  FilesystemManager::GetInstance().AddListener(this);
}

FilesystemInstance::~FilesystemInstance() {
  LoggerD("enter");
  FilesystemManager::GetInstance().StopListening();
  FilesystemManager::GetInstance().RemoveListener();
}

#define CHECK_EXIST(args, name, out)                                       \
  if (!args.contains(name)) {                                              \
    ReportError(TypeMismatchException(name " is required argument"), out); \
    return;                                                                \
  }

void FilesystemInstance::FileCreateSync(const picojson::value& args, picojson::object& out)
{
  LoggerD("enter");
  CHECK_EXIST(args, "location", out)

  const std::string& location = args.get("location").get<std::string>();

  auto onSuccess = [&](const FilesystemStat& data) {
    LoggerD("enter");
    ReportSuccess(data.toJSON(), out);
  };

  auto onError = [&](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().CreateFile(location, onSuccess, onError);
}

void FilesystemInstance::FileRename(const picojson::value& args,
                                    picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "oldPath", out)
  CHECK_EXIST(args, "newPath", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& oldPath = args.get("oldPath").get<std::string>();
  const std::string& newPath = args.get("newPath").get<std::string>();

  auto onSuccess = [this, callback_id](const FilesystemStat& data) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(data.toJSON(), obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fsm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::Rename, &fsm, oldPath, newPath, onSuccess, onError));
}

void FilesystemInstance::FileRead(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "location", out)
  CHECK_EXIST(args, "offset", out)
  CHECK_EXIST(args, "length", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& location = args.get("location").get<std::string>();
  size_t offset = static_cast<size_t>(args.get("offset").get<double>());
  size_t length = static_cast<size_t>(args.get("length").get<double>());

  auto onSuccess = [this, callback_id](const std::string& data) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(picojson::value(data), obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fsm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(&FilesystemManager::FileRead,
                                                   &fsm,
                                                   location,
                                                   offset,
                                                   length,
                                                   onSuccess,
                                                   onError));
}

void FilesystemInstance::FileReadSync(const picojson::value& args,
                                      picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "location", out)
  CHECK_EXIST(args, "offset", out)
  CHECK_EXIST(args, "length", out)

  const std::string& location = args.get("location").get<std::string>();
  size_t offset = static_cast<size_t>(args.get("offset").get<double>());
  size_t length = static_cast<size_t>(args.get("length").get<double>());

  auto onSuccess = [this, &out](const std::string& data) {
    LoggerD("enter");
    ReportSuccess(picojson::value(data), out);
  };

  auto onError = [this, &out](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().FileRead(
      location, offset, length, onSuccess, onError);
}

void FilesystemInstance::FileWrite(const picojson::value& args,
                                   picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "location", out)
  CHECK_EXIST(args, "data", out)
  CHECK_EXIST(args, "offset", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& location = args.get("location").get<std::string>();
  const std::string& data = args.get("data").get<std::string>();
  size_t offset = static_cast<size_t>(args.get("location").get<double>());

  auto onSuccess = [this, callback_id]() {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fsm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(
      std::bind(&FilesystemManager::FileWrite,
                &fsm,
                location,
                data,
                offset,
                onSuccess,
                onError));
}

void FilesystemInstance::FileWriteSync(const picojson::value& args,
                                       picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "location", out)
  CHECK_EXIST(args, "data", out)
  CHECK_EXIST(args, "offset", out)

  const std::string& location = args.get("location").get<std::string>();
  const std::string& data = args.get("data").get<std::string>();
  size_t offset = static_cast<size_t>(args.get("offset").get<double>());

  auto onSuccess = [this, &out]() {
    LoggerD("enter");
    ReportSuccess(out);
  };

  auto onError = [this, &out](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().FileWrite(
      location, data, offset, onSuccess, onError);
}

void FilesystemInstance::FileStat(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "location", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& location = args.get("location").get<std::string>();

  auto onSuccess = [this, callback_id](const FilesystemStat& data) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(data.toJSON(), obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fsm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::StatPath, &fsm, location, onSuccess, onError));
}

void FilesystemInstance::FileStatSync(const picojson::value& args,
                                      picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "location", out)

  const std::string& location = args.get("location").get<std::string>();

  auto onSuccess = [&](const FilesystemStat& data) {
    LoggerD("enter");
    ReportSuccess(data.toJSON(), out);
  };

  auto onError = [&](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().StatPath(location, onSuccess, onError);
}

void FilesystemInstance::FilesystemFetchVirtualRoots(
    const picojson::value& args, picojson::object& out) {
  LoggerD("enter");

  auto onSuccess = [&](const std::vector<common::VirtualRoot>& result) {
    LoggerD("enter");
    picojson::array roots;
    for (const auto& root : result) {
      roots.push_back(root.ToJson());
    }
    ReportSuccess(picojson::value(roots), out);
  };

  auto onError = [&](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().GetVirtualRoots(onSuccess, onError);
}

void FilesystemInstance::FileSystemManagerFetchStorages(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");

  auto onSuccess = [&](const std::vector<common::VirtualStorage>& result) {
    LoggerD("enter");
    picojson::array storages;
    storages.reserve(result.size());
    for (const auto& storage : result) {
      storages.push_back(storage.ToJson());
    }
    ReportSuccess(picojson::value(storages), out);
  };

  auto onError = [&](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().FetchStorages(onSuccess, onError);
}
void FilesystemInstance::StartListening(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");
  FilesystemManager::GetInstance().StartListening();
  ReportSuccess(out);
}

void FilesystemInstance::StopListening(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");
  FilesystemManager::GetInstance().StopListening();
  ReportSuccess(out);
}

void FilesystemInstance::onFilesystemStateChangeSuccessCallback(const common::VirtualStorage& storage) {
  LoggerD("entered");

  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj["label"] = picojson::value(storage.name_);
  obj["type"] = picojson::value(common::to_string(storage.type_));
  obj["state"] = picojson::value(common::to_string(storage.state_));
  obj["listenerId"] = picojson::value("StorageStateChangeListener");
  PostMessage(event.serialize().c_str());
}

void FilesystemInstance::onFilesystemStateChangeErrorCallback() {
  LoggerD("enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  ReportError(UnknownException(std::string("Failed to registerd listener")), obj);
  obj["listenerId"] = picojson::value("StorageStateChangeListener");
  LoggerD("Posting: %s", event.serialize().c_str());
  PostMessage(event.serialize().c_str());
}

void FilesystemInstance::FileSystemManagerMakeDirectory(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "location", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& location = args.get("location").get<std::string>();

  auto onResult = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    if (e == FilesystemError::DirectoryExists)
      ReportSuccess(obj);
    else
      PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  auto onAction = [location, onResult]() {
    FilesystemManager::GetInstance().MakeDirectory(location, onResult);
  };

  common::TaskQueue::GetInstance().Async(onAction);
}

void FilesystemInstance::FileSystemManagerMakeDirectorySync(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "location", out)

  const std::string& location = args.get("location").get<std::string>();

  auto onResult = [&](FilesystemError e) {
    LoggerD("enter");
    if (e == FilesystemError::DirectoryExists)
      ReportSuccess(out);
    else
      PrepareError(e, out);
  };

  FilesystemManager::GetInstance().MakeDirectory(location, onResult);
}

void FilesystemInstance::ReadDir(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "pathToDir", out)
  CHECK_EXIST(args, "callbackId", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& pathToDir = args.get("pathToDir").get<std::string>();

  auto onSuccess = [this, callback_id](const std::vector<std::string>& paths) {
    LoggerD("enter");
    picojson::value result = picojson::value(picojson::array());;
    picojson::array& statPaths = result.get<picojson::array>();
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    for(auto path : paths) {
      FilesystemStat stat = FilesystemStat::getStat(path);
      statPaths.push_back(stat.toJSON());
    }
    ReportSuccess(result, obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::ReadDir, &fm, pathToDir, onSuccess, onError));
}

void FilesystemInstance::UnlinkFile(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "pathToFile", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& pathToFile = args.get("pathToFile").get<std::string>();

  auto onSuccess = [this, callback_id]() {
    LoggerD("enter");
    picojson::value result = picojson::value();
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(result, obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::UnlinkFile, &fm, pathToFile, onSuccess, onError));
}

void FilesystemInstance::RemoveDirectory(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "pathToDelete", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& pathToDelete = args.get("pathToDelete").get<std::string>();

  auto onSuccess = [this, callback_id]() {
    LoggerD("enter");
    picojson::value result = picojson::value();
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(result, obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::RemoveDirectory, &fm, pathToDelete, onSuccess, onError));
}

void FilesystemInstance::CopyTo(const picojson::value& args,
                                  picojson::object& out) {
  LoggerD("enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "originFilePath", out)
  CHECK_EXIST(args, "destinationFilePath", out)
  CHECK_EXIST(args, "overwrite", out)

  double callback_id = args.get("callbackId").get<double>();
  const std::string& originPath = args.get("originFilePath").get<std::string>();
  const std::string& destinationPath = args.get("destinationFilePath").get<std::string>();
  const bool& overwrite = args.get("overwrite").get<bool>();

  auto onSuccess = [this, callback_id]() {
    LoggerD("enter");
    picojson::value result = picojson::value();
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    ReportSuccess(result, obj);
    PostMessage(response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    PostMessage(response.serialize().c_str());
  };

  FilesystemManager& fm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::CopyTo, &fm, originPath, destinationPath, overwrite, onSuccess, onError));
}

void FilesystemInstance::PrepareError(const FilesystemError& error, picojson::object& out)
{
  LoggerD("enter");
  switch (error) {
    case FilesystemError::None:
      ReportError(UnknownException("PLATFORM ERROR"), out);
      break;
    case FilesystemError::NotFound:
      ReportError(NotFoundException("PLATFORM ERROR"), out);
      break;
    case FilesystemError::FileExists:
      ReportError(IOException("File already exists"), out);
      break;
    case FilesystemError::DirectoryExists:
      ReportError(IOException("Directory already exists"), out);
      break;
    case FilesystemError::PermissionDenied:
      ReportError(IOException("Permission denied"), out);
      break;
    case FilesystemError::IOError:
      ReportError(IOException("IO Error"), out);
      break;
    case FilesystemError::Other:
      ReportError(UnknownException("PLATFORM ERROR"), out);
      break;
    case FilesystemError::InvalidValue:
      ReportError(InvalidValuesException("PLATFORM ERROR"), out);
      break;
    default:
      ReportError(UnknownException("PLATFORM ERROR"), out);
      break;
  }
}


#undef CHECK_EXIST

}  // namespace filesystem
}  // namespace extension