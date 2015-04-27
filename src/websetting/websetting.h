// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBSETTING_WEBSETTING_H_
#define WEBSETTING_WEBSETTING_H_

#include <gio/gio.h>
#include <memory>
#include <string>

#include "common/platform_result.h"

class WebSetting {
 public:
  explicit WebSetting(const std::string& app_id);
  ~WebSetting();

  std::string app_id() const { return app_id_; }

  common::PlatformResult RemoveAllCookies();
  common::PlatformResult SetUserAgentString(const std::string& user_agent);

 private:
  std::string app_id_;
  GDBusProxy* running_app_proxy_;
};

#endif  // websetting_websetting_H_
