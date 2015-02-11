// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem/filesystem_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace filesystem {

namespace {
// The privileges that required in Filesystem API
const std::string kPrivilegeFilesystem = "";

} // namespace

using namespace common;
using namespace extension::filesystem;

FilesystemInstance::FilesystemInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&FilesystemInstance::x, this, _1, _2));
  REGISTER_SYNC("File_moveTo", FileMoveTo);
  REGISTER_SYNC("FileSystemManager_listStorages", FileSystemManagerListStorages);
  REGISTER_SYNC("FileSystemManager_resolve", FileSystemManagerResolve);
  REGISTER_SYNC("FileSystemManager_addStorageStateChangeListener", FileSystemManagerAddStorageStateChangeListener);
  REGISTER_SYNC("FileSystemManager_removeStorageStateChangeListener", FileSystemManagerRemoveStorageStateChangeListener);
  REGISTER_SYNC("File_toURI", FileToURI);
  REGISTER_SYNC("File_resolve", FileResolve);
  REGISTER_SYNC("File_listFiles", FileListFiles);
  REGISTER_SYNC("File_deleteDirectory", FileDeleteDirectory);
  REGISTER_SYNC("File_openStream", FileOpenStream);
  REGISTER_SYNC("File_createDirectory", FileCreateDirectory);
  REGISTER_SYNC("File_createFile", FileCreateFile);
  REGISTER_SYNC("File_deleteFile", FileDeleteFile);
  REGISTER_SYNC("File_readAsText", FileReadAsText);
  REGISTER_SYNC("File_copyTo", FileCopyTo);
  REGISTER_SYNC("FileSystemManager_getStorage", FileSystemManagerGetStorage);
  #undef REGISTER_SYNC
}

FilesystemInstance::~FilesystemInstance() {
}


enum FilesystemCallbacks {
  FileMoveToCallback,
  FileSystemManagerListStoragesCallback,
  FileSystemManagerResolveCallback,
  FileSystemManagerAddStorageStateChangeListenerCallback,
  FileSystemManagerRemoveStorageStateChangeListenerCallback,
  FileToURICallback,
  FileResolveCallback,
  FileListFilesCallback,
  FileDeleteDirectoryCallback,
  FileOpenStreamCallback,
  FileCreateDirectoryCallback,
  FileCreateFileCallback,
  FileDeleteFileCallback,
  FileReadAsTextCallback,
  FileCopyToCallback,
  FileSystemManagerGetStorageCallback
};

static void ReplyAsync(FilesystemInstance* instance, FilesystemCallbacks cbfunc,
                       int callbackId, bool isSuccess, picojson::object& param) {
  param["callbackId"] = picojson::value(static_cast<double>(callbackId));
  param["status"] = picojson::value(isSuccess ? "success" : "error");

  // insert result for async callback to param

  picojson::value result = picojson::value(param);

  instance->PostMessage(result.serialize().c_str());
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }


void FilesystemInstance::FileSystemManagerResolve(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "location", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& location = args.get("location").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileSystemManagerGetStorage(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "label", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& label = args.get("label").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileSystemManagerListStorages(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileSystemManagerAddStorageStateChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileSystemManagerRemoveStorageStateChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)

  double watchId = args.get("watchId").get<double>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileToURI(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileListFiles(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileOpenStream(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& encoding = args.get("encoding").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileReadAsText(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& encoding = args.get("encoding").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileCopyTo(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "originFilePath", out)
  CHECK_EXIST(args, "destinationFilePath", out)
  CHECK_EXIST(args, "overwrite", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& originFilePath = args.get("originFilePath").get<std::string>();
  const std::string& destinationFilePath = args.get("destinationFilePath").get<std::string>();
  bool overwrite = args.get("overwrite").get<bool>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileMoveTo(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "originFilePath", out)
  CHECK_EXIST(args, "destinationFilePath", out)
  CHECK_EXIST(args, "overwrite", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& originFilePath = args.get("originFilePath").get<std::string>();
  const std::string& destinationFilePath = args.get("destinationFilePath").get<std::string>();
  bool overwrite = args.get("overwrite").get<bool>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileCreateDirectory(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "dirPath", out)

  const std::string& dirPath = args.get("dirPath").get<std::string>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileCreateFile(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "relativeFilePath", out)

  const std::string& relativeFilePath = args.get("relativeFilePath").get<std::string>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileResolve(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "filePath", out)

  const std::string& filePath = args.get("filePath").get<std::string>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileDeleteDirectory(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "directoryPath", out)
  CHECK_EXIST(args, "recursive", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& directoryPath = args.get("directoryPath").get<std::string>();
  bool recursive = args.get("recursive").get<bool>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void FilesystemInstance::FileDeleteFile(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "filePath", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& filePath = args.get("filePath").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}


#undef CHECK_EXIST

} // namespace filesystem
} // namespace extension
