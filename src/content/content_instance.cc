// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
#include "content/content_manager.h"

namespace extension {
namespace content {

namespace {
// The privileges that required in Content API
const std::string kPrivilegeContent = "";

} // namespace

using common::tools::ReportSuccess;
using common::tools::ReportError;

ContentInstance::ContentInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&ContentInstance::x, this, _1, _2));

  REGISTER_SYNC("ContentManager_find", ContentManagerFind);
  REGISTER_SYNC("ContentManager_update", ContentManagerUpdate);
  REGISTER_SYNC("ContentManager_scanFile", ContentManagerScanfile);
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

  #undef REGISTER_SYNC
}

ContentInstance::~ContentInstance() {
  LoggerD("entered");
}

static gboolean CompletedCallback(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("entered");

  picojson::object out;
  out["callbackId"] = picojson::value(user_data->callbackId);

  if (user_data->isSuccess) {
    ReportSuccess(user_data->result, out);
  } else {
    ReportError(out);
  }

  user_data->instance->PostMessage(picojson::value(out).serialize().c_str());

  return false;
}

static void* WorkThread(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerD("entered");

  user_data->isSuccess = true;
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
      int res = ContentManager::getInstance()->scanFile(contentURI);
      if (res != MEDIA_CONTENT_ERROR_NONE) {
        LOGGER(ERROR) << "Scan file failed, error: " << res;
        user_data->isSuccess = false;
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
        user_data->isSuccess = true;
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
      std::string playlist_id = user_data->args.get("playlistId").get<std::string>();
      double member_id = user_data->args.get("memberId").get<double>();
      double delta = user_data->args.get("delta").get<double>();
      ContentManager::getInstance()->playlistMove(user_data);
      break;
    }
    case ContentManagerErrorCallback: {
      common::PlatformResult err(common::ErrorCode::UNKNOWN_ERR, "DB Connection is failed.");
      user_data->isSuccess = false;
      user_data->result = err.ToJSON();
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
      return NULL;
    }
  }
  return NULL;
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
  LoggerD("entered");

  if (error != MEDIA_CONTENT_ERROR_NONE) {
    LOGGER(ERROR) << "Media content changed callback error: " << error;
    return;
  }

  if (update_item != MEDIA_ITEM_FILE) {
    LOGGER(DEBUG) << "Media item is not file, skipping.";
    return;
  }

  int ret;
  ReplyCallbackData* cbData = static_cast<ReplyCallbackData*>(user_data);
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
    }
  } else {
    ReportSuccess(picojson::value(std::string(uuid)), obj);
    obj["state"] = picojson::value("oncontentremoved");
  }

  obj["listenerId"] = cbData->args.get("listenerId");
  cbData->instance->PostMessage(result.serialize().c_str());
}


#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(common::PlatformResult(common::ErrorCode::TYPE_MISMATCH_ERR, (name" is required argument")), &out);\
    return;\
  }


void ContentInstance::ContentManagerUpdate(const picojson::value& args, picojson::object& out) {
  int ret;
  if (ContentManager::getInstance()->isConnected()) {
    ret = ContentManager::getInstance()->update(args);
    if (ret != 0) {
      ReportError(ContentManager::getInstance()->convertError(ret), &out);
    }
  } else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::ContentManagerUpdatebatch(const picojson::value& args, picojson::object& out) {
  LoggerE("entered");
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

void ContentInstance::ContentManagerSetchangelistener(const picojson::value& args,
                                                      picojson::object& out) {
  CHECK_EXIST(args, "listenerId", out)

  ReplyCallbackData* cbData = new ReplyCallbackData();

  cbData->instance = this;
  cbData->args = args;
  if (ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerSetchangelistenerCallback;
  } else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  if (ContentManager::getInstance()->setChangeListener(changedContentCallback, static_cast<void*>(cbData)).IsError()) {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "The callback did not register properly"), &out);
  }
}

void ContentInstance::ContentManagerUnsetchangelistener(const picojson::value& args, picojson::object& out) {
  if (ContentManager::getInstance()->unSetChangeListener().IsError()) {
    LoggerD("unsuccesfull deregistering of callback");
  }
}

void ContentInstance::ContentManagerGetplaylists(const picojson::value& args, picojson::object& out) {
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
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)

  double callbackId = args.get("callbackId").get<double>();
  const std::string& name = args.get("name").get<std::string>();

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
  int ret;
  if(ContentManager::getInstance()->isConnected()) {
    std::string playlist_id = args.get("playlistId").get<std::string>();
    std::string content_id = args.get("contentId").get<std::string>();
    ret = ContentManager::getInstance()->playlistAdd(playlist_id, content_id);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),&out);
    }
  }
  else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::ContentManagerPlaylistAddbatch(const picojson::value& args, picojson::object& out) {
  LoggerE("entered");
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
  LoggerE("entered");
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
  int ret;
  if(ContentManager::getInstance()->isConnected()) {
    std::string playlist_id = args.get("playlistId").get<std::string>();
    int member_id = args.get("memberId").get<double>();
    ret = ContentManager::getInstance()->playlistRemove(playlist_id, member_id);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),&out);
    }
  }
  else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

void ContentInstance::ContentManagerPlaylistRemovebatch(const picojson::value& args, picojson::object& out) {
  LoggerE("entered");
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
  LoggerE("entered");
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
  LoggerE("entered");
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
  LOGGER(DEBUG) << "entered";

  int ret;
  picojson::object lyrics;
  if (ContentManager::getInstance()->isConnected()) {
    ret = ContentManager::getInstance()->getLyrics(args, lyrics);
    if (ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret), &out);
    } else {
      ReportSuccess(picojson::value(lyrics), out);
    }
  } else {
    ReportError(common::PlatformResult(common::ErrorCode::UNKNOWN_ERR, "DB connection is failed."), &out);
  }
}

#undef CHECK_EXIST

} // namespace content
} // namespace extension
