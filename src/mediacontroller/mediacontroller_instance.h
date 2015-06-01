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

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_INSTANCE_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_INSTANCE_H_

#include <memory>

#include "common/extension.h"
#include "mediacontroller/mediacontroller_client.h"
#include "mediacontroller/mediacontroller_server.h"

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
  void MediaControllerServerReplyCommand(const picojson::value& args, picojson::object& out);
  void MediaControllerServerRemoveCommandListener(const picojson::value& args, picojson::object& out);

  // client
  void MediaControllerManagerGetClient(const picojson::value& args, picojson::object& out);
  void MediaControllerClientFindServers(const picojson::value& args, picojson::object& out);
  void MediaControllerClientGetLatestServerInfo(const picojson::value& args, picojson::object& out);
  void MediaControllerClientGetPlaybackInfo(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendPlaybackState(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendPlaybackPosition(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendRepeatMode(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendShuffleMode(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoSendCommand(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoAddServerStatusChangeListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoRemoveServerStatusChangeListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoAddPlaybackInfoChangeListener(const picojson::value& args, picojson::object& out);
  void MediaControllerServerInfoRemovePlaybackInfoChangeListener(const picojson::value& args, picojson::object& out);

  std::shared_ptr<MediaControllerClient> client_;
  std::shared_ptr<MediaControllerServer> server_;
};

} // namespace mediacontroller
} // namespace extension

#endif // MEDIACONTROLLER_MEDIACONTROLLER_INSTANCE_H_
