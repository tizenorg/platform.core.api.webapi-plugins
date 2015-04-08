// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_INSTANCE_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace mediacontroller {

class MediaControllerInstance : public common::ParsedInstance {
 public:
  MediaControllerInstance();
  virtual ~MediaControllerInstance();

 private:
  // server
  void MediaControllerManagerCreateServer(const picojson::value& args, picojson::object& out);
  void MediaControllerServerUpdatePlaybackState(const picojson::value& args, picojson::object& out);
  void MediaControllerServerUpdatePlaybackPosition(const picojson::value& args, picojson::object& out);
  void MediaControllerServerUpdateRepeatMode(const picojson::value& args, picojson::object& out);
  void MediaControllerServerUpdateShuffleMode(const picojson::value& args, picojson::object& out);
  void MediaControllerServerUpdateMetadata(const picojson::value& args, picojson::object& out);
  void MediaControllerServerAddChangeRequestPlaybackInfoListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerRemoveChangeRequestPlaybackInfoListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerAddCommandListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerRemoveCommandListener(const picojson::value& args, picojson::object& out);

  // client
  void MediaControllerManagerGetClient(const picojson::value& args, picojson::object& out);
  void MediaControllerClientFindServers(const picojson::value& args, picojson::object& out);
  void MediaControllerClientGetLatestServerInfo(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendPlaybackState(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendPlaybackPosition(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendRepeatMode(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendShuffleMode(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendCommand(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoAddServerStatusChangeListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoRemoveServerStatusChangeListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoAddPlaybackInfoChangeListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoRemovePlaybackInfoChangeListener(const picojson::value& args, picojson::object& out);
};

} // namespace mediacontroller
} // namespace extension

#endif // MEDIACONTROLLER_MEDIACONTROLLER_INSTANCE_H_
