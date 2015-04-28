// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_TYPES_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_TYPES_H_

#include <map>
#include <media_controller_type.h>
#include <string>

#include "common/platform_result.h"

namespace extension {
namespace mediacontroller {

typedef std::map<std::string, std::map<std::string, int>> PlatformEnumMap;
typedef std::map<std::string, std::map<int, std::string>> PlatformEnumReverseMap;

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

 private:
  static const PlatformEnumMap platform_enum_map_;
  // TODO(r.galka) can be replaced by Boost.Bimap
  static PlatformEnumReverseMap platform_enum_reverse_map_;
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_TYPES_H_
