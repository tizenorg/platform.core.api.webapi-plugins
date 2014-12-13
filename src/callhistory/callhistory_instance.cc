// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory/callhistory_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace callhistory {

namespace {
// The privileges that required in CallHistory API
const std::string kPrivilegeCallHistoryRead = "http://tizen.org/privilege/callhistory.read";
const std::string kPrivilegeCallHistoryWrite = "http://tizen.org/privilege/callhistory.write";
}

using namespace common;

CallHistoryInstance::CallHistoryInstance() {
    using namespace std::placeholders;
#define REGISTER_SYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&CallHistoryInstance::x, this, _1, _2));
    REGISTER_SYNC("remove", Remove);
    REGISTER_SYNC("addChangeListener", AddChangeListener);
    REGISTER_SYNC("removeChangeListener", RemoveChangeListener);
#undef REGISTER_SYNC
#define REGISTER_ASYNC(c,x) \
        RegisterHandler(c, std::bind(&CallHistoryInstance::x, this, _1, _2));
    REGISTER_ASYNC("find", Find);
    REGISTER_ASYNC("removeBatch", RemoveBatch);
    REGISTER_ASYNC("removeAll", RemoveAll);
#undef REGISTER_ASYNC
}

CallHistoryInstance::~CallHistoryInstance() {
}

void CallHistoryInstance::Find(const picojson::value& args, picojson::object& out) {

}

void CallHistoryInstance::Remove(const picojson::value& args, picojson::object& out) {

}

void CallHistoryInstance::RemoveBatch(const picojson::value& args, picojson::object& out) {

}

void CallHistoryInstance::RemoveAll(const picojson::value& args, picojson::object& out) {

}

void CallHistoryInstance::AddChangeListener(const picojson::value& args, picojson::object& out) {

}

void CallHistoryInstance::RemoveChangeListener(const picojson::value& args, picojson::object& out) {

}

} // namespace callhistory
} // namespace extension
