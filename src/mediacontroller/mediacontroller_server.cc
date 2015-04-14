// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_server.h"

#include "common/logger.h"

#include "mediacontroller/mediacontroller_types.h"

namespace extension {
namespace mediacontroller {

using common::PlatformResult;
using common::ErrorCode;

MediaControllerServer::MediaControllerServer() : handle_(nullptr) {
}

MediaControllerServer::~MediaControllerServer() {
  LOGGER(DEBUG) << "entered";

  if (handle_) {
    int ret = mc_server_destroy(handle_);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "Unable to destroy media controller server";
    }
  }
}

common::PlatformResult MediaControllerServer::Init() {
  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

  int ret = mc_server_create(&handle_);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "Unable to create media controller server, error: " << ret;
    result = PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Unable to create media controller server");
  }

  return result;
}

common::PlatformResult MediaControllerServer::SetPlaybackState(
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

common::PlatformResult MediaControllerServer::SetPlaybackPosition(
    double position) {

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

common::PlatformResult MediaControllerServer::SetShuffleMode(bool mode) {

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

common::PlatformResult MediaControllerServer::SetRepeatMode(bool mode) {

  int ret = mc_server_update_repeat_mode(handle_,
                                         mode ? REPEAT_MODE_ON
                                              : REPEAT_MODE_OFF);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_server_update_repeat_mode failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Error updating repeat mode");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult MediaControllerServer::SetMetadata(
    const picojson::object& metadata) {

  int attribute_int, ret;
  PlatformResult result(ErrorCode::NO_ERROR);
  for (picojson::object::const_iterator i = metadata.begin();
       i != metadata.end();
       ++i) {

    result = Types::StringToPlatformEnum(
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

  return result;
}

} // namespace mediacontroller
} // namespace extension
