// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secureelement/secureelement_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"


namespace extension {
namespace secureelement {

using namespace common;

SecureElementInstance::SecureElementInstance() {
    using namespace std::placeholders;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SecureElementInstance::x, this, _1, _2));

    REGISTER_SYNC("SEService_registerSEListener", RegisterSEListener);
    REGISTER_SYNC("SEService_unregisterSEListener", UnregisterSEListener);
    REGISTER_SYNC("SEService_shutdown", Shutdown);
    REGISTER_SYNC("SEReader_getName", GetName);
    REGISTER_SYNC("SEReader_closeSessions", CloseSessions);
    REGISTER_SYNC("SESession_getATR", GetATR);
    REGISTER_SYNC("SESession_close", CloseSession);
    REGISTER_SYNC("SESession_closeChannels", CloseChannels);
    REGISTER_SYNC("SEChannel_close", CloseChannel);
    REGISTER_SYNC("SEChannel_getSelectResponse", GetSelectResponse);
#undef REGISTER_SYNC

#define REGISTER(c,x) \
    RegisterHandler(c, std::bind(&SecureElementInstance::x, this, _1, _2));

    REGISTER("SEService_getReaders", GetReaders);
    REGISTER("SEReader_openSession", OpenSession);
    REGISTER("SESession_openBasicChannel", OpenBasicChannel);
    REGISTER("SESession_openLogicalChannel ", OpenLogicalChannel);
    REGISTER("SEChannel_transmit", Transmit);
#undef REGISTER
}

SecureElementInstance::~SecureElementInstance() {
}

void SecureElementInstance::RegisterSEListener(
        const picojson::value& args, picojson::object& out) {
}

void SecureElementInstance::UnregisterSEListener(
        const picojson::value& args, picojson::object& out) {
}

void SecureElementInstance::Shutdown(
        const picojson::value& args, picojson::object& out) {
}

void SecureElementInstance::GetName(
        const picojson::value& args, picojson::object& out) {
}

void SecureElementInstance::CloseSessions(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::GetATR(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::CloseSession(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::CloseChannels(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::CloseChannel(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::GetSelectResponse(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::GetReaders(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::OpenSession(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::OpenBasicChannel(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::OpenLogicalChannel(
        const picojson::value& args, picojson::object& out) {

}

void SecureElementInstance::Transmit(
        const picojson::value& args, picojson::object& out) {

}

} // namespace secureelement
} // namespace extension
