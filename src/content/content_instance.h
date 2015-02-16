// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CONTENT_INSTANCE_H_
#define CONTENT_CONTENT_INSTANCE_H_

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

//
};

typedef struct _ReplyCallbackData{
  ContentInstance* instance;
  ContentCallbacks cbType;
  double callbackId;
  bool isSuccess;
  picojson::value args;
  picojson::value result;
}ReplyCallbackData;


} // namespace content
} // namespace extension

#endif // CONTENT_CONTENT_INSTANCE_H_
