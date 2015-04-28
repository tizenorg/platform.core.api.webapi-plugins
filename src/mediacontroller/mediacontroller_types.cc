// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mediacontroller/mediacontroller_types.h"

#include "common/platform_result.h"

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
        {"PLAY", MEDIA_PLAYBACK_STATE_PLAYING},
        {"PAUSE", MEDIA_PLAYBACK_STATE_PAUSED},
        {"STOP", MEDIA_PLAYBACK_STATE_STOPPED},
        {"NEXT", MEDIA_PLAYBACK_STATE_NEXT_FILE},
        {"PREV", MEDIA_PLAYBACK_STATE_PREV_FILE},
        {"FORWARD", MEDIA_PLAYBACK_STATE_FAST_FORWARD},
        {"REWIND", MEDIA_PLAYBACK_STATE_REWIND}}},
    {kMediaControllerMetadataAttribute, {
        {"title", MEDIA_TITLE},
        {"artist", MEDIA_ARTIST},
        {"album", MEDIA_ALBUM},
        {"author", MEDIA_AUTHOR},
        {"genre", MEDIA_GENRE},
        {"duration", MEDIA_DURATION},
        {"date", MEDIA_DATE},
        {"copyright", MEDIA_COPYRIGHT},
        {"description", MEDIA_DESCRIPTION},
        {"trackNum", MEDIA_TRACK_NUM},
        {"picture", MEDIA_PICTURE}
    }}
};

PlatformEnumReverseMap Types::platform_enum_reverse_map_ = {};

PlatformResult Types::GetPlatformEnumMap(const std::string& type,
                                  std::map<std::string, int>* enum_map) {

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

} // namespace mediacontroller
} // namespace extension
