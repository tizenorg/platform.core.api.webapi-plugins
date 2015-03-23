// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_MANAGER_H_
#define CONTENT_MANAGER_H_

#include <glib.h>
#include <list>
#include <media_content.h>
#include <memory>
#include <string>

#include "common/extension.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "content/content_instance.h"

namespace extension {
namespace content {

void ContentToJson(media_info_h info, picojson::object& o);

class ContentManager {
 public:
  virtual ~ContentManager();
  bool isConnected();
  static ContentManager* getInstance();

  void getDirectories(const std::shared_ptr<ReplyCallbackData>& user_data);
  void find(const std::shared_ptr<ReplyCallbackData>& user_data);
  int update(picojson::value args);
  int updateBatch(picojson::value args);

  int scanFile(std::string& uri);
  int setChangeListener(media_content_db_update_cb callback, void *user_data);
  void unSetChangeListener();

//Lyrics
  int getLyrics(const picojson::value& args,picojson::object& result);

//playlist
  void createPlaylist(std::string name, const std::shared_ptr<ReplyCallbackData>& user_data);
  void getPlaylists(const std::shared_ptr<ReplyCallbackData>& user_data);
  void removePlaylist(std::string playlistId, const std::shared_ptr<ReplyCallbackData>& user_data);
  int playlistAdd(std::string playlist_id, std::string content_id);
  int playlistRemove(std::string playlist_id, int member_id);
  void playlistAddbatch(const std::shared_ptr<ReplyCallbackData>& user_data);
  void playlistGet(const std::shared_ptr<ReplyCallbackData>& user_data);
  void playlistRemovebatch(const std::shared_ptr<ReplyCallbackData>& user_data);
  void playlistSetOrder(const std::shared_ptr<ReplyCallbackData>& user_data);
  void playlistMove(const std::shared_ptr<ReplyCallbackData>& user_data);

//playlistSetOrder
  common::PlatformException convertError(int err);
 private:
  //int setContent(media_info_h media, picojson::value content);
  ContentManager();

 private:
  bool m_dbConnected;

};

} // namespace power
} // namespace extension

#endif

