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

#include <glib.h>
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
  REGISTER_SYNC("File_readSync", FileReadSync);
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
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    Instance::PostMessage(this, response.serialize().c_str());
  };

  FilesystemManager& fsm = FilesystemManager::GetInstance();
  common::TaskQueue::GetInstance().Async(std::bind(
      &FilesystemManager::Rename, &fsm, oldPath, newPath, onSuccess, onError));
}

void FilesystemInstance::FileReadSync(const common::ParsedDataRequest& req,
                                      common::ParsedDataResponse& res) {
  LoggerD("enter");
  const picojson::value& args = req.args();
  picojson::object& out = res.object();
  CHECK_EXIST(args, "location", out)
  CHECK_EXIST(args, "offset", out)
  CHECK_EXIST(args, "length", out)

  const std::string& location = args.get("location").get<std::string>();
  size_t offset = static_cast<size_t>(args.get("offset").get<double>());
  size_t length = static_cast<size_t>(args.get("length").get<double>());
  const bool is_base64 = args.get("is_base64").get<bool>();

  auto onSuccess = [this, &out, &length, &res, is_base64](const std::string& data, uint8_t* data_p, size_t readed) {
    LoggerD("enter");
    if (data_p) {
      if (is_base64) {
        gchar* encoded = g_base64_encode(data_p, length);
        free(data_p);
        // encoded will be freeed by runtime.
        res.SetBuffer(reinterpret_cast<uint8_t*>(encoded), strlen(encoded) + 1);
      } else {
        res.SetBuffer(data_p, length);
      }
      out["data_size"] = picojson::value(static_cast<double>(readed));
      ReportSuccess(out);
    }
  };

  auto onError = [this, &out](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };

  FilesystemManager::GetInstance().FileRead(
      location, offset, length, onSuccess, onError);
}

void FilesystemInstance::FileWriteSync(const common::ParsedDataRequest& req, common::ParsedDataResponse& res) {
  LoggerD("enter");
  const picojson::value& args = req.args();
  picojson::object& out = res.object();
  CHECK_EXIST(args, "location", out)
  CHECK_EXIST(args, "offset", out)

  const std::string& location = args.get("location").get<std::string>();
  const bool is_base64 = args.get("is_base64").get<bool>();
  size_t offset = static_cast<size_t>(args.get("offset").get<double>());

  auto onSuccess = [this, &out](size_t written) {
    LoggerD("enter");
    out["data_size"] = picojson::value(static_cast<double>(written));
    ReportSuccess(out);
  };

  auto onError = [this, &out](FilesystemError e) {
    LoggerD("enter");
    PrepareError(e, out);
  };
  uint8_t* data_p = nullptr;
  size_t data_size = 0;
  if (is_base64) {
    data_p = g_base64_decode(reinterpret_cast<char*>(req.buffer()),
                             &data_size);
  } else {
    data_p = req.buffer();
    data_size = req.buffer_length();
  }
  if (data_size > 0 && data_p) {
    FilesystemManager::GetInstance().FileWrite(
        location, data_p, data_size, offset, onSuccess, onError);
  }
  if (is_base64 && data_p) {
    free(data_p);
  }
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
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    Instance::PostMessage(this, response.serialize().c_str());
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
  Instance::PostMessage(this, event.serialize().c_str());
}

void FilesystemInstance::onFilesystemStateChangeErrorCallback() {
  LoggerD("enter");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  ReportError(UnknownException(std::string("Failed to registerd listener")), obj);
  obj["listenerId"] = picojson::value("StorageStateChangeListener");
  LoggerD("Posting: %s", event.serialize().c_str());
  Instance::PostMessage(this, event.serialize().c_str());
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
    Instance::PostMessage(this, response.serialize().c_str());
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
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    Instance::PostMessage(this, response.serialize().c_str());
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
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    Instance::PostMessage(this, response.serialize().c_str());
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
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    Instance::PostMessage(this, response.serialize().c_str());
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
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto onError = [this, callback_id](FilesystemError e) {
    LoggerD("enter");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& obj = response.get<picojson::object>();
    obj["callbackId"] = picojson::value(callback_id);
    PrepareError(e, obj);
    Instance::PostMessage(this, response.serialize().c_str());
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
      LoggerE("UnknownException - PLATFORM ERROR");
      ReportError(UnknownException("PLATFORM ERROR"), out);
      break;
    case FilesystemError::NotFound:
      LoggerE("NotFoundException - PLATFORM ERROR");
      ReportError(NotFoundException("PLATFORM ERROR"), out);
      break;
    case FilesystemError::FileExists:
      LoggerE("IOException - File already exists");
      ReportError(IOException("File already exists"), out);
      break;
    case FilesystemError::DirectoryExists:
      LoggerE("IOException - Directory already exists");
      ReportError(IOException("Directory already exists"), out);
      break;
    case FilesystemError::PermissionDenied:
      LoggerE("IOException - Permission denied");
      ReportError(IOException("Permission denied"), out);
      break;
    case FilesystemError::IOError:
      LoggerE("IOException - IO Error");
      ReportError(IOException("IO Error"), out);
      break;
    case FilesystemError::Other:
      LoggerE("UnknownException - PLATFORM ERROR other");
      ReportError(UnknownException("PLATFORM ERROR other"), out);
      break;
    case FilesystemError::InvalidValue:
      LoggerE("InvalidValuesException - PLATFORM ERROR");
      ReportError(InvalidValuesException("PLATFORM ERROR"), out);
      break;
    default:
      LoggerE("UnknownException - PLATFORM ERROR default");
      ReportError(UnknownException("PLATFORM ERROR default"), out);
      break;
  }
}


#undef CHECK_EXIST

}  // namespace filesystem
}  // namespace extension
