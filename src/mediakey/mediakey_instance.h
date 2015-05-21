// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_MEDIAKEY_MEDIAKEY_INSTANCE_H_
#define SRC_MEDIAKEY_MEDIAKEY_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "common/picojson.h"

#include "mediakey/mediakey_manager.h"

namespace extension {
namespace mediakey {

class MediaKeyInstance : public common::ParsedInstance, public MediaKeyListener {
 public:
  MediaKeyInstance();
  virtual ~MediaKeyInstance();

 private:
  void SetMediaKeyEventListener(const picojson::value& args,
                                picojson::object& out);
  void UnsetMediaKeyEventListener(const picojson::value& args,
                                  picojson::object& out);
  virtual void OnPressedMediaKeyEventCallback(media_key_e type);
  virtual void OnReleasedMediaKeyEventCallback(media_key_e type);
  void PostEvent(const std::string& event, media_key_e type);
};

}  // namespace mediakey
}  // namespace extension

#endif  // SRC_MEDIAKEY_MEDIAKEY_INSTANCE_H_
