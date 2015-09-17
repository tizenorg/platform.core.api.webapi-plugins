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

#include "content/content_instance.h"

#include <functional>
#include <string>
#include <dlog.h>
#include <glib.h>
#include <memory>
#include <media_content.h>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"
#include "common/task-queue.h"
#include "common/virtual_fs.h"
#include "content/content_manager.h"

namespace extension {
namespace content {

using common::tools::ReportSuccess;
using common::tools::ReportError;

ContentInstance::ContentInstance() :
    noti_handle_(nullptr),
    listener_data_(nullptr) {
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ContentInstance::x, this, _1, _2));

  REGISTER_SYNC("ContentManager_find", ContentManagerFind);
  REGISTER_SYNC("ContentManager_update", ContentManagerUpdate);
  REGISTER_SYNC("ContentManager_scanFile", ContentManagerScanfile);
  REGISTER_SYNC("ContentManager_scanDirectory", ContentManagerScanDirectory);
  REGISTER_SYNC("ContentManager_cancelScanDirectory", ContentManagerCancelScanDirectory);
  REGISTER_SYNC("ContentManager_unsetChangeListener", ContentManagerUnsetchangelistener);
  REGISTER_SYNC("ContentManager_setChangeListener", ContentManagerSetchangelistener);
  REGISTER_SYNC("ContentManager_getDirectories", ContentManagerGetdirectories);
  REGISTER_SYNC("ContentManager_updateBatch", ContentManagerUpdatebatch);
  REGISTER_SYNC("ContentManager_removePlaylist", ContentManagerRemoveplaylist);
  REGISTER_SYNC("ContentManager_createPlaylist", ContentManagerCreateplaylist);
  REGISTER_SYNC("ContentManager_getPlaylists", ContentManagerGetplaylists);
  REGISTER_SYNC("ContentPlaylist_add", ContentManagerPlaylistAdd);
  REGISTER_SYNC("ContentPlaylist_addBatch", ContentManagerPlaylistAddbatch);
  REGISTER_SYNC("ContentPlaylist_get", ContentManagerPlaylistGet);
  REGISTER_SYNC("ContentPlaylist_remove", ContentManagerPlaylistRemove);
  REGISTER_SYNC("ContentPlaylist_removeBatch", ContentManagerPlaylistRemovebatch);
  REGISTER_SYNC("ContentPlaylist_setOrder", ContentManagerPlaylistSetorder);
  REGISTER_SYNC("ContentPlaylist_move", ContentManagerPlaylistMove);
  REGISTER_SYNC("ContentManager_getLyrics", ContentManagerAudioGetLyrics);

  REGISTER_SYNC("ContentPlaylist_getName", PlaylistGetName);
  REGISTER_SYNC("ContentPlaylist_setName", PlaylistSetName);
  REGISTER_SYNC("ContentPlaylist_getThumbnailUri", PlaylistGetThumbnailUri);
  REGISTER_SYNC("ContentPlaylist_setThumbnailUri", PlaylistSetThumbnailUri);
  REGISTER_SYNC("ContentPlaylist_getNumberOfTracks", PlaylistGetNumberOfTracks);
  #undef REGISTER_SYNC
}

ContentInstance::~ContentInstance() {
  LoggerD("entered");
  if (noti_handle_) {
    media_content_unset_db_updated_cb_v2(noti_handle_);
    noti_handle_ = nullptr;
  }
  if (listener_data_) {
    delete listener_data_;
    listener_data_ = nullptr;
  }
}

static gboolean CompletedCallback(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("entered");

  picojson::object out;
  out["callbackId"] = picojson::value(user_data->callbackId);

  if (user_data->isSuccess) {
    ReportSuccess(user_data->result, out);
  } else {
    LoggerE("Failed: user_data->isSuccess");
    ReportError(user_data->isSuccess, &out);
  }

  common::Instance::PostMessage(user_data->instance, picojson::value(out).serialize().c_str());

  return false;
}

static void* WorkThread(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("entered");

  ContentCallbacks cbType = user_data->cbType;
  switch(cbType) {
    case ContentManagerUpdatebatchCallback: {
      ContentManager::getInstance()->updateBatch(user_data->args);
      break;
    }
    case ContentManagerGetdirectoriesCallback: {
      ContentManager::getInstance()->getDirectories(user_data);
      break;
    }
    case ContentManagerFindCallback: {
      ContentManager::getInstance()->find(user_data);
      break;
    }
    case ContentManagerScanfileCallback: {
      std::string contentURI = user_data->args.get("contentURI").get<std::string>();
      std::string real_path = common::VirtualFs::GetInstance().GetRealPath(contentURI);
      int res = ContentManager::getInstance()->scanFile(real_path);
      if (res != MEDIA_CONTENT_ERROR_NONE) {
        LOGGER(ERROR) << "Scan file failed, error: " << res;
        common::PlatformResult err(common::ErrorCode::UNKNOWN_ERR, "Scan file failed.");
        user_data->isSuccess = err;
      }
      break;
    }
    case ContentManagerGetplaylistsCallback: {
      ContentManager::getInstance()->getPlaylists(user_data);
      break;
    }
    case ContentManagerCreateplaylistCallback: {
      if (user_data->args.contains("sourcePlaylist")) {
        picojson::object playlist = user_data->args.get("sourcePlaylist").get<picojson::object>();
        user_data->result = picojson::value(playlist);
      }
      else{
        std::string name = user_data->args.get("name").get<std::string>();
        ContentManager::getInstance()->createPlaylist(name, user_data);
      }
      break;
    }
    case ContentManagerRemoveplaylistCallback: {
      std::string id = user_data->args.get("id").get<std::string>();
      ContentManager::getInstance()->removePlaylist(id, user_data);
      // do something...
      break;
    }
    case ContentManagerPlaylistAddbatchCallback: {
      ContentManager::getInstance()->playlistAddbatch(user_data);
      break;
    }
    case ContentManagerPlaylistGetCallback: {
      ContentManager::getInstance()->playlistGet(user_data);
      break;
    }
    case ContentManagerPlaylistRemovebatchCallback: {
      ContentManager::getInstance()->playlistRemovebatch(user_data);
      break;
    }
    case ContentManagerPlaylistSetOrderCallback: {
      ContentManager::getInstance()->playlistSetOrder(user_data);
      break;
      //ContentManagerPlaylistSetOrderCallback
    }
    case ContentManagerPlaylistMoveCallback: {
      ContentManager::getInstance()->playlistMove(user_data);
      break;
    }
    case ContentManagerErrorCallback: {
      common::PlatformResult err(common::ErrorCode::UNKNOWN_ERR, "DB Connection is failed.");
      user_data->isSuccess = err;
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
      return NULL;
    }
  }
  return NULL;
}


static void ScanDirectoryCallback(media_content_error_e error, void* user_data) {
  LoggerD("Enter");

  ReplyCallbackData* cbData = (ReplyCallbackData*) user_data;

  picojson::object out;
  out["callbackId"] = picojson::value(cbData->callbackId);

  if (error == MEDIA_CONTENT_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    LoggerE("Scanning directory failed (error = %d)", error);
    ReportError(out);
  }

  common::Instance::PostMessage(cbData->instance, picojson::value(out).serialize().c_str());
}


static void changedContentCallback(media_content_error_e error,
                                   int pid,
                                   media_content_db_update_item_type_e update_item,
                                   media_content_db_update_type_e update_type,
                                   media_content_type_e media_type,
                                   char* uuid,
                                   char* path,
                                   char* mime_type,
                                   void* user_data) {
  LoggerD("Entered file change callback");

  if (error != MEDIA_CONTENT_ERROR_NONE) {
    LOGGER(ERROR) << "Media content changed callback error: " << error;
    return;
  }

  if (update_item == MEDIA_ITEM_FILE) {
    if (!uuid) {
      LOGGER(ERROR) << "Provided uuid is NULL, ignoring";
      return;
    }

    ReplyCallbackData* cbData = static_cast<ReplyCallbackData*>(user_data);

    int ret;
    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    if (update_type == MEDIA_CONTENT_INSERT || update_type == MEDIA_CONTENT_UPDATE) {
      media_info_h media = NULL;
      ret = media_info_get_media_from_db(uuid, &media);
      if (ret == MEDIA_CONTENT_ERROR_NONE && media != NULL) {
        picojson::object o;

        ContentToJson(media, o);
        ReportSuccess(picojson::value(o), obj);

        if (update_type == MEDIA_CONTENT_INSERT) {
          obj["state"] = picojson::value("oncontentadded");
        } else {
          obj["state"] = picojson::value("oncontentupdated");
        }

        media_info_destroy(media);
      }
    } else {
      ReportSuccess(picojson::value(std::string(uuid)), obj);
      obj["state"] = picojson::value("oncontentremoved");
    }

    obj["listenerId"] = cbData->args.get("listenerId");
    common::Instance::PostMessage(cbData->instance, result.serialize().c_str());
  } else {
    LOGGER(DEBUG) << "Media item is not a file, skipping.";
    return;
  }
}

static void changedContentV2Callback(media_content_error_e error,
                                   int pid,
                                   media_content_db_update_item_type_e update_item,
                                   media_content_db_update_type_e update_type,
                                   media_content_type_e media_type,
                                   char* uuid,
                                   char* path,
                                   char* mime_type,
                                   void* user_data) {
  LoggerD("Entered directory change callback");

  if (error != MEDIA_CONTENT_ERROR_NONE) {
    LOGGER(ERROR) << "Media content changed v2 callback error: " << error;
    return;
  }

  if (update_item == MEDIA_ITEM_DIRECTORY) {
    if (!uuid) {
      LOGGER(ERROR) << "Provided uuid is NULL, ignoring";
      return;
    }

    ReplyCallbackData* cbData = static_cast<ReplyCallbackData*>(user_data);

    int ret;
    picojson::value result = picojson::value(picojson::object());
    picojson::object& obj = result.get<picojson::object>();

    if (update_type == MEDIA_CONTENT_INSERT || update_type == MEDIA_CONTENT_UPDATE) {
      media_folder_h folder = NULL;
      ret = media_folder_get_folder_from_db(uuid, &folder);
      if (ret == MEDIA_CONTENT_ERROR_NONE && folder != NULL) {
        picojson::object o;

        ContentDirToJson(folder, o);
        ReportSuccess(picojson::value(o), obj);

        if (update_type == MEDIA_CONTENT_INSERT) {
          obj["state"] = picojson::value("oncontentdiradded");
        } else {
          obj["state"] = picojson::value("oncontentdirupdated");
        }

        media_folder_destroy(folder);
      }
    } else {
      ReportSuccess(picojson::value(std::string(uuid)), obj);
      obj["state"] = picojson::value("oncontentdirremoved");
    }

    obj["listenerId"] = cbData->args.get("listenerId");
    common::Instance::PostMessage(cbData->instance, result.serialize().c_str());
  } else {
    LOGGER(DEBUG) << "Media item is not directory, skipping.";
    return;
  }
}

#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(common::PlatformResult(common::ErrorCode::TYPE_MISMATCH_ERR, (name" is required argument")), &out);\
    return;\
  }


void ContentInstance::ContentManagerUpdate(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  if (ContentManager::getInstance()->isConnected()) {
    int ret = ContentManager::getInstance()->update(args);
    if (ret != 0) {
      LoggerE("Failed: ContentManager::getInstance()");
      ReportError(ContentManager::getInstance()->convertError(ret), &out);
    }
  } else {
    LoggerE("Failed: DB connection is failed");
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::ContentManagerUpdatebatch(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerUpdatebatchCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}
void ContentInstance::ContentManagerGetdirectories(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "callbackId", out)

  double callbackId = args.get("callbackId").get<double>();
  // implement it

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerGetdirectoriesCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);

}
void ContentInstance::ContentManagerFind(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "callbackId", out)

  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;
  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerFindCallback;
  } else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);

}

void ContentInstance::ContentManagerScanfile(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "contentURI", out)

  double callbackId = args.get("callbackId").get<double>();
  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;
  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerScanfileCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}


void ContentInstance::ContentManagerScanDirectory(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "contentDirURI", out)
  CHECK_EXIST(args, "recursive", out)

  ReplyCallbackData* cbData = new ReplyCallbackData;
  cbData->callbackId = args.get("callbackId").get<double>();
  cbData->instance = this;
  cbData->args = args;

  common::PlatformResult result = ContentManager::getInstance()->scanDirectory(ScanDirectoryCallback, cbData);
  if (result.IsError()) {
    ReportError(result, &out);
  }
}


void ContentInstance::ContentManagerCancelScanDirectory(const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "contentDirURI", out)
  const std::string& content_dir_uri = args.get("contentDirURI").get<std::string>();

  if (ContentManager::getInstance()->cancelScanDirectory(content_dir_uri).IsError()) {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "Cancel scan directory failed"), &out);
  }
}


void ContentInstance::ContentManagerSetchangelistener(const picojson::value& args,
                                                      picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "listenerId", out)

  if (!listener_data_) {
    listener_data_ = new ReplyCallbackData();
  }

  listener_data_->instance = this;
  listener_data_->args = args;
  if (ContentManager::getInstance()->isConnected()) {
    listener_data_->cbType = ContentManagerSetchangelistenerCallback;
  } else {
    listener_data_->cbType = ContentManagerErrorCallback;
  }

  if (ContentManager::getInstance()->setChangeListener(changedContentCallback,
                                                       static_cast<void*>(listener_data_)).IsError()) {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "The callback did not register properly"), &out);
  }
  if (ContentManager::getInstance()->setV2ChangeListener(&noti_handle_,
                                                       changedContentV2Callback,
                                                       static_cast<void*>(listener_data_)).IsError()) {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "The callback did not register properly"), &out);
  }
}

void ContentInstance::ContentManagerUnsetchangelistener(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  if (ContentManager::getInstance()->unSetChangeListener().IsError()) {
    LoggerD("unsuccesfull deregistering of callback");
  }
  if (ContentManager::getInstance()->unSetV2ChangeListener(&noti_handle_).IsError()) {
    LoggerD("unsuccesfull deregistering of callback");
  }
}

void ContentInstance::ContentManagerGetplaylists(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "callbackId", out)

  double callbackId = args.get("callbackId").get<double>();

  // implement it
  std::shared_ptr<ReplyCallbackData>cbData(new ReplyCallbackData);

  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;
  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerGetplaylistsCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);

}
void ContentInstance::ContentManagerCreateplaylist(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)

  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerCreateplaylistCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}
void ContentInstance::ContentManagerRemoveplaylist(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerRemoveplaylistCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  // implement it
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);

}

void ContentInstance::ContentManagerPlaylistAdd(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  if(ContentManager::getInstance()->isConnected()) {
    std::string playlist_id = args.get("playlistId").get<std::string>();
    std::string content_id = args.get("contentId").get<std::string>();
    int ret = ContentManager::getInstance()->playlistAdd(playlist_id, content_id);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),&out);
    }
  }
  else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::ContentManagerPlaylistAddbatch(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerPlaylistAddbatchCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}


void ContentInstance::ContentManagerPlaylistGet(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerPlaylistGetCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}

void ContentInstance::ContentManagerPlaylistRemove(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  if(ContentManager::getInstance()->isConnected()) {
    std::string playlist_id = args.get("playlistId").get<std::string>();
    int member_id = args.get("memberId").get<double>();
    int ret = ContentManager::getInstance()->playlistRemove(playlist_id, member_id);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),&out);
    }
  }
  else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::ContentManagerPlaylistRemovebatch(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerPlaylistRemovebatchCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}


void ContentInstance::ContentManagerPlaylistSetorder(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerPlaylistSetOrderCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}

void ContentInstance::ContentManagerPlaylistMove(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  double callbackId = args.get("callbackId").get<double>();

  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;

  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerPlaylistMoveCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }
  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
}

void ContentInstance::ContentManagerAudioGetLyrics(const picojson::value& args,
                                                   picojson::object& out) {
  LoggerD("entered");
  LOGGER(DEBUG) << "entered";

  picojson::object lyrics;
  if (ContentManager::getInstance()->isConnected()) {
    int ret = ContentManager::getInstance()->getLyrics(args, lyrics);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret), &out);
    } else {
      ReportSuccess(picojson::value(lyrics), out);
    }
  } else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::PlaylistGetName(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  int ret;
  CHECK_EXIST(args, "id", out)
  int id = static_cast<int>(args.get("id").get<double>());
  std::string name;
  ret = ContentManager::getInstance()->getPlaylistName(id, &name);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    ReportError(ContentManager::getInstance()->convertError(ret), &out);
  } else {
    ReportSuccess(picojson::value(name),out);
  }
}

void ContentInstance::PlaylistSetName(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  int ret;
  CHECK_EXIST(args, "id", out)
  CHECK_EXIST(args, "name", out)
  int id = static_cast<int>(args.get("id").get<double>());
  std::string name = args.get("name").get<std::string>();
  ret = ContentManager::getInstance()->setPlaylistName(id, name);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    ReportError(ContentManager::getInstance()->convertError(ret), &out);
  } else {
    ReportSuccess(out);
  }
}

void ContentInstance::PlaylistGetThumbnailUri(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  int ret;
  CHECK_EXIST(args, "id", out)
  int id = static_cast<int>(args.get("id").get<double>());
  std::string uri;
  ret = ContentManager::getInstance()->getThumbnailUri(id, &uri);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    ReportError(ContentManager::getInstance()->convertError(ret), &out);
  } else {
    ReportSuccess(picojson::value(uri),out);
  }
}

void ContentInstance::PlaylistSetThumbnailUri(const picojson::value& args, picojson::object& out) {
  LoggerD("entered");
  int ret;
  CHECK_EXIST(args, "id", out)
  CHECK_EXIST(args, "uri", out)
  int id = static_cast<int>(args.get("id").get<double>());
  std::string uri = args.get("uri").get<std::string>();
  ret = ContentManager::getInstance()->setThumbnailUri(id, uri);
  if(ret != MEDIA_CONTENT_ERROR_NONE) {
    ReportError(ContentManager::getInstance()->convertError(ret), &out);
  } else {
    ReportSuccess(out);
  }
}

void ContentInstance::PlaylistGetNumberOfTracks(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("entered");
  CHECK_EXIST(args, "id", out)
  int id = static_cast<int>(args.get("id").get<double>());
  int count = 0;
  int ret = ContentManager::getInstance()->getNumberOfTracks(id, &count);
  if (ret != MEDIA_CONTENT_ERROR_NONE) {
    ReportError(ContentManager::getInstance()->convertError(ret), &out);
  } else {
    ReportSuccess(picojson::value(static_cast<double>(count)), out);
  }
}

#undef CHECK_EXIST

} // namespace content
} // namespace extension
