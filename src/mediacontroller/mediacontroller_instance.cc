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

#include "mediacontroller/mediacontroller_instance.h"

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"
#include "common/task-queue.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

using common::ErrorCode;
using common::PlatformResult;
using common::TaskQueue;

MediaControllerInstance::MediaControllerInstance() {
  LoggerD("Enter");
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
  REGISTER_SYNC("MediaControllerServer_replyCommand",
      MediaControllerServerReplyCommand);
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
  LoggerD("Enter");
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

  LoggerD("Enter");
  if (server_) {
    ReportSuccess(out);
    return;
  }

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
  LoggerD("Enter");
  CHECK_EXIST(args, "state", out)

  if (!server_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  const std::string& state = args.get("state").get<std::string>();
  const PlatformResult& result = server_->SetPlaybackState(state);
  if (!result) {
    LoggerE("Failed server_->SetPlaybackState()");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdatePlaybackPosition(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "position", out)

  double position = args.get("position").get<double>();
  const PlatformResult& result = server_->SetPlaybackPosition(position);
  if (!result) {
    LoggerE("Failed: server_->SetPlaybackPosition()");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdateShuffleMode(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  const PlatformResult& result = server_->SetShuffleMode(mode);
  if (!result) {
    LoggerE("Failed: server_->SetShuffleMode()");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdateRepeatMode(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");

  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  const PlatformResult& result = server_->SetRepeatMode(mode);
  if (!result) {
    LoggerE("Failed: server_->SetRepeatMode()");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerUpdateMetadata(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
        "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "metadata", out)

  const picojson::object& metadata =
      args.get("metadata").get<picojson::object>();

  const PlatformResult& result = server_->SetMetadata(metadata);
  if (!result) {
    LoggerE("Failed: server_->SetMetadata()");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerAddChangeRequestPlaybackInfoListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
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

    Instance::PostMessage(this, data->serialize().c_str());
  };

  server_->SetChangeRequestPlaybackInfoListener(callback);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerRemoveChangeRequestPlaybackInfoListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
    return;
  }

  server_->SetChangeRequestPlaybackInfoListener(nullptr);
}

void MediaControllerInstance::MediaControllerServerAddCommandListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
    return;
  }

  JsonCallback on_command = [this, args](picojson::value* request) -> void {

    picojson::object& request_o = request->get<picojson::object>();
    request_o["listenerId"] = args.get("listenerId");

    Instance::PostMessage(this, request->serialize().c_str());
  };

  server_->set_command_listener(on_command);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerReplyCommand(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "clientName", out)
  CHECK_EXIST(args, "replyId", out)
  CHECK_EXIST(args, "data", out)

  server_->CommandReply(args.get("clientName").get<std::string>(),
                        args.get("replyId").to_str(),
                        args.get("data"));

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerRemoveCommandListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!server_) {
    LoggerE("Failed: server_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Server not initialized."), &out);
    return;
  }

  server_->set_command_listener(nullptr);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerManagerGetClient(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (client_) {
    ReportSuccess(out);
    return;
  }

  client_ = std::make_shared<MediaControllerClient>();
  const PlatformResult& result = client_->Init();
  if (!result) {
    client_.reset();
    LoggerE("Failed: client_->Init()");
    ReportError(result, &out);
  }

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerClientFindServers(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    LoggerE("Failed: client_");

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

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(search);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerClientGetLatestServerInfo(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    LoggerE("Failed: client_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  picojson::value server_info = picojson::value();
  PlatformResult result = client_->GetLatestServerInfo(&server_info);
  if (!result) {
    LoggerE("Failed: client_->GetLatestServerInfo");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(server_info, out);
}

void MediaControllerInstance::MediaControllerClientGetPlaybackInfo(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
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
    LoggerE("Failed: client_->GetPlaybackInfo");
    ReportError(result, &out);
    return;
  }

  ReportSuccess(playback_info, out);
}

void MediaControllerInstance::MediaControllerServerInfoSendPlaybackState(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    LoggerE("Failed: client_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)
  CHECK_EXIST(args, "state", out)

  auto send = [this, args]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = args.get("callbackId");

    PlatformResult result = client_->SendPlaybackState(
        args.get("name").get<std::string>(),
        args.get("state").get<std::string>());

    if (result) {
      ReportSuccess(response_obj);
    } else {
      LoggerE("Failed: client_->SendPlaybackState");
      ReportError(result, &response_obj);
    }

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(send);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendPlaybackPosition(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)
  CHECK_EXIST(args, "position", out)

  auto send = [this, args]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = args.get("callbackId");

    PlatformResult result = client_->SendPlaybackPosition(
        args.get("name").get<std::string>(),
        args.get("position").get<double>());

    if (result) {
      ReportSuccess(response_obj);
    } else {
      LoggerE("Failed: client_->SendPlaybackPosition");
      ReportError(result, &response_obj);
    }

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(send);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendShuffleMode(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");

  if (!client_) {
    LOGGER(ERROR) << "Client not initialized.";
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)
  CHECK_EXIST(args, "mode", out)

  auto send = [this, args]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = args.get("callbackId");

    PlatformResult result = client_->SendShuffleMode(
        args.get("name").get<std::string>(),
        args.get("mode").get<bool>());

    if (result) {
      ReportSuccess(response_obj);
    } else {
      ReportError(result, &response_obj);
    }

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(send);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendRepeatMode(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    LOGGER(ERROR) << "Client not initialized.";
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "name", out)
  CHECK_EXIST(args, "mode", out)

  auto send = [this, args]() -> void {
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj["callbackId"] = args.get("callbackId");

    PlatformResult result = client_->SendRepeatMode(
        args.get("name").get<std::string>(),
        args.get("mode").get<bool>());

    if (result) {
      ReportSuccess(response_obj);
    } else {
      ReportError(result, &response_obj);
    }

    Instance::PostMessage(this, response.serialize().c_str());
  };

  TaskQueue::GetInstance().Async(send);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendCommand(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    LOGGER(ERROR) << "Client not initialized.";
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "listenerId", out)
  CHECK_EXIST(args, "replyId", out)
  CHECK_EXIST(args, "name", out)
  CHECK_EXIST(args, "command", out)
  CHECK_EXIST(args, "data", out)

  JsonCallback reply_cb = [this, args](picojson::value* reply) -> void {

    picojson::object& reply_obj = reply->get<picojson::object>();

    reply_obj["listenerId"] = args.get("listenerId");

    Instance::PostMessage(this, reply->serialize().c_str());
  };

  PlatformResult result = client_->SendCommand(
      args.get("name").get<std::string>(),
      args.get("command").get<std::string>(),
      args.get("data"),
      args.get("replyId").to_str(),
      reply_cb);

  if (result) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void MediaControllerInstance::MediaControllerServerInfoAddServerStatusChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  CHECK_EXIST(args, "listenerId", out)

  JsonCallback callback = [this, args](picojson::value* data) -> void {

    if (nullptr == data) {
      LOGGER(ERROR) << "No data passed to json callback";
      return;
    }

    picojson::object& request_o = data->get<picojson::object>();
    request_o["listenerId"] = args.get("listenerId");

    Instance::PostMessage(this, data->serialize().c_str());
  };

  client_->SetServerStatusChangeListener(callback);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoRemoveServerStatusChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  client_->SetServerStatusChangeListener(nullptr);
}

void MediaControllerInstance::MediaControllerServerInfoAddPlaybackInfoChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  LoggerD("Enter");
  if (!client_) {
    LoggerE("client_");
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

    Instance::PostMessage(this, data->serialize().c_str());
  };

  client_->SetPlaybackInfoListener(callback);

  ReportSuccess(out);
}

void MediaControllerInstance::MediaControllerServerInfoRemovePlaybackInfoChangeListener(
    const picojson::value& args,
    picojson::object& out) {

  if (!client_) {
    LoggerE("client_");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Client not initialized."), &out);
    return;
  }

  client_->SetPlaybackInfoListener(nullptr);
}

#undef CHECK_EXIST

} // namespace mediacontroller
} // namespace extension
