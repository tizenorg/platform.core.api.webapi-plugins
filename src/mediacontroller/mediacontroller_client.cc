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

} // namespace mediacontroller
} // namespace extension
