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
#include "common/platform_result.h"
#include "content/content_instance.h"

namespace extension {
namespace content {

typedef std::unique_ptr<std::remove_pointer<media_playlist_h>::type,
    void (*)(media_playlist_h&)> PlaylistUniquePtr;

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
  common::PlatformResult setChangeListener(media_content_db_update_cb callback, void *user_data);
  common::PlatformResult unSetChangeListener();

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

  int getPlaylistName(int id, std::string* result);
  int setPlaylistName(int id, const std::string& name);

  int getThumbnailUri(int id, std::string* result);
  int setThumbnailUri(int id, const std::string& thb_uri);

//playlistSetOrder
  common::PlatformResult convertError(int err);
 private:
  //int setContent(media_info_h media, picojson::value content);
  ContentManager();

 private:
  bool m_dbConnected;

};

} // namespace power
} // namespace extension

#endif

