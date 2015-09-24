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
 
#include "mediacontroller/mediacontroller_client.h"

#include <bundle.h>
#include <bundle_internal.h>
#include <memory>

#include "common/logger.h"
#include "common/scope_exit.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

using common::PlatformResult;
using common::ErrorCode;

MediaControllerClient::MediaControllerClient() : handle_(nullptr) {
  LoggerD("Enter");
}

MediaControllerClient::~MediaControllerClient() {
  LoggerD("Enter");
  if (handle_) {
    int ret = mc_client_destroy(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to destroy media controller client";
    }
  }
}

PlatformResult MediaControllerClient::Init() {
  LoggerD("Enter");
  int ret = mc_client_create(&handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "Unable to create media controller client, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unable to create media controller client");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerClient::FindServers(picojson::array* servers) {

  LoggerD("Enter");
  int ret;

  ret = mc_client_foreach_server(handle_, FindServersCallback, servers);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "Unable to fetch active servers, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unable to create media controller client");
  }

  // check latest server state - if exist
  picojson::value latest_server = picojson::value();
  PlatformResult result = GetLatestServerInfo(&latest_server);
  if (!result) {
    LOGGER(ERROR) << "GetLatestServerInfo failed, error: " << result.message();
    return result;
  }

  if (latest_server.is<picojson::null>()) {
    LOGGER(DEBUG) << "No latest server available";
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  const std::string& latest_name = latest_server.get("name").get<std::string>();

  // update current server state in list
  for (auto& it : *servers) {
    picojson::object& server = it.get<picojson::object>();
    if (server["name"].get<std::string>() == latest_name) {
      server["state"] = latest_server.get("state");
      break;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

bool MediaControllerClient::FindServersCallback(const char* server_name,
                                                void* user_data) {

  LoggerD("Enter");
  picojson::array* servers = static_cast<picojson::array*>(user_data);

  picojson::value server = picojson::value(picojson::object());
  picojson::object& server_o = server.get<picojson::object>();
  server_o["name"] = picojson::value(std::string(server_name));
  // active by default in CAPI
  server_o["state"] = picojson::value(std::string("ACTIVE"));

  servers->push_back(server);

  return true;
}

PlatformResult MediaControllerClient::GetLatestServerInfo(
    picojson::value* server_info) {

  LoggerD("Enter");
  int ret;

  char* name = nullptr;
  SCOPE_EXIT {
    free(name);
  };
  mc_server_state_e state;
  ret = mc_client_get_latest_server_info(handle_, &name, &state);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_latest_server_info failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting latest server info");
  }

  if (!name) {
    LOGGER(DEBUG) << "No active server available";
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::string state_str;
  PlatformResult result = Types::PlatformEnumToString(
      Types::kMediaControllerServerState, static_cast<int>(state), &state_str);
  if (!result) {
    LOGGER(ERROR) << "PlatformEnumToString failed, error: " << result.message();
    return result;
  }

  *server_info = picojson::value(picojson::object());
  picojson::object& obj = server_info->get<picojson::object>();
  obj["name"] = picojson::value(std::string(name));
  obj["state"] = picojson::value(state_str);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerClient::GetPlaybackInfo(
    const std::string& server_name,
    picojson::object* playback_info) {

  LoggerD("Enter");
  int ret;

  mc_playback_h playback_h;
  ret = mc_client_get_server_playback_info(handle_,
                                           server_name.c_str(),
                                           &playback_h);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_latest_server_info failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting latest server info");
  }

  SCOPE_EXIT {
    mc_client_destroy_playback(playback_h);
  };

  // playback state
  std::string state;
  PlatformResult result = Types::ConvertPlaybackState(playback_h, &state);
  if (!result) {
    LOGGER(ERROR) << "ConvertPlaybackState failed, error: " << result.message();
    return result;
  }

  // playback position
  double position;
  result = Types::ConvertPlaybackPosition(playback_h, &position);
  if (!result) {
    LOGGER(ERROR) << "ConvertPlaybackPosition failed, error: " << result.message();
    return result;
  }

  // shuffle mode
  mc_shuffle_mode_e shuffle;
  ret = mc_client_get_server_shuffle_mode(
      handle_, server_name.c_str(), &shuffle);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_server_shuffle_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting shuffle mode");
  }

  // repeat mode
  mc_repeat_mode_e repeat;
  ret = mc_client_get_server_repeat_mode(
      handle_, server_name.c_str(), &repeat);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_server_repeat_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting repeat mode");
  }

  // metadata
  picojson::value metadata = picojson::value(picojson::object());
  result = GetMetadata(server_name, &metadata.get<picojson::object>());
  if (!result) {
    return result;
  }

  // fill return object
  (*playback_info)["state"] = picojson::value(state);
  (*playback_info)["position"] = picojson::value(position);
  (*playback_info)["shuffleMode"] = picojson::value(shuffle == MC_SHUFFLE_MODE_ON);
  (*playback_info)["repeatMode"] = picojson::value(repeat == MC_REPEAT_MODE_ON);
  (*playback_info)["metadata"] = metadata;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerClient::GetMetadata(
    const std::string& server_name,
    picojson::object* metadata) {

  LoggerD("Enter");
  int ret;

  mc_metadata_h metadata_h;
  ret = mc_client_get_server_metadata(handle_, server_name.c_str(),
                                      &metadata_h);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_server_metadata failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting metadata");
  }

  SCOPE_EXIT {
    mc_client_destroy_metadata(metadata_h);
  };

  PlatformResult result = Types::ConvertMetadata(metadata_h, metadata);
  if (!result) {
    return result;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerClient::SetServerStatusChangeListener(
    JsonCallback callback) {

  LoggerD("Enter");
  if (callback && server_status_listener_) {
    LOGGER(ERROR) << "Listener already registered";
    return PlatformResult(ErrorCode::INVALID_STATE_ERR,
                          "Listener already registered");
  }

  server_status_listener_ = callback;

  int ret;
  if (callback) { // set platform callbacks

    ret = mc_client_set_server_update_cb(handle_, OnServerStatusUpdate, this);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to set server status listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to set server status listener");
    }

  } else { // unset platform callbacks

    ret = mc_client_unset_server_update_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unset server status listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to unset server status listener");
    }

  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void MediaControllerClient::OnServerStatusUpdate(const char* server_name,
                                                 mc_server_state_e state,
                                                 void* user_data) {

  LoggerD("Enter");
  MediaControllerClient* client = static_cast<MediaControllerClient*>(user_data);

  if (!client->server_status_listener_) {
    LOGGER(DEBUG) << "No server status listener registered, skipping";
    return;
  }

  // server state
  std::string state_str;
  PlatformResult result = Types::PlatformEnumToString(
      Types::kMediaControllerServerState, static_cast<int>(state), &state_str);
  if (!result) {
    LOGGER(ERROR) << "PlatformEnumToString failed, error: " << result.message();
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["state"] = picojson::value(state_str);

  client->server_status_listener_(&data);
}

PlatformResult MediaControllerClient::SetPlaybackInfoListener(
    JsonCallback callback) {

  LoggerD("Enter");
  if (callback && playback_info_listener_) {
    LOGGER(ERROR) << "Listener already registered";
    return PlatformResult(ErrorCode::INVALID_STATE_ERR,
                          "Listener already registered");
  }

  playback_info_listener_ = callback;

  int ret;
  if (callback) { // set platform callbacks

    ret = mc_client_set_playback_update_cb(handle_, OnPlaybackUpdate, this);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to register playback listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to register playback listener");
    }

    ret = mc_client_set_shuffle_mode_update_cb(handle_, OnShuffleModeUpdate, this);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to register shuffle mode listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to register shuffle mode listener");
    }

    ret = mc_client_set_repeat_mode_update_cb(handle_, OnRepeatModeUpdate, this);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to register repeat mode listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to register repeat mode listener");
    }

    ret = mc_client_set_metadata_update_cb(handle_, OnMetadataUpdate, this);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to register metadata listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to register metadata listener");
    }

  } else { // unset platform callbacks

    ret = mc_client_unset_playback_update_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unregister playback listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to unregister playback listener");
    }

    ret = mc_client_unset_shuffle_mode_update_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unregister shuffle mode listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to unregister shuffle mode listener");
    }

    ret = mc_client_unset_repeat_mode_update_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unregister repeat mode listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to unregister repeat mode listener");
    }

    ret = mc_client_unset_metadata_update_cb(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to unregister metadata listener, error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to unregister metadata listener");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
};

void MediaControllerClient::OnPlaybackUpdate(const char *server_name,
                                             mc_playback_h playback,
                                             void *user_data) {

  LoggerD("Enter");
  MediaControllerClient* client = static_cast<MediaControllerClient*>(user_data);

  if (!client->playback_info_listener_) {
    LOGGER(DEBUG) << "No playback info listener registered, skipping";
    return;
  }

  // playback state
  std::string state;
  PlatformResult result = Types::ConvertPlaybackState(playback, &state);
  if (!result) {
    LOGGER(ERROR) << "ConvertPlaybackState failed, error: " << result.message();
    return;
  }

  // playback position
  double position;
  result = Types::ConvertPlaybackPosition(playback, &position);
  if (!result) {
    LOGGER(ERROR) << "ConvertPlaybackPosition failed, error: " << result.message();
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onplaybackchanged"));
  data_o["state"] = picojson::value(state);
  data_o["position"] = picojson::value(position);

  client->playback_info_listener_(&data);
}

void MediaControllerClient::OnShuffleModeUpdate(const char *server_name,
                                             mc_shuffle_mode_e mode,
                                             void *user_data) {

  LoggerD("Enter");
  MediaControllerClient* client = static_cast<MediaControllerClient*>(user_data);

  if (!client->playback_info_listener_) {
    LOGGER(DEBUG) << "No playback info listener registered, skipping";
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onshufflemodechanged"));
  data_o["mode"] = picojson::value(mode == MC_SHUFFLE_MODE_ON);

  client->playback_info_listener_(&data);
}

void MediaControllerClient::OnRepeatModeUpdate(const char *server_name,
                                                mc_repeat_mode_e mode,
                                                void *user_data) {

  LoggerD("Enter");
  MediaControllerClient* client = static_cast<MediaControllerClient*>(user_data);

  if (!client->playback_info_listener_) {
    LOGGER(DEBUG) << "No playback info listener registered, skipping";
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  data_o["action"] = picojson::value(std::string("onrepeatmodechanged"));
  data_o["mode"] = picojson::value(mode == MC_REPEAT_MODE_ON);

  client->playback_info_listener_(&data);
}

void MediaControllerClient::OnMetadataUpdate(const char* server_name,
                                             mc_metadata_h metadata_h,
                                             void* user_data) {

  LoggerD("Enter");
  MediaControllerClient* client = static_cast<MediaControllerClient*>(user_data);

  if (!client->playback_info_listener_) {
    LOGGER(DEBUG) << "No playback info listener registered, skipping";
    return;
  }

  picojson::value data = picojson::value(picojson::object());
  picojson::object& data_o = data.get<picojson::object>();

  picojson::value metadata = picojson::value(picojson::object());
  PlatformResult result = Types::ConvertMetadata(
      metadata_h, &metadata.get<picojson::object>());
  if (!result) {
    LOGGER(ERROR) << "ConvertMetadata failed, error: " << result.message();
    return;
  }

  data_o["action"] = picojson::value(std::string("onmetadatachanged"));
  data_o["metadata"] = metadata;

  client->playback_info_listener_(&data);
}

PlatformResult MediaControllerClient::SendCommand(
    const std::string& server_name,
    const std::string& command,
    const picojson::value& data,
    const std::string& reply_id,
    const JsonCallback& reply_cb) {

  LoggerD("Enter");
  bundle* bundle = bundle_create();
  SCOPE_EXIT {
    bundle_free(bundle);
  };

  int ret;
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

  ret = mc_client_send_custom_command(handle_,
                                      server_name.c_str(),
                                      command.c_str(),
                                      bundle,
                                      OnCommandReply,
                                      this);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_send_custom_command failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error sending custom command");
  }

  command_reply_callback_ = reply_cb;

  return PlatformResult(ErrorCode::NO_ERROR);
}

void MediaControllerClient::OnCommandReply(const char* server_name,
                                           int result_code,
                                           bundle* bundle,
                                           void* user_data) {

  LoggerD("Enter");
  MediaControllerClient* client = static_cast<MediaControllerClient*>(user_data);

  picojson::value reply = picojson::value(picojson::object());
  picojson::object& reply_o = reply.get<picojson::object>();

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

  reply_o["replyId"] = picojson::value(std::string(reply_id_str));

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
  reply_o["data"] = data;

  client->command_reply_callback_(&reply);
}

PlatformResult MediaControllerClient::SendPlaybackState(
    const std::string& server_name,
    const std::string& state) {

  LoggerD("Enter");
  int state_e;
  PlatformResult result = Types::StringToPlatformEnum(
      Types::kMediaControllerPlaybackState, state, &state_e);
  if (!result) {
    return result;
  }

  int ret;
  ret = mc_client_send_playback_state_command(
      handle_, server_name.c_str(), static_cast<mc_playback_states_e>(state_e));
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_send_playback_state_command failed, error: "
        << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error sending playback state");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerClient::SendPlaybackPosition(
    const std::string& server_name,
    double position) {

  // TODO(r.galka) implement when dedicated method will be available in CAPI

  return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR);
}

PlatformResult MediaControllerClient::SendShuffleMode(
    const std::string& server_name,
    bool mode) {

  // TODO(r.galka) implement when dedicated method will be available in CAPI

  return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR);
}

PlatformResult MediaControllerClient::SendRepeatMode(
    const std::string& server_name,
    bool mode) {

  // TODO(r.galka) implement when dedicated method will be available in CAPI

  return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR);
}

} // namespace mediacontroller
} // namespace extension
