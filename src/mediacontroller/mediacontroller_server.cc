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

#include "mediacontroller/mediacontroller_server.h"

#include <bundle.h>
#include <bundle_internal.h>

#include "common/logger.h"
#include "common/scope_exit.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

namespace {
// The privileges that are required in Application API
const std::string kInternalCommandSendPlaybackPosition
    = "__internal_sendPlaybackPosition";
const std::string kInternalCommandSendShuffleMode
    = "__internal_sendShuffleMode";
const std::string kInternalCommandSendRepeatMode
    = "__internal_sendRepeatMode";
}  // namespace

using common::PlatformResult;
using common::ErrorCode;

MediaControllerServer::MediaControllerServer() : handle_(nullptr) {
  LoggerD("Enter");
}

MediaControllerServer::~MediaControllerServer() {

  LoggerD("Enter");

  if (handle_) {
    int ret;
    ret = mc_server_unset_custom_command_received_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unset command callback, error: " << ret;
    }

    ret = mc_server_destroy(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "mc_server_destroy() failed, error: " << ret;
    }
  }
}

PlatformResult MediaControllerServer::Init() {

  LoggerD("Enter");

  int ret = mc_server_create(&handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "Unable to create media controller server, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unable to create media controller server");
  }

  ret = mc_server_set_custom_command_received_cb(handle_,
                                                 OnCommandReceived,
                                                 this);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "Unable to set command callback, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unable to set command callback");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetPlaybackState(
    const std::string& state) {

  LoggerD("Enter");

  int state_int;
  PlatformResult result = Types::StringToPlatformEnum(
      Types::kMediaControllerPlaybackState, state, &state_int);

  if (!result) {
    return result;
  }

  int ret = mc_server_set_playback_state(
      handle_, static_cast<mc_playback_states_e>(state_int));
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_set_playback_state failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error setting playback state");
  }

  ret = mc_server_update_playback_info(handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_playback_info failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error updating playback info");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetPlaybackPosition(double position) {

  LoggerD("Enter");

  int ret = mc_server_set_playback_position(
      handle_, static_cast<unsigned long long>(position));
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_set_playback_position failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error setting playback position");
  }

  ret = mc_server_update_playback_info(handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_playback_info failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error updating playback info");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetShuffleMode(bool mode) {

  LoggerD("Enter");

  int ret = mc_server_update_shuffle_mode(handle_,
                                          mode ? MC_SHUFFLE_MODE_ON
                                               : MC_SHUFFLE_MODE_OFF);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_shuffle_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error updating shuffle mode");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetRepeatMode(bool mode) {

  LoggerD("Enter");

  int ret = mc_server_update_repeat_mode(handle_,
                                         mode ? MC_REPEAT_MODE_ON
                                              : MC_REPEAT_MODE_OFF);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_repeat_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error updating repeat mode");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetMetadata(
    const picojson::object& metadata) {

  LoggerD("Enter");

  int attribute_int, ret;
  for (picojson::object::const_iterator i = metadata.begin();
       i != metadata.end();
       ++i) {

    PlatformResult result = Types::StringToPlatformEnum(
        Types::kMediaControllerMetadataAttribute, i->first, &attribute_int);
    if (!result) {
      return result;
    }

    ret = mc_server_set_metadata(handle_, static_cast<mc_meta_e>(attribute_int),
                                 i->second.to_str().c_str());
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "set_metadata failed for '" << i->first
                    << "', error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error setting metadata");
    }
  }

  ret = mc_server_update_metadata(handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_metadata failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error updating metadata");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void MediaControllerServer::OnCommandReceived(const char* client_name,
                                              const char* command,
                                              bundle* bundle,
                                              void* user_data) {

  LoggerD("Enter");

  MediaControllerServer* server = static_cast<MediaControllerServer*>(user_data);

  int ret;
  char* data_str = nullptr;
  char* reply_id_str = nullptr;
  SCOPE_EXIT {
    free(data_str);
    free(reply_id_str);
  };

  ret = bundle_get_str(bundle, "data", &data_str);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "bundle_get_str(data) failed, error: " << ret;
    return;
  }

  ret = bundle_get_str(bundle, "replyId", &reply_id_str);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "bundle_get_str(replyId) failed, error: " << ret;
    return;
  }

  picojson::value data;
  std::string err;
  picojson::parse(data, data_str, data_str + strlen(data_str), &err);
  if (!err.empty()) {
    LOGGER(ERROR) << "Failed to parse bundle data: " << err;
    return;
  }

  // TODO(r.galka) CAPI have no dedicated methods for position/shuffle/repeat change.
  // It should be updated when new version of CAPI will be available.
  // For now implementation is using internal commands.
  if (command == kInternalCommandSendPlaybackPosition) {
    double position = data.get("position").get<double>();
    server->SetPlaybackPosition(position);
    server->OnPlaybackPositionCommand(client_name,
                                      static_cast<unsigned long long>(position),
                                      server);
    server->CommandReply(client_name, reply_id_str, data);
    return;
  }
  if (command == kInternalCommandSendShuffleMode) {
    bool mode = data.get("mode").get<bool>();
    server->SetShuffleMode(mode);
    server->OnShuffleModeCommand(client_name,
                                 mode ? MC_SHUFFLE_MODE_ON : MC_SHUFFLE_MODE_OFF,
                                 server);
    server->CommandReply(client_name, reply_id_str, data);
    return;
  }
  if (command == kInternalCommandSendRepeatMode) {
    bool mode = data.get("mode").get<bool>();
    server->SetRepeatMode(mode);
    server->OnRepeatModeCommand(client_name,
                                mode ? MC_REPEAT_MODE_ON : MC_REPEAT_MODE_OFF,
                                server);
    server->CommandReply(client_name, reply_id_str, data);
    return;
  }

  if (server->command_listener_) {
    picojson::value request = picojson::value(picojson::object());
    picojson::object& request_o = request.get<picojson::object>();

    request_o["clientName"] = picojson::value(std::string(client_name));
    request_o["command"] = picojson::value(std::string(command));
    request_o["replyId"] = picojson::value(std::string(reply_id_str));
    request_o["data"] = data;

    server->command_listener_(&request);
  }
}

PlatformResult MediaControllerServer::CommandReply(
    const std::string& client_name,
    const std::string& reply_id,
    const picojson::value& data) {

  LoggerD("Enter");

  int ret;

  bundle* bundle = bundle_create();
  SCOPE_EXIT {
    bundle_free(bundle);
  };

  ret = bundle_add(bundle, "replyId", reply_id.c_str());
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "bundle_add(replyId) failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unable to add replyId to bundle");
  }

  ret = bundle_add(bundle, "data", data.serialize().c_str());
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "bundle_add(data) failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unable to add data to bundle");
  }

  ret = mc_server_send_command_reply(handle_, client_name.c_str(), 0, bundle);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_send_command_reply failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error sending command reply");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetChangeRequestPlaybackInfoListener(
    JsonCallback callback) {

  LoggerD("Enter");

  if (callback && change_request_playback_info_listener_) {
    LOGGER(ERROR) << "Listener already registered";
    return PlatformResult(ErrorCode::INVALID_STATE_ERR,
                          "Listener already registered");
  }

  change_request_playback_info_listener_ = callback;

  int ret;
  if (callback) { // set platform callbacks
    ret = mc_server_set_playback_state_command_received_cb(
        handle_, OnPlaybackStateCommand, this);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to set playback state command listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to set playback state command listener");
    }
  } else { // unset platform callbacks
    ret = mc_server_unset_playback_state_command_received_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unset playback state command listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to unset playback state command listener");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void MediaControllerServer::OnPlaybackStateCommand(const char* client_name,
                                                   mc_playback_states_e state_e,
                                                   void *user_data) {

  LoggerD("Enter");

  MediaControllerServer* server = static_cast<MediaControllerServer*>(user_data);

  if (!server->change_request_playback_info_listener_) {
    LOGGER(DEBUG) << "No change request playback info listener registered, skipping";
    return;
  }

  std::string state;
  PlatformResult result = Types::PlatformEnumToString(
      Types::kMediaControllerPlaybackState,
      static_cast<int>(state_e), &state);
  if (!result) {
    LOGGER(ERROR) << "PlatformEnumToString failed, error: " << result.message();
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onplaybackstaterequest"));
  data_o["state"] = picojson::value(state);

  server->change_request_playback_info_listener_(&data);
}

void MediaControllerServer::OnPlaybackPositionCommand(
    const char* client_name,
    unsigned long long position,
    void* user_data) {

  LoggerD("Enter");

  MediaControllerServer* server = static_cast<MediaControllerServer*>(user_data);

  if (!server->change_request_playback_info_listener_) {
    LOGGER(DEBUG) << "No change request playback info listener registered, skipping";
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onplaybackpositionrequest"));
  data_o["position"] = picojson::value(static_cast<double>(position));

  server->change_request_playback_info_listener_(&data);
}

void MediaControllerServer::OnShuffleModeCommand(const char* client_name,
                                                 mc_shuffle_mode_e mode,
                                                 void* user_data) {

  LoggerD("Enter");

  MediaControllerServer* server = static_cast<MediaControllerServer*>(user_data);

  if (!server->change_request_playback_info_listener_) {
    LOGGER(DEBUG) << "No change request playback info listener registered, skipping";
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onshufflemoderequest"));
  data_o["mode"] = picojson::value(mode == MC_SHUFFLE_MODE_ON);

  server->change_request_playback_info_listener_(&data);
}

void MediaControllerServer::OnRepeatModeCommand(const char* client_name,
                                                mc_repeat_mode_e mode,
                                                void* user_data) {

  LoggerD("Enter");

  MediaControllerServer* server = static_cast<MediaControllerServer*>(user_data);

  if (!server->change_request_playback_info_listener_) {
    LOGGER(DEBUG) << "No change request playback info listener registered, skipping";
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onrepeatmoderequest"));
  data_o["mode"] = picojson::value(mode == MC_REPEAT_MODE_ON);

  server->change_request_playback_info_listener_(&data);
}

} // namespace mediacontroller
} // namespace extension
