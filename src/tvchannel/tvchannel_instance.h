// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
#define SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "common/picojson.h"

#include "tvchannel/tvchannel_extension.h"

namespace extension {
namespace tvchannel {

class TVChannelInstance: public common::ParsedInstance {
 public:
    TVChannelInstance();
    virtual ~TVChannelInstance();

 private:
    void getCurrentChannel(const picojson::value& args, picojson::object& out);
};

}  // namespace tvchannel
}  // namespace extension

#endif  // SRC_TVCHANNEL_TVCHANNEL_INSTANCE_H_
