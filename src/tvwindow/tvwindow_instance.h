// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVWINDOW_TVWINDOW_INSTANCE_H_
#define SRC_TVWINDOW_TVWINDOW_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "common/picojson.h"

#include "tvwindow/tvwindow_extension.h"

namespace extension {
namespace tvwindow {

class TVWindowInstance : public common::ParsedInstance {
 public:
  explicit TVWindowInstance(TVWindowExtension const& extension);
  virtual ~TVWindowInstance();

 private:
  virtual void HandleMessage(const char* msg);
  virtual void HandleSyncMessage(const char* msg);
};

}  // namespace tvwindow
}  // namespace extension

#endif  // SRC_TVWINDOW_TVWINDOW_INSTANCE_H_
