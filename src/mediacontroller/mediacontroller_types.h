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
 
#ifndef MEDIACONTROLLER_MEDIACONTROLLER_TYPES_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_TYPES_H_

#include <functional>
#include <map>
#include <media_controller_type.h>
#include <string>

#include "common/platform_result.h"

namespace extension {
namespace mediacontroller {

typedef std::map<std::string, std::map<std::string, int>> PlatformEnumMap;
typedef std::map<std::string, std::map<int, std::string>> PlatformEnumReverseMap;
typedef std::function<void(picojson::value*)> JsonCallback;

class Types {
 public:
  static const std::string kMediaControllerServerState;
  static const std::string kMediaControllerPlaybackState;
  static const std::string kMediaControllerMetadataAttribute;

  static common::PlatformResult GetPlatformEnumMap(
      const std::string& type, std::map<std::string, int>* platform_str);

  static common::PlatformResult StringToPlatformEnum(const std::string& type,
                                                     const std::string& value,
                                                     int* platform_enum);

  static common::PlatformResult PlatformEnumToString(const std::string& type,
                                                     int value,
                                                     std::string* platform_str);

  static common::PlatformResult ConvertPlaybackState(mc_playback_h playback_h,
                                                     std::string* state);
  static common::PlatformResult ConvertPlaybackPosition(mc_playback_h playback_h,
                                                        double* position);
  static common::PlatformResult ConvertMetadata(mc_metadata_h metadata_h,
                                                picojson::object* metadata);

 private:
  static const PlatformEnumMap platform_enum_map_;
  // TODO(r.galka) can be replaced by Boost.Bimap
  static PlatformEnumReverseMap platform_enum_reverse_map_;
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_TYPES_H_
