// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIACONTROLLER_MEDIACONTROLLER_CLIENT_H_
#define MEDIACONTROLLER_MEDIACONTROLLER_CLIENT_H_

#include <media_controller_client.h>
#include <string>

#include "common/platform_result.h"

namespace extension {
namespace mediacontroller {

class MediaControllerClient {
 public:
  MediaControllerClient();
  virtual ~MediaControllerClient();

  common::PlatformResult Init();
  common::PlatformResult FindServers(picojson::array* servers);
  common::PlatformResult GetLatestServerInfo(picojson::value* server_info);

 private:
  mc_client_h handle_;

  static bool FindServersCallback(const char* server_name, void* user_data);
};

} // namespace mediacontroller
} // namespace extension

#endif  // MEDIACONTROLLER_MEDIACONTROLLER_CLIENT_H_
