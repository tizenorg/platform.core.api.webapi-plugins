// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_instance.h"
#include "nfc_util.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

// platform header
#include <nfc.h>


namespace extension {
namespace nfc {

using namespace common;
using namespace extension::nfc;

NFCInstance& NFCInstance::getInstance() {
    static NFCInstance instance;
    return instance;
}

NFCInstance::NFCInstance() {
    using namespace std::placeholders;
#define REGISTER_SYNC(c,x) \
        RegisterSyncHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
    REGISTER_SYNC("NFCManager_getDefaultAdapter", GetDefaultAdapter);
    REGISTER_SYNC("NFCManager_setExclusiveMode", SetExclusiveMode);
    REGISTER_SYNC("NFCAdapter_getPowered", GetPowered);
    REGISTER_SYNC("NFCAdapter_cardEmulationModeSetter", CardEmulationModeSetter);
    REGISTER_SYNC("NFCAdapter_cardEmulationModeGetter", CardEmulationModeGetter);
    REGISTER_SYNC("NFCAdapter_activeSecureElementSetter", ActiveSecureElementSetter);
    REGISTER_SYNC("NFCAdapter_activeSecureElementGetter", ActiveSecureElementGetter);
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

void NFCInstance::InstanceReportSuccess(picojson::object& out) {
    out.insert(std::make_pair("status", picojson::value("success")));
}

void NFCInstance::InstanceReportSuccess(const picojson::value& result, picojson::object& out) {
    out.insert(std::make_pair("status", picojson::value("success")));
    out.insert(std::make_pair("result", result));
}

void NFCInstance::InstanceReportError(picojson::object& out) {
    out.insert(std::make_pair("status", picojson::value("error")));
}

void NFCInstance::InstanceReportError(const PlatformException& ex, picojson::object& out) {
    out.insert(std::make_pair("status", picojson::value("error")));
    out.insert(std::make_pair("error", ex.ToJSON()));
}


void NFCInstance::GetDefaultAdapter(
        const picojson::value& args, picojson::object& out) {

    // Default NFC adapter is created at JS level
    // Here there's only check for NFC support
    LoggerD("Entered");
    if(!nfc_manager_is_supported()) {
        LoggerE("NFC manager is not supported");
        // According to API reference only Security and Unknown
        // exceptions are allowed here
        auto ex = common::UnknownException("NFC manager not supported");
        ReportError(ex, out);
    }
    else {
        ReportSuccess(out);
    }
}

void NFCInstance::SetExclusiveMode(
        const picojson::value& args, picojson::object& out) {

    bool exmode = args.get("exclusiveMode").get<bool>();
    int ret = NFC_ERROR_NONE;

    int result = nfc_manager_set_system_handler_enable(!exmode);
    if (NFC_ERROR_NONE != result) {
        NFCUtil::throwNFCException(result, "Failed to set exclusive mode.");
    }
    ReportSuccess(out);
}

void NFCInstance::SetPowered(
        const picojson::value& args, picojson::object& out) {
    NFCAdapter::GetInstance()->SetPowered(args);
}

void NFCInstance::GetPowered(
        const picojson::value& args, picojson::object& out) {
    bool ret = NFCAdapter::GetInstance()->GetPowered();
    ReportSuccess(picojson::value(ret), out);
}

void NFCInstance::CardEmulationModeSetter(
        const picojson::value& args, picojson::object& out) {

    std::string mode = args.get("emulationMode").get<std::string>();
    try {
        NFCAdapter::GetInstance()->SetCardEmulationMode(mode);
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::CardEmulationModeGetter(
        const picojson::value& args, picojson::object& out) {

    std::string mode;
    try {
        mode = NFCAdapter::GetInstance()->GetCardEmulationMode();
        ReportSuccess(picojson::value(mode), out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::ActiveSecureElementSetter(
        const picojson::value& args, picojson::object& out) {

    std::string ase = args.get("secureElement").get<std::string>();
    try {
        NFCAdapter::GetInstance()->SetActiveSecureElement(ase);
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::ActiveSecureElementGetter(
        const picojson::value& args, picojson::object& out) {

    std::string ase;
    try {
        ase = NFCAdapter::GetInstance()->GetActiveSecureElement();
        ReportSuccess(picojson::value(ase), out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
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
    NFCAdapter::GetInstance()->AddCardEmulationModeChangeListener();
}

void NFCInstance::RemoveCardEmulationModeChangeListener(
        const picojson::value& args, picojson::object& out) {
    NFCAdapter::GetInstance()->RemoveCardEmulationModeChangeListener();
}

void NFCInstance::AddTransactionEventListener(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::AddActiveSecureElementChangeListener(
        const picojson::value& args, picojson::object& out) {
    NFCAdapter::GetInstance()->AddActiveSecureElementChangeListener();
}

void NFCInstance::RemoveActiveSecureElementChangeListener(
        const picojson::value& args, picojson::object& out) {
    NFCAdapter::GetInstance()->RemoveActiveSecureElementChangeListener();
}

void NFCInstance::GetCachedMessage(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SetExclusiveModeForTransaction(
        const picojson::value& args, picojson::object& out) {

    bool transaction_mode = args.get("transactionMode").get<bool>();
    int ret = NFC_ERROR_NONE;

    try {
        NFCAdapter::GetInstance()->SetExclusiveModeForTransaction(
                transaction_mode);
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
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
