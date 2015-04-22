// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_instance.h"

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"
#include "common/task-queue.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

namespace {
// The privileges required in MediaController API
const std::string kPrivilegeMediaControllerClient =
    "http://tizen.org/privilege/mediacontroller.write";
const std::string kPrivilegeMediaControllerServer =
    "http://tizen.org/privilege/mediacontroller.read";

} // namespace

using common::ErrorCode;
using common::PlatformResult;
using common::TaskQueue;

MediaControllerInstance::MediaControllerInstance() {
  using namespace std::placeholders;

#define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&MediaControllerInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&MediaControllerInstance::x, this, _1, _2));

  // server
  REGISTER_SYNC("MediaControllerManager_createServer",
      MediaControllerManagerCreateServer);
  REGISTER_SYNC("MediaControllerServer_updatePlaybackState",
      MediaControllerServerUpdatePlaybackState);
  REGISTER_SYNC("MediaControllerServer_updatePlaybackPosition",
      MediaControllerServerUpdatePlaybackPosition);
  REGISTER_SYNC("MediaControllerServer_updateRepeatMode",
      MediaControllerServerUpdateRepeatMode);
  REGISTER_SYNC("MediaControllerServer_updateShuffleMode",
      MediaControllerServerUpdateShuffleMode);
  REGISTER_SYNC("MediaControllerServer_updateMetadata",
      MediaControllerServerUpdateMetadata);
  REGISTER_SYNC("MediaControllerServer_addChangeRequestPlaybackInfoListener",
      MediaControllerServerAddChangeRequestPlaybackInfoListener);
  REGISTER_SYNC("MediaControllerServer_removeChangeRequestPlaybackInfoListener",
      MediaControllerServerRemoveChangeRequestPlaybackInfoListener);
  REGISTER_SYNC("MediaControllerServer_addCommandListener",
      MediaControllerServerAddCommandListener);
  REGISTER_SYNC("MediaControllerServer_removeCommandListener",
      MediaControllerServerRemoveCommandListener);

  // client
  REGISTER_SYNC("MediaControllerManager_getClient",
      MediaControllerManagerGetClient);
  REGISTER_ASYNC("MediaControllerClient_findServers",
      MediaControllerClientFindServers);
  REGISTER_SYNC("MediaControllerClient_getLatestServerInfo",
      MediaControllerClientGetLatestServerInfo);
  REGISTER_SYNC("MediaControllerClient_getPlaybackInfo",
      MediaControllerClientGetPlaybackInfo);
  REGISTER_SYNC("MediaControllerServerInfo_sendPlaybackState",
      MediaControllerServerInfoSendPlaybackState);
  REGISTER_ASYNC("MediaControllerServerInfo_sendPlaybackPosition",
      MediaControllerServerInfoSendPlaybackPosition);
  REGISTER_ASYNC("MediaControllerServerInfo_sendRepeatMode",
      MediaControllerServerInfoSendRepeatMode);
  REGISTER_ASYNC("MediaControllerServerInfo_sendShuffleMode",
      MediaControllerServerInfoSendShuffleMode);
  REGISTER_ASYNC("MediaControllerServerInfo_sendCommand",
      MediaControllerServerInfoSendCommand);
  REGISTER_SYNC("MediaControllerServerInfo_addServerStatusChangeListener",
      MediaControllerServerInfoAddServerStatusChangeListener);
  REGISTER_SYNC("MediaControllerServerInfo_removeServerStatusChangeListener",
      MediaControllerServerInfoRemoveServerStatusChangeListener);
  REGISTER_SYNC("MediaControllerServerInfo_addPlaybackInfoChangeListener",
      MediaControllerServerInfoAddPlaybackInfoChangeListener);
  REGISTER_SYNC("MediaControllerServerInfo_removePlaybackInfoChangeListener",
      MediaControllerServerInfoRemovePlaybackInfoChangeListener);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

MediaControllerInstance::~MediaControllerInstance() {
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) { \
      ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, \
          name" is required argument"), &out); \
      return; \
    }


void MediaControllerInstance::MediaControllerManagerCreateServer(
    const picojson::value& args,
    picojson::object& out) {

  if (server_) {
    ReportSuccess(out);
    return;
  }

  // TODO(r.galka) check privileges

  server_ = std::make_shared<MediaControllerServer>();
  const PlatformResult& result = server_->Init();
  if (!result) {
    server_.reset();
    ReportError(result, &out);
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdatePlaybackState(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "state", out)

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  const std::string& state = args.get("state").get<std::string>();
  const PlatformResult& result = server_->SetPlaybackState(state);
  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdatePlaybackPosition(
    const picojson::value& args,
    picojson::object& out) {

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "position", out)

  double position = args.get("position").get<double>();
  const PlatformResult& result = server_->SetPlaybackPosition(position);
  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdateShuffleMode(
    const picojson::value& args,
    picojson::object& out) {

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  const PlatformResult& result = server_->SetShuffleMode(mode);
  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdateRepeatMode(
    const picojson::value& args,
    picojson::object& out) {

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  const PlatformResult& result = server_->SetRepeatMode(mode);
  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdateMetadata(
    const picojson::value& args,
    picojson::object& out) {

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "metadata", out)

  const picojson::object& metadata =
      args.get("metadata").get<picojson::object>();

  const PlatformResult& result = server_->SetMetadata(metadata);
  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerAddChangeRequestPlaybackInfoListener(
    const picojson::value& args,
    picojson::object& out) {
  LOGGER(DEBUG) << "entered";

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "listenerId", out)

  JsonCallback callback = [this, args](picojson::value* data) -> void {
    LOGGER(DEBUG) << "entered";

    if (nullptr == data) {
      LOGGER(ERROR) << "No data passed to json callback";
      return;
    }

    picojson::object& request_o = data->get<picojson::object>();
    request_o["listenerId"] = args.get("listenerId");

    PostMessage(data->serialize().c_str());
  };

  server_->SetChangeRequestPlaybackInfoListener(callback);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerRemoveChangeRequestPlaybackInfoListener(
    const picojson::value& args,
    picojson::object& out) {
  LOGGER(DEBUG) << "entered";

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
    return;
  }

  server_->SetChangeRequestPlaybackInfoListener(nullptr);
}

void MediaControllerInstance::MediaControllerServerAddCommandListener(
    const picojson::value& args,
    picojson::object& out) {

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerRemoveCommandListener(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)

  double watchId = args.get("watchId").get<double>();

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerManagerGetClient(
    const picojson::value& args,
    picojson::object& out) {

  if (client_) {
    ReportSuccess(out);
    return;
  }

  // TODO(r.galka) check privileges

  client_ = std::make_shared<MediaControllerClient>();
  const PlatformResult& result = client_->Init();
  if (!result) {
    client_.reset();
    ReportError(result, &out);
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerClientFindServers(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "callbackId", out)

  auto search = [this, args]() -> void {

    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();

    picojson::value servers = picojson::value(picojson::array());
    PlatformResult result = client_->FindServers(
        &servers.get<picojson::array>());

    response_obj["callbackId"] = args.get("callbackId");
    if (result) {
      ReportSuccess(servers, response_obj);
    } else {
      ReportError(result, &response_obj);
    }

    PostMessage(response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(search);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerClientGetLatestServerInfo(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  picojson::value server_info = picojson::value();
  PlatformResult result = client_->GetLatestServerInfo(&server_info);
  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(server_info, out);
}

void MediaControllerInstance::MediaControllerClientGetPlaybackInfo(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "name", out)

  picojson::value playback_info = picojson::value(picojson::object());
  PlatformResult result = client_->GetPlaybackInfo(
      args.get("name").get<std::string>(),
      &playback_info.get<picojson::object>());

  if (!result) {
    ReportError(result, &out);
    return;
  }

  ReportSuccess(playback_info, out);
}

void MediaControllerInstance::MediaControllerServerInfoSendPlaybackState(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)
  CHECK_EXIST(args, "state", out)

  auto send = [this, args]() -> void {
    LOGGER(DEBUG) << "entered";

    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = args.get("callbackId");

    PlatformResult result = client_->SendPlaybackState(
        args.get("name").get<std::string>(),
        args.get("state").get<std::string>());

    if (result) {
      ReportSuccess(response_obj);
    } else {
      ReportError(result, &response_obj);
    }

    PostMessage(response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(send);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendPlaybackPosition(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "position", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  double position = args.get("position").get<double>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendShuffleMode(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "mode", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  bool mode = args.get("mode").get<bool>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendRepeatMode(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "mode", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  bool mode = args.get("mode").get<bool>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendCommand(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "command", out)
  CHECK_EXIST(args, "data", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& command = args.get("command").get<std::string>();
  const picojson::object data = args.get("data").get<picojson::object>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoAddServerStatusChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoRemoveServerStatusChangeListener(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)

  double watchId = args.get("watchId").get<double>();

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoAddPlaybackInfoChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "listenerId", out)

  JsonCallback callback = [this, args](picojson::value* data) -> void {

    if (!data) {
      LOGGER(ERROR) << "No data passed to json callback";
      return;
    }

    picojson::object& request_o = data->get<picojson::object>();
    request_o["listenerId"] = args.get("listenerId");

    PostMessage(data->serialize().c_str());
  };

  client_->SetPlaybackInfoListener(callback);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoRemovePlaybackInfoChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  client_->SetPlaybackInfoListener(nullptr);
}

#undef CHECK_EXIST

} // namespace mediacontroller
} // namespace extension
