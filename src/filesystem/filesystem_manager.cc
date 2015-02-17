// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem_manager.h"

#include <app_manager.h>
#include <package_manager.h>
#include <storage-expand.h>
#include <storage.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#define _XOPEN_SOURCE 500
#include <ftw.h>
#undef _XOPEN_SOURCE

#include "common/logger.h"
#include "common/scope_exit.h"
#include "common/extension.h"
#include "filesystem_file.h"

namespace extension {
namespace filesystem {

namespace {
void storage_cb(int storage_id, storage_state_e state, void* user_data) {
  LoggerD("entered");
  if (user_data) {
    FilesystemStateChangeListener* listener =
        static_cast<FilesystemStateChangeListener*>(user_data);
    storage_type_e type;
    storage_get_type(storage_id, &type);
    listener->onFilesystemStateChangeSuccessCallback(
        std::to_string(type) + std::to_string(storage_id),
        std::to_string(state),
        std::to_string(type));
  }
}

int unlink_cb(const char* fpath,
              const struct stat* sb,
              int typeflag,
              struct FTW* ftwbuf) {
  int result = remove(fpath);
  if (result)
    LoggerE("error occured");
  return result;
}

bool fetch_storages_cb(int storage_id,
                       storage_type_e type,
                       storage_state_e state,
                       const char* path,
                       void* user_data) {
  LoggerD("enter");
  if (!user_data) {
    LoggerE("Invalid user_data pointer!");
    return false;
  }

  std::vector<FilesystemStorage>* result =
      static_cast<std::vector<FilesystemStorage>*>(user_data);
  result->push_back(FilesystemStorage(storage_id, type, state, path));
  return true;
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
    LoggerD("Cannot create directory: %s", strerror(errno));
    return FilesystemError::Other;
  }
  return parent_result;
}
}  // namespace

void FilesystemManager::FetchStorages(
    const std::function<void(const std::vector<FilesystemStorage>&)>&
        success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  std::vector<FilesystemStorage> result;
  if (STORAGE_ERROR_NONE !=
      storage_foreach_device_supported(fetch_storages_cb, &result))
    error_cb(FilesystemError::Other);
  for (auto storage : result) {
    ids_.insert(storage.storage_id);
  }
  success_cb(result);
}

FilesystemManager::FilesystemManager()
    : listener_(nullptr), is_listener_registered_(false) {}
FilesystemManager::~FilesystemManager() {
  if (is_listener_registered_) {
    for (auto id : ids_) {
      storage_unset_state_changed_cb(id, storage_cb);
    }
  }
}

FilesystemManager& FilesystemManager::GetInstance() {
  static FilesystemManager instance;
  return instance;
}

void FilesystemManager::StatPath(
    const std::string& path,
    const std::function<void(const FilesystemStat&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {

  FilesystemStat statData = FilesystemStat::getStat(path);
  if (!statData.valid) {
    error_cb(FilesystemError::Other);
    return;
  }

  success_cb(statData);
}

static std::string getAppRoot() {
  std::string appId = common::Extension::GetRuntimeVariable("app_id", 64);
  app_info_h app_info;
  int err = app_info_create(appId.c_str(), &app_info);
  if (err != APP_MANAGER_ERROR_NONE) {
    LoggerE("Can't create app info handle from appId (%s)", appId.c_str());
    return "";
  }
  SCOPE_EXIT {
    app_info_destroy(app_info);
  };
  char* package = NULL;
  err = app_info_get_package(app_info, &package);
  if (err != APP_MANAGER_ERROR_NONE) {
    LoggerE("Can't get package name from app info (%s)",
            get_error_message(err));
    return "";
  }
  SCOPE_EXIT {
    if (package != NULL)
      free(package);
  };

  package_info_h pkg_info;
  err = package_info_create(package, &pkg_info);
  if (err != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Can't create package info handle from pkg (%s)",
            get_error_message(err));
    return "";
  }
  SCOPE_EXIT {
    package_info_destroy(pkg_info);
  };
  char* root = NULL;
  package_info_get_root_path(pkg_info, &root);
  if (err != PACKAGE_MANAGER_ERROR_NONE) {
    LoggerE("Can't get root path from package info (%s)",
            get_error_message(err));
    return "";
  }
  std::string app_root_dir(root);
  free(root);

  return app_root_dir;
}

void FilesystemManager::GetWidgetPaths(
    const std::function<void(const std::map<std::string, std::string>&)>&
        success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("enter");
  std::map<std::string, std::string> result;
  const std::string app_root = getAppRoot();
  LoggerD("App root is %s", app_root.c_str());
  if (app_root.empty()) {
    error_cb(FilesystemError::Other);
    return;
  }

  result["wgt-package"] = app_root + "/res/wgt";
  result["wgt-private"] = app_root + "/data";
  result["wgt-private-tmp"] = app_root + "/tmp";
  success_cb(result);
}

void FilesystemManager::CreateFile(
    const std::string& path,
    const std::function<void(const FilesystemStat&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  const mode_t create_mode = S_IRWXU | S_IRWXG | S_IRWXO;
  int status;
  status =
      TEMP_FAILURE_RETRY(open(path.c_str(), O_RDWR | O_CREAT, create_mode));
  if (-1 == status) {
    LoggerE("Cannot create or open file %s: %s", path.c_str(), strerror(errno));
    error_cb(FilesystemError::Other);
  }
  status = close(status);
  if (0 != status) {
    LoggerE("Cannot close file %s: %s", path.c_str(), strerror(errno));
    error_cb(FilesystemError::Other);
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
    const std::string& oldPath,
    const std::string& newPath,
    const std::function<void(const FilesystemStat&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {

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
    LoggerE("Cannot rename file: %s", strerror(errno));
    error_cb(FilesystemError::Other);
  }
}

void FilesystemManager::ReadDir(
    const std::string& path,
    const std::function<void(const std::vector<std::string>&)>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  LoggerD("entered");

  std::vector<std::string> fileList;
  DIR* dp = nullptr;
  struct dirent entry;
  struct dirent* result = nullptr;
  int status = 0;

  dp = opendir(path.c_str());
  if (dp != NULL) {
    while ((status = readdir_r(dp, &entry, &result)) == 0 &&
           result != nullptr) {
      if (strcmp(result->d_name, ".") != 0 && strcmp(result->d_name, "..") != 0)
        fileList.push_back(path + "/" + std::string(result->d_name));
    }
    (void)closedir(dp);
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
    const std::string& path,
    const std::function<void()>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  if (unlink(path.c_str()) != 0) {
    LoggerE("Error occured while deleting file");
    error_cb(FilesystemError::Other);
    return;
  }
  success_cb();
}

void FilesystemManager::RemoveDirectory(
    const std::string& path,
    const std::function<void()>& success_cb,
    const std::function<void(FilesystemError)>& error_cb) {
  const int maxDirOpened = 64;
  if (nftw(path.c_str(), unlink_cb, maxDirOpened, FTW_DEPTH | FTW_PHYS) != 0) {
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
  LoggerD("enter");

  if (!is_listener_registered_ && !ids_.empty()) {
    int result = STORAGE_ERROR_NONE;
    std::set<int> registeredSuccessfully;
    for (auto id : ids_) {
      result = storage_set_state_changed_cb(id, storage_cb, (void*)listener_);
      LoggerD("registered id %d", id);
      if (result != STORAGE_ERROR_NONE) {
        for (auto registeredId : registeredSuccessfully) {
          storage_unset_state_changed_cb(registeredId, storage_cb);
          LoggerD("unregistering id %d", registeredId);
        }
        listener_->onFilesystemStateChangeErrorCallback();
        break;
      } else {
        registeredSuccessfully.insert(id);
      }
    }
    if (ids_.size() == registeredSuccessfully.size())
      is_listener_registered_ = true;
  }
}

void FilesystemManager::StopListening() {
  LoggerD("enter");
  if (is_listener_registered_) {
    for (auto id : ids_) {
      storage_unset_state_changed_cb(id, storage_cb);
    }
  }
  is_listener_registered_ = false;
}

void FilesystemManager::AddListener(FilesystemStateChangeListener* listener) {
  LoggerD("enter");
  listener_ = listener;
}
}  // namespace filesystem
}  // namespace extension
