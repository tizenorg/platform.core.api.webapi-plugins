// Copyright (c) 2014 Samsung Electronics Co., Ltd. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBSETTING_WEBSETTING_INSTANCE_H_
#define WEBSETTING_WEBSETTING_INSTANCE_H_

#include "common/extension.h"
#include "common/picojson.h"
#include "tizen/tizen.h"

#include "websetting/websetting_extension.h"

namespace extension {
namespace websetting {

class WebSettingInstance : public common::ParsedInstance {
 public:
  explicit WebSettingInstance(WebSettingExtension* extension);
  virtual ~WebSettingInstance();

 private:
  void setUserAgentString(const picojson::value& args, picojson::object& out);
  void removeAllCookies(const picojson::value& args, picojson::object& out);

  WebSettingExtension* extension_;
};

}  // namespace websetting
}  // namespace extension

#endif  // WEBSETTING_WEBSETTING_INSTANCE_H_
