// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_instance.h"

#include <functional>

#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace mediacontroller {

namespace {
// The privileges required in MediaController API
const std::string kPrivilegeMediaControllerClient =
    "http://tizen.org/privilege/mediacontroller.client";
const std::string kPrivilegeMediaControllerServer =
    "http://tizen.org/privilege/mediacontroller.server";

} // namespace

using common::PlatformResult;
using common::ErrorCode;

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

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerUpdatePlaybackState(
    const picojson::value& args,
    picojson::object& out) {

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerUpdatePlaybackPosition(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "position", out)

  double position = args.get("position").get<double>();

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerUpdateShuffleMode(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerUpdateRepeatMode(
    const picojson::value& args,
    picojson::object& out) {

  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerUpdateMetadata(
    const picojson::value& args,
    picojson::object& out) {

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerAddChangeRequestPlaybackInfoListener(
    const picojson::value& args,
    picojson::object& out) {

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerRemoveChangeRequestPlaybackInfoListener(
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

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerClientFindServers(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerClientGetLatestServerInfo(
    const picojson::value& args,
    picojson::object& out) {

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoSendPlaybackState(
    const picojson::value& args,
    picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
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

  // implement it

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}

void MediaControllerInstance::MediaControllerServerInfoRemovePlaybackInfoChangeListener(
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

#undef CHECK_EXIST

} // namespace mediacontroller
} // namespace extension
