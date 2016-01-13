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

#include "filesystem/filesystem_manager.h"

#include <app_manager.h>
#include <package_manager.h>
#include <storage-expand.h>
#include <storage.h>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#include <ftw.h>
#undef _XOPEN_SOURCE

#include "common/logger.h"
#include "common/tools.h"
#include "common/scope_exit.h"
#include "common/extension.h"
#include "filesystem/filesystem_file.h"

namespace extension {
namespace filesystem {

using common::tools::GetErrorString;

namespace {

int unlink_cb(const char* fpath, const struct stat* sb, int typeflag,
              struct FTW* ftwbuf) {
  if (ftwbuf->level == 0)
    return 0;

  int result = remove(fpath);
  if (result)
    LoggerE("error occured");
  return result;
}

int unlink_with_base_dir_cb(const char* fpath, const struct stat* sb,
                            int typeflag, struct FTW* ftwbuf) {
  int result = remove(fpath);
  if (result)
    LoggerE("error occured");
  return result;
}

FilesystemError copyFile(const std::string& originPath,
                         const std::string& destPath) {
  LoggerD("enter src %s dst %s", originPath.c_str(), destPath.c_str());

  std::ifstream src(originPath, std::ios::in | std::ios::binary);
  std::ofstream dst(destPath, std::ios::out | std::ios::binary);

  std::istreambuf_iterator<char> begin_source(src);
  std::istreambuf_iterator<char> end_source;
  std::ostreambuf_iterator<char> begin_dest(dst);
  std::copy(begin_source, end_source, begin_dest);

  if (src.fail() || dst.fail()) {
    LoggerE("Cannot copy file");
    return FilesystemError::IOError;
  }
  return FilesystemError::None;
}

FilesystemError copyDirectory(const std::string& originPath,
                              const std::string& destPath) {
  LoggerD("enter src %s dst %s", originPath.c_str(), destPath.c_str());
  FilesystemStat destStat = FilesystemStat::getStat(destPath);

  int status;
  if (!destStat.valid) {
    const mode_t create_mode = S_IRWXU | S_IRWXG | S_IRWXO;
    status = mkdir(destPath.c_str(), create_mode);
    if (status) {
      LoggerE("Cannot create directory: %s", GetErrorString(errno).c_str());
      return FilesystemError::Other;
    }
  }
  DIR* dp = opendir(originPath.c_str());
  if (dp == NULL) {
    LoggerE("Cannot open directory: %s", GetErrorString(errno).c_str());
    return FilesystemError::Other;
  }
  SCOPE_EXIT {
    (void)closedir(dp);
  };
  struct dirent entry;
  struct dirent* result = nullptr;
  while (0 == (status = readdir_r(dp, &entry, &result)) && result != nullptr) {
    if (strcmp(result->d_name, ".") == 0 || strcmp(result->d_name, "..") == 0)
      continue;

    std::string oldLocation = originPath + std::string("/")
        + std::string(result->d_name);
    std::string newLocation = destPath + std::string("/")
        + std::string(result->d_name);
    FilesystemError fstatus = FilesystemError::None;
    if (result->d_type == DT_DIR) {
      fstatus = copyDirectory(oldLocation, newLocation);
    } else if (result->d_type == DT_REG) {
      fstatus = copyFile(oldLocation, newLocation);
    }
    if (fstatus != FilesystemError::None) {
      LoggerE("Error while copying tree");
      return fstatus;
    }
  }
  if (status != 0) {
    LoggerE("error occured");
    return FilesystemError::Other;
  }
  return FilesystemError::None;
}

FilesystemError perform_deep_copy(const std::string& originPath,
                                  const std::string& destPath, bool overwrite) {
  LoggerD("enter src %s dst %s", originPath.c_str(), destPath.c_str());
  FilesystemStat originStat = FilesystemStat::getStat(originPath);
  FilesystemStat destStat = FilesystemStat::getStat(destPath);
  int status;
  std::string path = destPath;
  if (!originStat.valid) {
    LoggerE("Cannot retrieve stat in deep copy");
    return FilesystemError::Other;
  }

  // Overwrite check
  if (!overwrite && destStat.valid) {
    // Do not overwrite file.
    LoggerD("Cannot overwrite existing entity");
    return FilesystemError::FileExists;
  }

  // Remove existing data.
  if (destStat.valid) {
    if (destStat.isDirectory) {
      path.append("/");
      if (originStat.isFile) {
        std::string dstPathWithFilename = originPath.substr(
            originPath.find_last_of("/") + 1);
        path.append(dstPathWithFilename);
        FilesystemStat destStatWithFilename = FilesystemStat::getStat(path);
        if (destStatWithFilename.valid) {
          status = remove(path.c_str());
          if (status) {
            LoggerE("Cannot remove old file: %s",
                    GetErrorString(errno).c_str());
            return FilesystemError::Other;
          }
        }
      } else {
        const int maxDirOpened = 64;
        if (nftw(path.c_str(), unlink_cb, maxDirOpened, FTW_DEPTH | FTW_PHYS)
            != 0) {
          LoggerE("Error occured");
          return FilesystemError::Other;
        }
      }
    } else {
      status = remove(path.c_str());
      if (status) {
        LoggerE("Cannot remove old directory: %s",
                GetErrorString(errno).c_str());
        return FilesystemError::Other;
      }
    }
  }

  if (originStat.isFile) {
    return copyFile(originPath, path);
  } else if (originStat.isDirectory) {
    return copyDirectory(originPath, destPath);
  }
  return FilesystemError::None;
}

FilesystemError make_directory_worker(const std::string& path) {
  LoggerD("enter: %s", path.c_str());
  auto fsstat = FilesystemStat::getStat(path);
  if (fsstat.valid) {
    if (fsstat.isDirectory) {
      LoggerD("Directory exists");
      return FilesystemError::DirectoryExists;
    } else {
      LoggerD("It is a file and exists");
      return FilesystemError::FileExists;
    }
  }

  std::string parent_path = FilesystemUtils::get_dirname(path);
  auto parent_result = make_directory_worker(parent_path);

  if (parent_result == FilesystemError::DirectoryExists) {
    LoggerD("Creating directrory: %s", path.c_str());
    mode_t create_mode = S_IRWXU | S_IRWXG | S_IRWXO;
    int r = mkdir(path.c_str(), create_mode);
    if (r == 0) {
      return FilesystemError::DirectoryExists;
    }
    LoggerD("Cannot create directory: %s", GetErrorString(errno).c_str());
    return FilesystemError::Other;
  }
  return parent_result;
}
}  // namespace

void FilesystemManager::FetchStorages(
    const std::function<void(const common::Storages&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("Entered");

  success_cb(fs_provider_.GetStorages());
}

FilesystemManager::FilesystemManager()
    : listener_(nullptr),
      fs_provider_(common::FilesystemProvider::Create()) {
  LoggerD("enter");
}

FilesystemManager::~FilesystemManager() {
  LoggerD("enter");
}

FilesystemManager& FilesystemManager::GetInstance() {
  LoggerD("enter");
  static FilesystemManager instance;
  return instance;
}

void FilesystemManager::StatPath(
    const std::string& path,
    const std::function<void(const FilesystemStat&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {

  LoggerD("enter");
  FilesystemStat statData = FilesystemStat::getStat(path);
  if (!statData.valid) {
    error_cb(statData.error);
    return;
  }

  success_cb(statData);
}

void FilesystemManager::GetVirtualRoots(
    const std::function<void(const common::VirtualRoots&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("enter");
  success_cb(fs_provider_.GetVirtualPaths());
}

void FilesystemManager::CreateFile(
    const std::string& path,
    const std::function<void(const FilesystemStat&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("enter");
  const mode_t create_mode = S_IRWXU | S_IRWXG | S_IRWXO;
  int status;
  status = TEMP_FAILURE_RETRY(
      open(path.c_str(), O_RDWR | O_CREAT, create_mode));
  if (-1 == status) {
    LoggerE("Cannot create or open file %s: %s", path.c_str(),
            GetErrorString(errno).c_str());
    error_cb(FilesystemError::Other);
    return;
  }
  status = close(status);
  if (0 != status) {
    LoggerE("Cannot close file %s: %s", path.c_str(),
            GetErrorString(errno).c_str());
    error_cb(FilesystemError::Other);
    return;
  }
  FilesystemStat stat = FilesystemStat::getStat(path);
  if (stat.valid) {
    success_cb(stat);
  } else {
    LoggerE("Cannot create stat data!");
    error_cb(FilesystemError::Other);
  }
}

void FilesystemManager::MakeDirectory(
    const std::string& path,
    const std::function<void(FilesystemError)>& result_cb) {
  LoggerD("enter");
  result_cb(make_directory_worker(path));
}

void FilesystemManager::Rename(
    const std::string& oldPath, const std::string& newPath,
    const std::function<void(const FilesystemStat&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {

  LoggerD("enter");
  int status = rename(oldPath.c_str(), newPath.c_str());
  if (0 == status) {
    FilesystemStat fileStat = FilesystemStat::getStat(newPath);
    if (fileStat.valid) {
      success_cb(FilesystemStat::getStat(newPath));
    } else {
      LoggerE("Cannot perform stat on new path!");
      error_cb(FilesystemError::Other);
    }
  } else {
    LoggerE("Cannot rename file: %s", GetErrorString(errno).c_str());
    error_cb(FilesystemError::Other);
  }
}

void FilesystemManager::ReadDir(
    const std::string& path,
    const std::function<void(const std::vector<std::string>&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("entered");

  std::vector < std::string > fileList;
  DIR* dp = nullptr;
  struct dirent entry;
  struct dirent* result = nullptr;
  int status = 0;

  dp = opendir(path.c_str());
  if (dp != NULL) {
    while ((status = readdir_r(dp, &entry, &result)) == 0 && result != nullptr) {
      if (strcmp(result->d_name, ".") != 0 && strcmp(result->d_name, "..") != 0)
        fileList.push_back(path + "/" + std::string(result->d_name));
    }
    (void) closedir(dp);
    if (status == 0) {
      success_cb(fileList);
    } else {
      LoggerE("error occured");
      error_cb(FilesystemError::Other);
    }
  } else {
    LoggerE("Couldn't open the directory");
    error_cb(FilesystemError::Other);
    return;
  }
}

void FilesystemManager::UnlinkFile(
    const std::string& path, const std::function<void()>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("enter");
  if (unlink(path.c_str()) != 0) {
    LoggerE("Error occured while deleting file");
    error_cb(FilesystemError::Other);
    return;
  }
  success_cb();
}

void FilesystemManager::RemoveDirectory(
    const std::string& path, const std::function<void()>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("enter");
  const int maxDirOpened = 64;
  if (nftw(path.c_str(), unlink_with_base_dir_cb, maxDirOpened,
           FTW_DEPTH | FTW_PHYS) != 0) {
    LoggerE("Error occured");
    error_cb(FilesystemError::Other);
  }
  success_cb();
  return;
}

void FilesystemManager::FileRead(
    const std::string& path,
    size_t offset,
    size_t length,
    const std::function<void(const std::string&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {

  LoggerD("enter");
  FilesystemFile file(path);
  FilesystemBuffer buffer;
  if (!file.Read(&buffer, offset, length)) {
    LoggerE("Cannot read file %s", path.c_str());
    error_cb(FilesystemError::Other);
    return;
  }

  std::string out_data = buffer.EncodeData();
  success_cb(out_data);
}

void FilesystemManager::FileWrite(
    const std::string& path,
    const std::string& data,
    size_t offset,
    const std::function<void()>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {

  LoggerD("enter");
  FilesystemFile file(path);
  FilesystemBuffer buffer;
  // Decode buffer data
  if (!buffer.DecodeData(data)) {
    LoggerE("Cannot decode file data!");
    error_cb(FilesystemError::Other);
    return;
  }

  if (file.Write(buffer, offset)) {
    success_cb();
  } else {
    LoggerE("Cannot write to file %s!", path.c_str());
    error_cb(FilesystemError::Other);
  }
}

void FilesystemManager::StartListening() {
  LoggerD("Entered");
  auto set = std::bind(&FilesystemManager::OnStorageDeviceChanged, this,
                       std::placeholders::_1, std::placeholders::_2,
                       std::placeholders::_3);

  fs_provider_.RegisterDeviceChangeState(set);
}

void FilesystemManager::StopListening() {
  LoggerD("Entered");

  fs_provider_.UnregisterDeviceChangeState();
}

void FilesystemManager::OnStorageDeviceChanged(common::Storage const& _storage,
                                               common::StorageState _old,
                                               common::StorageState _new) {
  LoggerD("Entered");
  if (listener_) {
    listener_->onFilesystemStateChangeSuccessCallback(_storage);
  }
}

void FilesystemManager::CopyTo(
    const std::string& originFilePath, const std::string& destinationFilePath,
    const bool overwrite, const std::function<void()>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("enter");
  FilesystemError retval = perform_deep_copy(originFilePath,
                                             destinationFilePath, overwrite);
  if (FilesystemError::None == retval) {
    success_cb();
  } else {
    LoggerE("Failed: perform_deep_copy()");
    error_cb(retval);
  }
}

void FilesystemManager::AddListener(FilesystemStateChangeListener* listener) {
  LoggerD("enter");
  listener_ = listener;
}

void FilesystemManager::RemoveListener() {
  LoggerD("enter");
  listener_ = NULL;
}
}  // namespace filesystem
}  // namespace extension
