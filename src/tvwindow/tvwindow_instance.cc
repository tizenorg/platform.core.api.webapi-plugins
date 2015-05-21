// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tvwindow/tvwindow_instance.h"
#include "common/logger.h"
#include "tizen/tizen.h"
#include "common/picojson.h"

namespace extension {
namespace tvwindow {

TVWindowInstance::TVWindowInstance(TVWindowExtension const& extension) {
    LOGD("Entered");
}

TVWindowInstance::~TVWindowInstance() {
    LOGD("Entered");
}

void TVWindowInstance::HandleMessage(const char* msg) {
    // this is stub, no async messages
    LOGD("Entered");
}

void TVWindowInstance::HandleSyncMessage(const char* msg) {
    LOGD("Entered %s", msg);
    picojson::object answer;
    answer["answer"] = picojson::value(true);

    SendSyncReply(picojson::value(answer).serialize().c_str());
}

}  // namespace tvwindow
}  // namespace extension
