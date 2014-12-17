// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc/nfc_instance.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

// platform header
#include <nfc.h>


namespace extension {
namespace nfc {

using namespace common;
using namespace extension::nfc;

NFCInstance::NFCInstance() {
    using namespace std::placeholders;
#define REGISTER_SYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
    REGISTER_SYNC("NFCManager_getDefaultAdapter", GetDefaultAdapter);
    REGISTER_SYNC("NFCManager_setExclusiveMode", SetExclusiveMode);
    REGISTER_SYNC("NFCAdapter_getPowered", GetPowered);
    REGISTER_SYNC("NFCAdapter_setPeerListener", SetPeerListener);
    REGISTER_SYNC("NFCAdapter_setTagListener", SetTagListener);
    REGISTER_SYNC("NFCAdapter_setPeerListener", SetPeerListener);
    REGISTER_SYNC("NFCAdapter_unsetTagListener", UnsetTagListener);
    REGISTER_SYNC("NFCAdapter_unsetPeerListener", UnsetPeerListener);
    REGISTER_SYNC("NFCAdapter_addCardEmulationModeChangeListener",
            AddCardEmulationModeChangeListener);
    REGISTER_SYNC("NFCAdapter_removeCardEmulationModeChangeListener",
            RemoveCardEmulationModeChangeListener);
    REGISTER_SYNC("NFCAdapter_addTransactionEventListener",
            AddTransactionEventListener);
    REGISTER_SYNC("NFCAdapter_addActiveSecureElementChangeListener",
            AddActiveSecureElementChangeListener);
    REGISTER_SYNC("NFCAdapter_removeActiveSecureElementChangeListener",
            RemoveActiveSecureElementChangeListener);
    REGISTER_SYNC("NFCAdapter_getCachedMessage", GetCachedMessage);
    REGISTER_SYNC("NFCAdapter_setExclusiveModeForTransaction",
            SetExclusiveModeForTransaction);
    REGISTER_SYNC("NFCPeer_unsetReceiveNDEFListener", UnsetReceiveNDEFListener);
    REGISTER_SYNC("NDEFMessage_toByte", ToByte);
#undef REGISTER_SYNC
#define REGISTER(c,x) \
    RegisterHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
    REGISTER("NFCAdapter_setPowered", SetPowered);
    REGISTER("NFCTag_readNDEF", ReadNDEF);
    REGISTER("NFCTag_writeNDEF", WriteNDEF);
    REGISTER("NFCTag_transceive ", Transceive );
    REGISTER("NFCPeer_setReceiveNDEFListener", SetReceiveNDEFListener);
    REGISTER("NFCPeer_sendNDEF", SendNDEF);
#undef REGISTER
    // NFC library initialization
    int result = nfc_manager_initialize();
    if (NFC_ERROR_NONE != result) {
        LoggerE("Could not initialize NFC Manager.");
    }
}

NFCInstance::~NFCInstance() {
    int result = nfc_manager_deinitialize();
    if (NFC_ERROR_NONE != result) {
        LoggerE("NFC Manager deinitialization failed.");
    }
}

void NFCInstance::GetDefaultAdapter(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SetExclusiveMode(
        const picojson::value& args, picojson::object& out) {

    bool exmode = args.get("exclusiveMode").get<bool>();
    int ret = NFC_ERROR_NONE;

    if(exmode) {
        ret = nfc_manager_enable_transaction_fg_dispatch();
    }
    else {
        ret = nfc_manager_disable_transaction_fg_dispatch();
    }

    if (NFC_ERROR_NONE != ret) {
        LoggerE("setExclusiveModeForTransaction failed: %d", ret);
        switch(ret) {
            case NFC_ERROR_SECURITY_RESTRICTED:
            {
                auto ex = common::SecurityException("Not allowed to set exclusive mode");
                ReportError(ex, out);
                break;
            }
            case NFC_ERROR_OPERATION_FAILED:
            {
                auto ex = common::UnknownException("Setting exclusive mode failed (IPC fail)");
                ReportError(ex, out);
                break;
            }
            default:
            {
                auto ex = common::UnknownException("Unkown error");
                ReportError(ex, out);
                break;
            }
        }
    }
    else {
        ReportSuccess(out);
    }
}

void NFCInstance::SetPowered(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::GetPowered(
        const picojson::value& args, picojson::object& out) {
    bool ret = NFCAdapter::GetInstance()->GetPowered();
    ReportSuccess(picojson::value(ret), out);
}

void NFCInstance::SetTagListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SetPeerListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::UnsetTagListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::UnsetPeerListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::AddCardEmulationModeChangeListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::RemoveCardEmulationModeChangeListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::AddTransactionEventListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::AddActiveSecureElementChangeListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::RemoveActiveSecureElementChangeListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::GetCachedMessage(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SetExclusiveModeForTransaction(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::ReadNDEF(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::WriteNDEF(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::Transceive(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SetReceiveNDEFListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::UnsetReceiveNDEFListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SendNDEF(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::ToByte(
        const picojson::value& args, picojson::object& out) {

}

} // namespace nfc
} // namespace extension
