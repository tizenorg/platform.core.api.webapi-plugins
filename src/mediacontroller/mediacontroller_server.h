// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_

#include <media_controller_server.h>
#include <string>

#include "common/platform_result.h"

namespace extension {
namespace mediacontroller {

class MediaControllerServer {
 public:
  MediaControllerServer();
  virtual ~MediaControllerServer();

  common::PlatformResult Init();
  common::PlatformResult SetPlaybackState(const std::string& state);
  common::PlatformResult SetPlaybackPosition(double position);
  common::PlatformResult SetShuffleMode(bool mode);
  common::PlatformResult SetRepeatMode(bool mode);
  common::PlatformResult SetMetadata(const picojson::object& metadata);

 private:
  mc_server_h handle_;
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_SERVER_H_
