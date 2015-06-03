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

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_CLIENT_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_CLIENT_H_

#include <media_controller_client.h>
#include <string>

#include "common/platform_result.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

class MediaControllerClient {
 public:
  MediaControllerClient();
  virtual ~MediaControllerClient();

  common::PlatformResult Init();
  common::PlatformResult FindServers(picojson::array* servers);
  common::PlatformResult GetLatestServerInfo(picojson::value* server_info);
  common::PlatformResult GetPlaybackInfo(const std::string& server_name,
                                         picojson::object* playback_info);
  common::PlatformResult GetMetadata(const std::string& server_name,
                                         picojson::object* metadata);

  common::PlatformResult SendPlaybackState(const std::string& server_name,
                                           const std::string& state);
  common::PlatformResult SendPlaybackPosition(const std::string& server_name,
                                              double position);
  common::PlatformResult SendShuffleMode(const std::string& server_name,
                                         bool mode);
  common::PlatformResult SendRepeatMode(const std::string& server_name,
                                        bool mode);

  common::PlatformResult SendCommand(const std::string& server_name,
                                     const std::string& command,
                                     const picojson::value& data,
                                     const std::string& reply_id,
                                     const JsonCallback& reply_cb);

  common::PlatformResult SetServerStatusChangeListener(JsonCallback callback);
  common::PlatformResult SetPlaybackInfoListener(JsonCallback callback);

 private:
  mc_client_h handle_;

  JsonCallback playback_info_listener_;
  JsonCallback server_status_listener_;
  JsonCallback command_reply_callback_;

  static bool FindServersCallback(const char* server_name, void* user_data);

  static void OnServerStatusUpdate(const char *server_name,
                                   mc_server_state_e state,
                                   void *user_data);
  static void OnCommandReply(const char* server_name,
                             int result_code,
                             bundle* bundle,
                             void* user_data);
  static void OnPlaybackUpdate(const char *server_name,
                               mc_playback_h playback,
                               void *user_data);
  static void OnShuffleModeUpdate(const char *server_name,
                                  mc_shuffle_mode_e mode,
                                  void *user_data);
  static void OnRepeatModeUpdate(const char *server_name,
                                  mc_repeat_mode_e mode,
                                  void *user_data);
  static void OnMetadataUpdate(const char* server_name,
                               mc_metadata_h metadata_h,
                               void* user_data);
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_CLIENT_H_
