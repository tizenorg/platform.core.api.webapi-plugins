// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace mediacontroller {

namespace {
// The privileges that required in Mediacontroller API
const std::string kPrivilegeMediacontroller = "";

} // namespace

using namespace common;
using namespace extension::mediacontroller;

MediacontrollerInstance::MediacontrollerInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&MediacontrollerInstance::x, this, _1, _2));
  REGISTER_SYNC("MediaControllerServerInfo_sendCommand", MediaControllerServerInfoSendCommand);
  REGISTER_SYNC("MediaControllerServer_updatePlaybackState", MediaControllerServerUpdatePlaybackState);
  REGISTER_SYNC("MediaControllerServer_updateRepeatMode", MediaControllerServerUpdateRepeatMode);
  REGISTER_SYNC("MediaControllerManager_createServer", MediaControllerManagerCreateServer);
  REGISTER_SYNC("MediaControllerServer_addCommandListener", MediaControllerServerAddCommandListener);
  REGISTER_SYNC("MediaControllerServerInfo_removePlaybackInfoChangeListener", MediaControllerServerInfoRemovePlaybackInfoChangeListener);
  REGISTER_SYNC("MediaControllerServer_updatePlaybackPosition", MediaControllerServerUpdatePlaybackPosition);
  REGISTER_SYNC("MediaControllerServer_updateShuffleMode", MediaControllerServerUpdateShuffleMode);
  REGISTER_SYNC("MediaControllerServerInfo_sendPlaybackPosition", MediaControllerServerInfoSendPlaybackPosition);
  REGISTER_SYNC("MediaControllerServerInfo_addPlaybackInfoChangeListener", MediaControllerServerInfoAddPlaybackInfoChangeListener);
  REGISTER_SYNC("MediaControllerServer_addChangeRequestPlaybackInfoListener", MediaControllerServerAddChangeRequestPlaybackInfoListener);
  REGISTER_SYNC("MediaControllerServerInfo_sendPlaybackState", MediaControllerServerInfoSendPlaybackState);
  REGISTER_SYNC("MediaControllerClient_findServers", MediaControllerClientFindServers);
  REGISTER_SYNC("MediaControllerServerInfo_addServerStatusChangeListener", MediaControllerServerInfoAddServerStatusChangeListener);
  REGISTER_SYNC("MediaControllerServer_removeChangeRequestPlaybackInfoListener", MediaControllerServerRemoveChangeRequestPlaybackInfoListener);
  REGISTER_SYNC("MediaControllerServer_removeCommandListener", MediaControllerServerRemoveCommandListener);
  REGISTER_SYNC("MediaControllerServerInfo_sendShuffleMode", MediaControllerServerInfoSendShuffleMode);
  REGISTER_SYNC("MediaControllerServerInfo_removeServerStatusChangeListener", MediaControllerServerInfoRemoveServerStatusChangeListener);
  REGISTER_SYNC("MediaControllerServer_updateMetadata", MediaControllerServerUpdateMetadata);
  REGISTER_SYNC("MediaControllerClient_getLatestServerInfo", MediaControllerClientGetLatestServerInfo);
  REGISTER_SYNC("MediaControllerManager_getClient", MediaControllerManagerGetClient);
  REGISTER_SYNC("MediaControllerServerInfo_sendRepeatMode", MediaControllerServerInfoSendRepeatMode);
  #undef REGISTER_SYNC
}

MediacontrollerInstance::~MediacontrollerInstance() {
}


enum MediacontrollerCallbacks {
  MediaControllerServerInfoSendCommandCallback,
  MediaControllerServerUpdatePlaybackStateCallback,
  MediaControllerServerUpdateRepeatModeCallback,
  MediaControllerManagerCreateServerCallback,
  MediaControllerServerAddCommandListenerCallback,
  MediaControllerServerInfoRemovePlaybackInfoChangeListenerCallback,
  MediaControllerServerUpdatePlaybackPositionCallback,
  MediaControllerServerUpdateShuffleModeCallback,
  MediaControllerServerInfoSendPlaybackPositionCallback,
  MediaControllerServerInfoAddPlaybackInfoChangeListenerCallback,
  MediaControllerServerAddChangeRequestPlaybackInfoListenerCallback,
  MediaControllerServerInfoSendPlaybackStateCallback,
  MediaControllerClientFindServersCallback,
  MediaControllerServerInfoAddServerStatusChangeListenerCallback,
  MediaControllerServerRemoveChangeRequestPlaybackInfoListenerCallback,
  MediaControllerServerRemoveCommandListenerCallback,
  MediaControllerServerInfoSendShuffleModeCallback,
  MediaControllerServerInfoRemoveServerStatusChangeListenerCallback,
  MediaControllerServerUpdateMetadataCallback,
  MediaControllerClientGetLatestServerInfoCallback,
  MediaControllerManagerGetClientCallback,
  MediaControllerServerInfoSendRepeatModeCallback
};

static void ReplyAsync(MediacontrollerInstance* instance, MediacontrollerCallbacks cbfunc,
                       int callbackId, bool isSuccess, picojson::object& param) {
  param["callbackId"] = picojson::value(static_cast<double>(callbackId));
  param["status"] = picojson::value(isSuccess ? "success" : "error");

  // insert result for async callback to param
  switch(cbfunc) {
    case MediaControllerManagerGetClientCallback: {
      // do something...
      break;
    }
    case MediaControllerManagerCreateServerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerUpdatePlaybackStateCallback: {
      // do something...
      break;
    }
    case MediaControllerServerUpdatePlaybackPositionCallback: {
      // do something...
      break;
    }
    case MediaControllerServerUpdateShuffleModeCallback: {
      // do something...
      break;
    }
    case MediaControllerServerUpdateRepeatModeCallback: {
      // do something...
      break;
    }
    case MediaControllerServerUpdateMetadataCallback: {
      // do something...
      break;
    }
    case MediaControllerServerAddChangeRequestPlaybackInfoListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerRemoveChangeRequestPlaybackInfoListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerAddCommandListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerRemoveCommandListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerClientFindServersCallback: {
      // do something...
      break;
    }
    case MediaControllerClientGetLatestServerInfoCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoSendPlaybackStateCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoSendPlaybackPositionCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoSendShuffleModeCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoSendRepeatModeCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoSendCommandCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoAddServerStatusChangeListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoRemoveServerStatusChangeListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoAddPlaybackInfoChangeListenerCallback: {
      // do something...
      break;
    }
    case MediaControllerServerInfoRemovePlaybackInfoChangeListenerCallback: {
      // do something...
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
      return;
    }
  }

  picojson::value result = picojson::value(param);

  instance->PostMessage(result.serialize().c_str());
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }


void MediacontrollerInstance::MediaControllerManagerGetClient(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerManagerCreateServer(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerUpdatePlaybackState(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerUpdatePlaybackPosition(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "position", out)

  double position = args.get("position").get<double>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerUpdateShuffleMode(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerUpdateRepeatMode(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "mode", out)

  bool mode = args.get("mode").get<bool>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerUpdateMetadata(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerAddChangeRequestPlaybackInfoListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerRemoveChangeRequestPlaybackInfoListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)

  double watchId = args.get("watchId").get<double>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerAddCommandListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerRemoveCommandListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)

  double watchId = args.get("watchId").get<double>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerClientFindServers(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerClientGetLatestServerInfo(const picojson::value& args, picojson::object& out) {


  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerInfoSendPlaybackState(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerInfoSendPlaybackPosition(const picojson::value& args, picojson::object& out) {
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
void MediacontrollerInstance::MediaControllerServerInfoSendShuffleMode(const picojson::value& args, picojson::object& out) {
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
void MediacontrollerInstance::MediaControllerServerInfoSendRepeatMode(const picojson::value& args, picojson::object& out) {
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
void MediacontrollerInstance::MediaControllerServerInfoSendCommand(const picojson::value& args, picojson::object& out) {
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
void MediacontrollerInstance::MediaControllerServerInfoAddServerStatusChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerInfoRemoveServerStatusChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "watchId", out)

  double watchId = args.get("watchId").get<double>();

  // implement it


  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerInfoAddPlaybackInfoChangeListener(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void MediacontrollerInstance::MediaControllerServerInfoRemovePlaybackInfoChangeListener(const picojson::value& args, picojson::object& out) {
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