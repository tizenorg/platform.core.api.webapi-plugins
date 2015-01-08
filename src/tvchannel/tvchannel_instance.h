// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
#define SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "common/picojson.h"

#include "tvchannel/tvchannel_extension.h"

namespace tvchannel {

class TVChannelInstance : public common::Instance {
 public:
  explicit TVChannelInstance(TVChannelExtension const& extension);
  virtual ~TVChannelInstance();

 private:
  virtual void HandleMessage(const char* msg);
  virtual void HandleSyncMessage(const char* msg);
};

}  // namespace tvchannel

#endif  // SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
