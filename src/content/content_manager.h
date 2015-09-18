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

#ifndef CONTENT_MANAGER_H_
#define CONTENT_MANAGER_H_

#include <glib.h>
#include <list>
#include <media_content.h>
#include <media_folder.h>
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
void ContentDirToJson(media_folder_h folder, picojson::object& o);

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
  common::PlatformResult scanDirectory(media_scan_completed_cb callback, ReplyCallbackData* cbData);
  common::PlatformResult cancelScanDirectory(const std::string& content_dir_uri);
  common::PlatformResult setChangeListener(media_content_db_update_cb callback,
                                           void *user_data);
  common::PlatformResult unSetChangeListener();
  common::PlatformResult setV2ChangeListener(media_content_noti_h* noti_handler,
                                           media_content_db_update_cb callback,
                                           void *user_data);
  common::PlatformResult unSetV2ChangeListener(media_content_noti_h* noti_handler);

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

  int getNumberOfTracks(int id, int* result);

//playlistSetOrder
  static common::PlatformResult convertError(int err);
 private:
  //int setContent(media_info_h media, picojson::value content);
  ContentManager();

 private:
  bool m_dbConnected;

};

} // namespace power
} // namespace extension

#endif

