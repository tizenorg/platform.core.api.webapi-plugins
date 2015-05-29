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

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_

#include <media_controller_server.h>
#include <string>

#include "common/platform_result.h"
#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

class MediaControllerServer {
 public:
  MediaControllerServer();
  virtual ~MediaControllerServer();

  common::PlatformResult Init();
  common::PlatformResult SetPlaybackState(const std::string& state);
  common::PlatformResult SetPlaybackPosition(double position);
  common::PlatformResult SetShuffleMode(bool mode);
  common::PlatformResult SetRepeatMode(bool mode);
  common::PlatformResult SetMetadata(const picojson::object& metadata);

  common::PlatformResult SetChangeRequestPlaybackInfoListener(
      JsonCallback callback);

  common::PlatformResult CommandReply(const std::string& client_name,
                                      const std::string& reply_id,
                                      const picojson::value& data);

  void set_command_listener(const JsonCallback& func) {
    command_listener_ = func;
  }

 private:
  mc_server_h handle_;

  JsonCallback change_request_playback_info_listener_;
  JsonCallback command_listener_;

  static void OnPlaybackStateCommand(const char* client_name,
                                     mc_playback_states_e state_e,
                                     void *user_data);
  static void OnPlaybackPositionCommand(const char* client_name,
                                        unsigned long long position,
                                        void* user_data);
  static void OnShuffleModeCommand(const char* client_name,
                                   mc_shuffle_mode_e mode,
                                   void* user_data);
  static void OnRepeatModeCommand(const char* client_name,
                                  mc_repeat_mode_e mode,
                                  void* user_data);

  static void OnCommandReceived(const char* client_name,
                                const char* command,
                                bundle* data,
                                void* user_data);
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_
