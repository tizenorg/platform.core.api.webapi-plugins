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
 
#include "mediacontroller/mediacontroller_types.h"

#include <media_controller_client.h>

#include "common/logger.h"
#include "common/platform_result.h"
#include "common/scope_exit.h"

namespace extension {
namespace mediacontroller {

using common::PlatformResult;
using common::ErrorCode;

const std::string Types::kMediaControllerServerState
    = "MediaControllerServerState";
const std::string Types::kMediaControllerPlaybackState
    = "MediaControllerPlaybackState";
const std::string Types::kMediaControllerMetadataAttribute
    = "MediaControllerMetadataAttribute";

const PlatformEnumMap Types::platform_enum_map_ = {
    {kMediaControllerServerState, {
        {"ACTIVE", MC_SERVER_STATE_ACTIVATE},
        {"INACTIVE", MC_SERVER_STATE_DEACTIVATE}}},
    {kMediaControllerPlaybackState, {
        {"PLAY", MC_PLAYBACK_STATE_PLAYING},
        {"PAUSE", MC_PLAYBACK_STATE_PAUSED},
        {"STOP", MC_PLAYBACK_STATE_STOPPED},
        {"NEXT", MC_PLAYBACK_STATE_NEXT_FILE},
        {"PREV", MC_PLAYBACK_STATE_PREV_FILE},
        {"FORWARD", MC_PLAYBACK_STATE_FAST_FORWARD},
        {"REWIND", MC_PLAYBACK_STATE_REWIND}}},
    {kMediaControllerMetadataAttribute, {
        {"title", MC_META_MEDIA_TITLE},
        {"artist", MC_META_MEDIA_ARTIST},
        {"album", MC_META_MEDIA_ALBUM},
        {"author", MC_META_MEDIA_AUTHOR},
        {"genre", MC_META_MEDIA_GENRE},
        {"duration", MC_META_MEDIA_DURATION},
        {"date", MC_META_MEDIA_DATE},
        {"copyright", MC_META_MEDIA_COPYRIGHT},
        {"description", MC_META_MEDIA_DESCRIPTION},
        {"trackNum", MC_META_MEDIA_TRACK_NUM},
        {"picture", MC_META_MEDIA_PICTURE}
    }}
};

PlatformEnumReverseMap Types::platform_enum_reverse_map_ = {};

PlatformResult Types::GetPlatformEnumMap(const std::string& type,
                                  std::map<std::string, int>* enum_map) {

  LoggerD("Enter");

  auto iter = platform_enum_map_.find(type);
  if (iter == platform_enum_map_.end()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          std::string("Undefined platform enum type ") + type);
  }

  *enum_map = platform_enum_map_.at(type);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Types::StringToPlatformEnum(const std::string& type,
                                           const std::string& value,
                                           int* platform_enum) {

  LoggerD("Enter");

  std::map<std::string, int> def;
  PlatformResult result = GetPlatformEnumMap(type, &def);
  if (!result) {
    return result;
  }

  auto def_iter = def.find(value);
  if (def_iter != def.end()) {
    *platform_enum = def_iter->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::string message =
      "Platform enum value " + value + " not found for " + type;
  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

PlatformResult Types::PlatformEnumToString(const std::string& type,
                                           int value,
                                           std::string* platform_str) {
  LoggerD("Enter");

  // TODO(r.galka) can be replaced by Boost.Bimap
  if (platform_enum_reverse_map_.empty()) {
    for (auto& def : platform_enum_map_) {
      platform_enum_reverse_map_[def.first] = {};

      for (auto& key : def.second) {
        platform_enum_reverse_map_[def.first][key.second] = key.first;
      }
    }
  }

  auto it = platform_enum_reverse_map_.find(type);
  if (it == platform_enum_reverse_map_.end()) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
        std::string("Undefined platform enum type ") + type);
  }

  auto def = platform_enum_reverse_map_.at(type);
  auto def_it = def.find(value);
  if (def_it != def.end()) {
    *platform_str = def_it->second;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  std::string message = "Platform enum value " + std::to_string(value) +
      " not found for " + type;
  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

PlatformResult Types::ConvertPlaybackState(mc_playback_h playback_h,
                                           std::string* state) {
  LoggerD("Enter");

  int ret;
  mc_playback_states_e state_e;
  ret = mc_client_get_playback_state(playback_h, &state_e);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_playback_state failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting playback state");
  }
  if (state_e == MC_PLAYBACK_STATE_NONE) {
    state_e = MC_PLAYBACK_STATE_STOPPED;
  }

  PlatformResult result = Types::PlatformEnumToString(
      Types::kMediaControllerPlaybackState,
      static_cast<int>(state_e), state);
  if (!result) {
    LOGGER(ERROR) << "PlatformEnumToString failed, error: " << result.message();
    return result;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Types::ConvertPlaybackPosition(mc_playback_h playback_h,
                                              double* position) {
  LoggerD("Enter");

  int ret;

  unsigned long long pos;
  ret = mc_client_get_playback_position(playback_h, &pos);
  if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
    LOGGER(ERROR) << "mc_client_get_playback_position failed, error: " << ret;
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error getting playback position");
  }

  *position = static_cast<double>(pos);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult Types::ConvertMetadata(mc_metadata_h metadata_h,
                                      picojson::object* metadata) {
  LoggerD("Enter");

  std::map<std::string, int> metadata_fields;
  PlatformResult result = GetPlatformEnumMap(
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
    int ret = mc_client_get_metadata(metadata_h,
                                 static_cast<mc_meta_e>(field.second),
                                 &value);
    if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
      LOGGER(ERROR) << "mc_client_get_metadata failed for field '"
          << field.first << "', error: " << ret;
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Error getting metadata");
    }

    (*metadata)[field.first] = picojson::value(std::string(value ? value : ""));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

} // namespace mediacontroller
} // namespace extension
