// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_server.h"

#include <bundle.h>

#include "common/logger.h"
#include "common/scope_exit.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

using common::PlatformResult;
using common::ErrorCode;

MediaControllerServer::MediaControllerServer() : handle_(nullptr) {
}

MediaControllerServer::~MediaControllerServer() {

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

  int ret = mc_server_update_shuffle_mode(handle_,
                                          mode ? SHUFFLE_MODE_ON
                                               : SHUFFLE_MODE_OFF);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_shuffle_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error updating shuffle mode");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetRepeatMode(bool mode) {

  int ret = mc_server_update_repeat_mode(handle_,
                                         mode ? REPEAT_MODE_ON
                                              : REPEAT_MODE_OFF);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_repeat_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error updating repeat mode");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetMetadata(
    const picojson::object& metadata) {

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
  LOGGER(DEBUG) << "entered";

  MediaControllerServer* server = static_cast<MediaControllerServer*>(user_data);

  if (server->command_listener_) {
    picojson::value request = picojson::value(picojson::object());
    picojson::object& request_o = request.get<picojson::object>();

    int ret;
    char* reply_id_str = nullptr;
    char* data_str = nullptr;
    SCOPE_EXIT {
      free(reply_id_str);
      free(data_str);
    };

    ret = bundle_get_str(bundle, "replyId", &reply_id_str);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "bundle_get_str(replyId) failed, error: " << ret;
      return;
    }

    ret = bundle_get_str(bundle, "data", &data_str);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "bundle_get_str(data) failed, error: " << ret;
      return;
    }

    picojson::value data;
    std::string err;
    picojson::parse(data, data_str, data_str + strlen(data_str), &err);
    if (!err.empty()) {
      LOGGER(ERROR) << "Failed to parse bundle data: " << err;
      return;
    }

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
  LOGGER(DEBUG) << "entered";

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

  ret = mc_server_send_command_reply(handle_, client_name.c_str(), 0, bundle, NULL);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_send_command_reply failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error sending command reply");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerServer::SetChangeRequestPlaybackInfoListener(
    JsonCallback callback) {

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
  LOGGER(DEBUG) << "entered";

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

} // namespace mediacontroller
} // namespace extension
