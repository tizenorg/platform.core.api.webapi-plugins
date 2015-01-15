// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBSETTING_WEBSETTING_EXTENSION_H_
#define WEBSETTING_WEBSETTING_EXTENSION_H_

#include <memory>
#include <string>
#include "common/extension.h"
#include "websetting/websetting.h"

class WebSettingExtension : public common::Extension {
 public:
  explicit WebSettingExtension(const std::string& app_id);
  virtual ~WebSettingExtension();

  WebSetting* current_app() { return current_app_.get(); }
 private:
  virtual common::Instance* CreateInstance();

  std::unique_ptr<WebSetting> current_app_;
};

#endif  // WEBSETTING_WEBSETTING_EXTENSION_H_
