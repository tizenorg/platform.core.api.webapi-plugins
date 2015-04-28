// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_client.h"

#include "common/logger.h"
#include "common/scope_exit.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

using common::PlatformResult;
using common::ErrorCode;

MediaControllerClient::MediaControllerClient() : handle_(nullptr) {
}

MediaControllerClient::~MediaControllerClient() {
  LOGGER(DEBUG) << "entered";

  if (handle_) {
    int ret = mc_client_destroy(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to destroy media controller client";
    }
  }
}

PlatformResult MediaControllerClient::Init() {
  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

  int ret = mc_client_create(&handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "Unable to create media controller client, error: " << ret;
    result = PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to create media controller client");
  }

  return result;
}

PlatformResult MediaControllerClient::FindServers(picojson::array* servers) {
  LOGGER(DEBUG) << "entered";

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
  for (picojson::array::iterator it; it != servers->end(); ++it) {
    picojson::object& server = it->get<picojson::object>();
    if (it->get("name").get<std::string>() == latest_name) {
      server["state"] = latest_server.get("state");
      break;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

bool MediaControllerClient::FindServersCallback(const char* server_name,
                                                void* user_data) {
  LOGGER(DEBUG) << "entered";

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

  int ret;

  char* name;
  mc_server_state_e state;
  ret = mc_client_get_latest_server_info(handle_, &name, &state);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_latest_server_info failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting latest server info");
  }

  if (NULL == name) {
    LOGGER(DEBUG) << "No active server available";
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  SCOPE_EXIT {
    free(name);
  };

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
  mc_playback_states_e state;
  ret = mc_client_get_playback_state(playback_h, &state);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_playback_state failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting playback state");
  }
  if (state == MEDIA_PLAYBACK_STATE_NONE) {
    state = MEDIA_PLAYBACK_STATE_STOPPED;
  }

  std::string state_str;
  PlatformResult result = Types::PlatformEnumToString(
      Types::kMediaControllerPlaybackState,
      static_cast<int>(state), &state_str);
  if (!result) {
    LOGGER(ERROR) << "PlatformEnumToString failed, error: " << result.message();
    return result;
  }

  // playback position
  unsigned long long position;
  ret = mc_client_get_playback_position(playback_h, &position);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_playback_position failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting playback position");
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
  (*playback_info)["state"] = picojson::value(state_str);
  (*playback_info)["position"] = picojson::value(static_cast<double>(position));
  (*playback_info)["shuffleMode"] = picojson::value(shuffle == SHUFFLE_MODE_ON);
  (*playback_info)["repeatMode"] = picojson::value(repeat == REPEAT_MODE_ON);
  (*playback_info)["metadata"] = metadata;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult MediaControllerClient::GetMetadata(
    const std::string& server_name,
    picojson::object* metadata) {
  LOGGER(DEBUG) << "entered";

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

  std::map<std::string, int> metadata_fields;
  PlatformResult result = Types::GetPlatformEnumMap(
      Types::kMediaControllerMetadataAttribute, &metadata_fields);
  if (!result) {
    LOGGER(ERROR) << "GetPlatformEnumMap failed, error: " << result.message();
    return result;
  }

  char* value = nullptr;
  SCOPE_EXIT {
    free(value);
  };
  for (auto& field : metadata_fields) {
    ret = mc_client_get_metadata(metadata_h,
                                 static_cast<mc_meta_e>(field.second),
                                 &value);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "mc_client_get_metadata failed for field '"
          << field.first << "', error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Error getting metadata");
    }
    if (!value) {
      value = strdup("");
    }

    (*metadata)[field.first] = picojson::value(std::string(value));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

} // namespace mediacontroller
} // namespace extension
