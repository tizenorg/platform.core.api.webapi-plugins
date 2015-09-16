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

#ifndef CONTENT_CONTENT_INSTANCE_H_
#define CONTENT_CONTENT_INSTANCE_H_

#include <media_content_internal.h>
#include "common/extension.h"

namespace extension {
namespace content {

enum ContentCallbacks {
  ContentManagerFindCallback,
  ContentManagerScanfileCallback,
  ContentManagerUnsetchangelistenerCallback,
  ContentManagerSetchangelistenerCallback,
  ContentManagerGetdirectoriesCallback,
  ContentManagerUpdatebatchCallback,
  ContentManagerRemoveplaylistCallback,
  ContentManagerCreateplaylistCallback,
  ContentManagerGetplaylistsCallback,
  ContentManagerPlaylistAddbatchCallback,
  ContentManagerPlaylistGetCallback,
  ContentManagerPlaylistRemovebatchCallback,
  ContentManagerPlaylistSetOrderCallback,
  ContentManagerPlaylistMoveCallback,
  ContentManagerErrorCallback
};

class ContentInstance;

typedef struct _ReplyCallbackData {
  _ReplyCallbackData()
      : instance(nullptr),
        cbType(ContentManagerFindCallback),
        callbackId(-1.0),
        isSuccess(common::ErrorCode::NO_ERROR) {
  }
  ContentInstance* instance;
  ContentCallbacks cbType;
  double callbackId;
  picojson::value args;
  picojson::value result;
  common::PlatformResult isSuccess;
} ReplyCallbackData;

class ContentInstance : public common::ParsedInstance {
 public:
  ContentInstance();
  virtual ~ContentInstance();

 private:
  void ContentManagerUpdate(const picojson::value& args, picojson::object& out);
  void ContentManagerUpdatebatch(const picojson::value& args, picojson::object& out);
  void ContentManagerGetdirectories(const picojson::value& args, picojson::object& out);
  void ContentManagerFind(const picojson::value& args, picojson::object& out);
  void ContentManagerScanfile(const picojson::value& args, picojson::object& out);
  void ContentManagerScanDirectory(const picojson::value& args, picojson::object& out);
  void ContentManagerCancelScanDirectory(const picojson::value& args, picojson::object& out);
  void ContentManagerSetchangelistener(const picojson::value& args, picojson::object& out);
  void ContentManagerUnsetchangelistener(const picojson::value& args, picojson::object& out);
  void ContentManagerGetplaylists(const picojson::value& args, picojson::object& out);
  void ContentManagerCreateplaylist(const picojson::value& args, picojson::object& out);
  void ContentManagerRemoveplaylist(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistAdd(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistAddbatch(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistGet(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistRemove(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistRemovebatch(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistSetorder(const picojson::value& args, picojson::object& out);
  void ContentManagerPlaylistMove(const picojson::value& args, picojson::object& out);
  void ContentManagerAudioGetLyrics(const picojson::value& args, picojson::object& out);

  void PlaylistGetName(const picojson::value& args, picojson::object& out);
  void PlaylistSetName(const picojson::value& args, picojson::object& out);
  void PlaylistGetThumbnailUri(const picojson::value& args, picojson::object& out);
  void PlaylistSetThumbnailUri(const picojson::value& args, picojson::object& out);
  void PlaylistGetNumberOfTracks(const picojson::value& args, picojson::object& out);

  media_content_noti_h noti_handle_;
  ReplyCallbackData* listener_data_;
};


} // namespace content
} // namespace extension

#endif // CONTENT_CONTENT_INSTANCE_H_
