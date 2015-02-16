// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/content_instance.h"

#include <functional>
#include <string>
#include <dlog.h>
#include <glib.h>
#include <memory>
#include <media_content.h>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/task-queue.h"

#include "content_manager.h"

namespace extension {
namespace content {

namespace {
// The privileges that required in Content API
const std::string kPrivilegeContent = "";

} // namespace

using namespace common;
using namespace extension::content;

ContentInstance::ContentInstance() {
  using namespace std::placeholders;
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

  //  
  #undef REGISTER_SYNC
}

ContentInstance::~ContentInstance() {
  LoggerE("<<endterd>>");
}

static void ReplyAsync(ContentInstance* instance, ContentCallbacks cbfunc, 
                       double callbackId, bool isSuccess, picojson::object& param) {
  LoggerE("<<endterd>>");
  param["callbackId"] = picojson::value(static_cast<double>(callbackId));
  param["status"] = picojson::value(isSuccess ? "success" : "error");
  
  picojson::value result = picojson::value(param);
  
  instance->PostMessage(result.serialize().c_str());
}

static gboolean CompletedCallback(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerE("<<endterd>>");

  picojson::value::object reply;
  reply["value"] = user_data->result;
  LoggerE("CompletedCallback...(%d)" , user_data->isSuccess);
  ReplyAsync(user_data->instance,user_data->cbType,user_data->callbackId,user_data->isSuccess,reply);

  return false;  
}

static void* WorkThread(const std::shared_ptr<ReplyCallbackData>& user_data) {
  LoggerE("<<endterd>>");
  user_data->isSuccess = true;
  ContentCallbacks cbType = user_data->cbType;
  switch(cbType) {
    case ContentManagerUpdatebatchCallback: {
      LoggerE("ContentManagerUpdatebatchCallback...");
      ContentManager::getInstance()->updateBatch(user_data->args);
      break;
    }
    case ContentManagerGetdirectoriesCallback: {
      LoggerE("ContentManagerGetdirectoriesCallback...");
      ContentManager::getInstance()->getDirectories(user_data);
      break;
    }
    case ContentManagerFindCallback: {
      ContentManager::getInstance()->find(user_data);        
      LoggerE("ContentManagerFindCallback...:%s", user_data->result.serialize().c_str());
      break;
    }
    case ContentManagerScanfileCallback: {
      std::string contentURI = user_data->args.get("contentURI").get<std::string>();
      ContentManager::getInstance()->scanFile(contentURI);
      break;
    }
    case ContentManagerGetplaylistsCallback: {
      LoggerE("ContentManagerGetplaylistsCallback...");
      ContentManager::getInstance()->getPlaylists(user_data);
      break;
    }
    case ContentManagerCreateplaylistCallback: {
      LoggerE("ContentManagerCreateplaylistCallback...");
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
      LoggerE("ContentManagerRemoveplaylistCallback...");
      std::string id = user_data->args.get("id").get<std::string>();
      ContentManager::getInstance()->removePlaylist(id, user_data);
      // do something...
      break;
    }
    case ContentManagerPlaylistAddbatchCallback: {
      LoggerE("ContentManagerPlaylistAddBatchCallback...");
      ContentManager::getInstance()->playlistAddbatch(user_data);
      break;
    }
    case ContentManagerPlaylistGetCallback: {
      LoggerE("ContentManagerPlaylistGetCallback...");
      ContentManager::getInstance()->playlistGet(user_data);
      break;
    }
    case ContentManagerPlaylistRemovebatchCallback: {
      LoggerE("ContentManagerPlaylistGetCallback...");
      ContentManager::getInstance()->playlistRemovebatch(user_data);
      break;
    }
    case ContentManagerPlaylistSetOrderCallback: {
      LoggerE("ContentManagerPlaylistSetOrderCallback...");
      ContentManager::getInstance()->playlistSetOrder(user_data);
      break;
      //ContentManagerPlaylistSetOrderCallback
    }
    case ContentManagerPlaylistMoveCallback: {
      LoggerE("ContentManagerPlaylistMove...");
      ContentManager::getInstance()->playlistMove(user_data);
      break;
    }
    case ContentManagerErrorCallback: {
      UnknownException err("DB Connection is failed.");
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

static void changedContentCallback(media_content_error_e error, int pid, media_content_db_update_item_type_e update_item, 
  media_content_db_update_type_e update_type, media_content_type_e media_type, 
  char *uuid, char *path, char *mime_type, void* user_data) {

  int ret;
  std::shared_ptr<ReplyCallbackData> *cbData = (std::shared_ptr<ReplyCallbackData>*)(user_data);
  LoggerE("ContentInstance::ContentManagerScanfile");
  picojson::object reply;
  
  picojson::object o;
  if( error == MEDIA_CONTENT_ERROR_NONE) {
    if( update_item == MEDIA_ITEM_FILE) {
      if(update_type == MEDIA_CONTENT_INSERT || update_type == MEDIA_CONTENT_UPDATE) {
        media_info_h media = NULL;
        std::string id(uuid);
        ret = media_info_get_media_from_db(id.c_str(), &media);
        if (ret == MEDIA_CONTENT_ERROR_NONE && media != NULL) {
          ContentManager::getInstance()->contentToJson(media, o);
          reply["value"] = picojson::value(o);
          if (update_type == MEDIA_CONTENT_INSERT) {
            reply["status"] = picojson::value("oncontentadded");
          }
          else if (update_type == MEDIA_CONTENT_UPDATE) {
            reply["status"] = picojson::value("oncontentupdated");
          }
        }
      }
      else {
        reply["value"] = picojson::value(std::string(uuid));
        reply["status"] = picojson::value("oncontentremoved");
      }
    }
  }
  else {
    return;
  }
  reply["callbackId"] = picojson::value(static_cast<double>((*cbData)->callbackId));
  picojson::value result = picojson::value(reply);
  (*cbData)->instance->PostMessage(result.serialize().c_str());
}


#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), out);\
    return;\
  }


void ContentInstance::ContentManagerUpdate(const picojson::value& args, picojson::object& out) {
  LoggerE("ContentInstance::ContentManagerUpdate");
  int ret;
  if(ContentManager::getInstance()->isConnected()) {
    ret = ContentManager::getInstance()->update(args);
    if(ret != 0) {
      ReportError(ContentManager::getInstance()->convertError(ret),out);
    }
  }
  else {
    ReportError(UnknownException("DB connection is failed."),out);
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
  dlog_print(DLOG_INFO, "DYKIM", "ContentInstance::getDirectories started");
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

  // call ReplyAsync in later (Asynchronously)
  
  // if success
  //ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ContentInstance::ContentManagerFind(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  //double count = args.get("count").get<double>();
  //double offset = args.get("offset").get<double>();

  double callbackId = args.get("callbackId").get<double>();
  
  auto cbData = std::shared_ptr<ReplyCallbackData>(new ReplyCallbackData);
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;
  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerFindCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  common::TaskQueue::GetInstance().Queue<ReplyCallbackData>(WorkThread, CompletedCallback, cbData);
  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ContentInstance::ContentManagerScanfile(const picojson::value& args, picojson::object& out) {
  LoggerE("ContentInstance::ContentManagerScanfile");
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
  // implement it
  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ContentInstance::ContentManagerSetchangelistener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  double callbackId = args.get("callbackId").get<double>();

  std::shared_ptr<ReplyCallbackData>cbData(new ReplyCallbackData);
  
  cbData->callbackId = callbackId;
  cbData->instance = this;
  cbData->args = args;
  if(ContentManager::getInstance()->isConnected()) {
    cbData->cbType = ContentManagerSetchangelistenerCallback;
  }
  else {
    cbData->cbType = ContentManagerErrorCallback;
  }

  ContentManager::getInstance()->setChangeListener(changedContentCallback,static_cast<void*>(&cbData));
  
  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ContentInstance::ContentManagerUnsetchangelistener(const picojson::value& args, picojson::object& out) {

  ContentManager::getInstance()->unSetChangeListener();

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void ContentInstance::ContentManagerGetplaylists(const picojson::value& args, picojson::object& out) {
  LoggerE("ContentInstance::ContentManagerGetplaylists");
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
  LoggerE("ContentInstance::ContentManagerCreateplaylist");
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
  LoggerE("ContentInstance::ContentManagerRemoveplaylist");

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


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void ContentInstance::ContentManagerPlaylistAdd(const picojson::value& args, picojson::object& out) {
  LoggerE("ContentInstance::ContentManagerPlaylistAdd");
  int ret;
  if(ContentManager::getInstance()->isConnected()) {
    std::string playlist_id = args.get("playlist_id").get<std::string>();
    std::string content_id = args.get("content_id").get<std::string>();
    LoggerE("playlist:%s / content:%s", playlist_id.c_str() , content_id.c_str());
    ret = ContentManager::getInstance()->playlistAdd(playlist_id, content_id);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),out);
    }
  }
  else {
    ReportError(UnknownException("DB connection is failed."),out);
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
  LoggerE("ContentInstance::ContentManagerPlaylistRemove");
  int ret;
  if(ContentManager::getInstance()->isConnected()) {
    std::string playlist_id = args.get("playlist_id").get<std::string>();
    int member_id = args.get("member_id").get<double>();
    LoggerE("playlist:%s / member_id:%d", playlist_id.c_str() , member_id);
    ret = ContentManager::getInstance()->playlistRemove(playlist_id, member_id);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),out);
    }
  }
  else {
    ReportError(UnknownException("DB connection is failed."),out);
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

void ContentInstance::ContentManagerAudioGetLyrics(const picojson::value& args, picojson::object& out) {
  LoggerE("ContentInstance::ContentManagerAudioGetLyrics");
  int ret;
  picojson::object lyrics;
  if(ContentManager::getInstance()->isConnected()) {
    ret = ContentManager::getInstance()->getLyrics(args,lyrics);
    if(ret != MEDIA_CONTENT_ERROR_NONE) {
      ReportError(ContentManager::getInstance()->convertError(ret),out);
    }
    else {
      ReportSuccess(picojson::value(lyrics),out);
    }
  }
  else {
    ReportError(UnknownException("DB connection is failed."),out);
  }
}
#undef CHECK_EXIST

} // namespace content
} // namespace extension
