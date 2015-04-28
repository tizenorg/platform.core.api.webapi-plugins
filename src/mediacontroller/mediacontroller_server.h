// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  static void OnCommandReceived(const char* client_name,
                                const char* command,
                                bundle* data,
                                void* user_data);
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_
