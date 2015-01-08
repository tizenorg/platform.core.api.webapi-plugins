// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvchannel/tvchannel_instance.h"
#include "common/logger.h"
#include "tizen/tizen.h"
#include "common/picojson.h"

namespace tvchannel {

TVChannelInstance::TVChannelInstance(TVChannelExtension const& extension) {
    LoggerE("Entered");
}

TVChannelInstance::~TVChannelInstance() {
    LoggerE("Entered");
}

void TVChannelInstance::HandleMessage(const char* msg) {
    // this is stub, no async messages
    LoggerE("Entered");
}

void TVChannelInstance::HandleSyncMessage(const char* msg) {
    LoggerE("Entered %s", msg);
    picojson::object answer;
    answer["answer"] = picojson::value(true);

    SendSyncReply(picojson::value(answer).serialize().c_str());
}

}  // namespace tvchannel
