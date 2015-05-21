// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVCHANNEL_TVCHANNEL_EXTENSION_H_
#define SRC_TVCHANNEL_TVCHANNEL_EXTENSION_H_

#include "common/extension.h"

class TVChannelExtension: public common::Extension {
 public:
    TVChannelExtension();
    virtual ~TVChannelExtension();
 private:
    virtual common::Instance* CreateInstance();
};

#endif  // SRC_TVCHANNEL_TVCHANNEL_EXTENSION_H_
