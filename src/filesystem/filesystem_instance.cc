// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  using namespace std::placeholders;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&FilesystemInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&FilesystemInstance::x, this, _1, _2));
  REGISTER_ASYNC("File_stat", FileStat);
  REGISTER_SYNC("File_statSync", FileStatSync);
  REGISTER_SYNC("File_createSync", FileCreateSync);
  REGISTER_ASYNC("File_readDir", ReadDir);
  REGISTER_ASYNC("File_rename", FileRename);
  REGISTER_SYNC("Filesystem_getWidgetPaths", FilesystemGetWidgetPaths);
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
#undef REGISTER_SYNC
#undef REGISTER_ASYNC
  FilesystemManager::GetInstance().AddListener(this);
}

FilesystemInstance::~FilesystemInstance() {}

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

void FilesystemInstance::FilesystemGetWidgetPaths(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("enter");

  auto onSuccess = [&](const std::map<std::string, std::string>& result) {
    LoggerD("enter");
    picojson::object paths;
    for (const auto& entry : result) {
      paths[entry.first] = picojson::value(entry.second);
    }
    ReportSuccess(picojson::value(paths), out);
  };

  auto onError = [&](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().GetWidgetPaths(onSuccess, onError);
}

void FilesystemInstance::FileSystemManagerFetchStorages(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("enter");

  auto onSuccess = [&](const std::vector<FilesystemStorage>& result) {
    LoggerD("enter");
    picojson::array storages;
    storages.reserve(result.size());
    for (const FilesystemStorage& storage : result) {
      storages.push_back(storage.toJSON());
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
  FilesystemManager::GetInstance().StartListening();
  ReportSuccess(out);
}

void FilesystemInstance::StopListening(
    const picojson::value& args,
    picojson::object& out) {
  FilesystemManager::GetInstance().StopListening();
  ReportSuccess(out);
}

void FilesystemInstance::onFilesystemStateChangeSuccessCallback(const std::string& label, const std::string& state, const std::string& type) {
  LoggerD("entered");

  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj["label"] = picojson::value(label);
  obj["type"] = picojson::value(type);
  obj["state"] = picojson::value(state);
  obj["listenerId"] = picojson::value("StorageStateChangeListener");
  PostMessage(event.serialize().c_str());
}

void FilesystemInstance::onFilesystemStateChangeErrorCallback() {
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
    case FilesystemError::Other:
      ReportError(UnknownException("PLATFORM ERROR"), out);
      break;
    default:
      ReportError(UnknownException("PLATFORM ERROR"), out);
      break;
  }
}


#undef CHECK_EXIST

}  // namespace filesystem
}  // namespace extension
