// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filesystem_storage.h"
#include "filesystem_utils.h"

namespace extension {
namespace filesystem {

namespace {

StoragePaths fetch_paths(int id) {
  StoragePaths paths;
  paths.images =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_IMAGES);
  paths.sounds =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_SOUNDS);
  paths.videos =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_VIDEOS);
  paths.camera =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_CAMERA);
  paths.downloads =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_DOWNLOADS);
  paths.music =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_MUSIC);
  paths.documents =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_DOCUMENTS);
  paths.others =
      FilesystemUtils::get_storage_dir_path(id, STORAGE_DIRECTORY_OTHERS);
  paths.ringtones = FilesystemUtils::get_storage_dir_path(
      id, STORAGE_DIRECTORY_SYSTEM_RINGTONES);
  return paths;
}

picojson::value paths_to_json(const StoragePaths& storagePaths) {
  picojson::object result;
  if (!storagePaths.images.empty())
    result["images"] = picojson::value(storagePaths.images);
  if (!storagePaths.sounds.empty())
    result["sounds"] = picojson::value(storagePaths.sounds);
  if (!storagePaths.videos.empty())
    result["videos"] = picojson::value(storagePaths.videos);
  if (!storagePaths.camera.empty())
    result["camera"] = picojson::value(storagePaths.camera);
  if (!storagePaths.downloads.empty())
    result["downloads"] = picojson::value(storagePaths.downloads);
  if (!storagePaths.music.empty())
    result["music"] = picojson::value(storagePaths.music);
  if (!storagePaths.documents.empty())
    result["documents"] = picojson::value(storagePaths.documents);
  if (!storagePaths.others.empty())
    result["others"] = picojson::value(storagePaths.others);
  if (!storagePaths.ringtones.empty())
    result["ringtones"] = picojson::value(storagePaths.ringtones);

  return picojson::value(result);
}
}

FilesystemStorage::FilesystemStorage(int storage_id_,
                                     storage_type_e type_,
                                     storage_state_e state_,
                                     const char* path_)
    : storage_id(storage_id_),
      type(std::to_string(type_)),
      state(std::to_string(state_)),
      path(std::string(path_)),
      name(std::to_string(type_) + std::to_string(storage_id_)),
      paths(fetch_paths(storage_id)) {}

picojson::value FilesystemStorage::toJSON() const {
  picojson::value retval = picojson::value(picojson::object());
  picojson::object& obj = retval.get<picojson::object>();

  obj["storage_id"] = picojson::value(static_cast<double>(storage_id));
  obj["type"] = picojson::value(type);
  obj["state"] = picojson::value(state);
  obj["path"] = picojson::value(path);
  obj["name"] = picojson::value(name);
  obj["paths"] = paths_to_json(paths);
  return retval;
}
}
}
