// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_instance.h"
#include "nfc_util.h"
#include "nfc_message_utils.h"

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
    REGISTER_SYNC("NFCAdapter_PeerIsConnectedGetter", PeerIsConnectedGetter);
    REGISTER_SYNC("NFCAdapter_unsetTagListener", UnsetTagListener);
    REGISTER_SYNC("NFCAdapter_unsetPeerListener", UnsetPeerListener);
    REGISTER_SYNC("NFCAdapter_addCardEmulationModeChangeListener",
            AddCardEmulationModeChangeListener);
    REGISTER_SYNC("NFCAdapter_removeCardEmulationModeChangeListener",
            RemoveCardEmulationModeChangeListener);
    REGISTER_SYNC("NFCAdapter_addTransactionEventListener",
            AddTransactionEventListener);
    REGISTER_SYNC("NFCAdapter_removeTransactionEventListener",
            RemoveTransactionEventListener);
    REGISTER_SYNC("NFCAdapter_addActiveSecureElementChangeListener",
            AddActiveSecureElementChangeListener);
    REGISTER_SYNC("NFCAdapter_removeActiveSecureElementChangeListener",
            RemoveActiveSecureElementChangeListener);
    REGISTER_SYNC("NFCAdapter_getCachedMessage", GetCachedMessage);
    REGISTER_SYNC("NFCAdapter_setExclusiveModeForTransaction",
            SetExclusiveModeForTransaction);
    REGISTER_SYNC("NFCPeer_setReceiveNDEFListener", SetReceiveNDEFListener);
    REGISTER_SYNC("NFCPeer_unsetReceiveNDEFListener", UnsetReceiveNDEFListener);
    REGISTER_SYNC("NDEFMessage_toByte", ToByte);
    //Message related methods
    REGISTER_SYNC("NDEFMessage_constructor", NDEFMessageContructor);
    REGISTER_SYNC("NDEFMessage_toByte", ToByte);
    REGISTER_SYNC("NDEFRecord_constructor", NDEFRecordContructor);
    REGISTER_SYNC("NDEFRecordText_constructor", NDEFRecordTextContructor);
    REGISTER_SYNC("NDEFRecordURI_constructor", NDEFRecordURIContructor);
    REGISTER_SYNC("NDEFRecordMedia_constructor", NDEFRecordMediaContructor);

    // NFCTag attributes getters
    REGISTER_SYNC("NFCTag_typeGetter", TagTypeGetter);
    REGISTER_SYNC("NFCTag_isSupportedNDEFGetter", TagIsSupportedNDEFGetter);
    REGISTER_SYNC("NFCTag_NDEFSizeGetter", TagNDEFSizeGetter);
    REGISTER_SYNC("NFCTag_propertiesGetter", TagPropertiesGetter);
    REGISTER_SYNC("NFCTag_isConnectedGetter", TagIsConnectedGetter);
#undef REGISTER_SYNC
#define REGISTER(c,x) \
    RegisterHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
    REGISTER("NFCAdapter_setPowered", SetPowered);
    REGISTER("NFCTag_readNDEF", ReadNDEF);
    REGISTER("NFCTag_writeNDEF", WriteNDEF);
    REGISTER("NFCTag_transceive ", Transceive );
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

    try {
        NFCAdapter::GetInstance()->SetTagListener();
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::PeerIsConnectedGetter(
        const picojson::value& args, picojson::object& out) {

    try {
        int peer_id = (int)args.get("id").get<double>();
        bool ret = NFCAdapter::GetInstance()->IsPeerConnected(peer_id);
        ReportSuccess(picojson::value(ret), out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::SetPeerListener(
        const picojson::value& args, picojson::object& out) {

    try {
        NFCAdapter::GetInstance()->SetPeerListener();
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::UnsetTagListener(
        const picojson::value& args, picojson::object& out) {

    NFCAdapter::GetInstance()->UnsetTagListener();
    ReportSuccess(out);
}

void NFCInstance::UnsetPeerListener(
        const picojson::value& args, picojson::object& out) {

    try {
        NFCAdapter::GetInstance()->UnsetPeerListener();
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::AddCardEmulationModeChangeListener(
        const picojson::value& args, picojson::object& out) {
    try {
        NFCAdapter::GetInstance()->AddCardEmulationModeChangeListener();
        ReportSuccess(out);
    } catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::RemoveCardEmulationModeChangeListener(
        const picojson::value& args, picojson::object& out) {
    NFCAdapter::GetInstance()->RemoveCardEmulationModeChangeListener();
}

void NFCInstance::AddTransactionEventListener(
        const picojson::value& args, picojson::object& out) {
    try {
        NFCAdapter::GetInstance()->AddTransactionEventListener(args);
        ReportSuccess(out);
    } catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::RemoveTransactionEventListener(
        const picojson::value& args, picojson::object& out) {
    NFCAdapter::GetInstance()->RemoveTransactionEventListener(args);
}

void NFCInstance::AddActiveSecureElementChangeListener(
        const picojson::value& args, picojson::object& out) {
    try {
        NFCAdapter::GetInstance()->AddActiveSecureElementChangeListener();
        ReportSuccess(out);
    } catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
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

    int tag_id = (int)args.get("id").get<double>();
    LoggerD("Tag id: %d", tag_id);

    try {
        NFCAdapter::GetInstance()->TagReadNDEF(tag_id, args);
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::WriteNDEF(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::Transceive(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::SetReceiveNDEFListener(
        const picojson::value& args, picojson::object& out) {

    try {
        int peer_id = (int)args.get("id").get<double>();
        NFCAdapter::GetInstance()->SetReceiveNDEFListener(peer_id);
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::UnsetReceiveNDEFListener(
        const picojson::value& args, picojson::object& out) {

    try {
        int peer_id = (int)args.get("id").get<double>();
        NFCAdapter::GetInstance()->UnsetReceiveNDEFListener(peer_id);
        ReportSuccess(out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::SendNDEF(
        const picojson::value& args, picojson::object& out) {

}

void NFCInstance::ToByte(
        const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    try {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();
        NFCMessageUtils::NDEFMessageToByte(args, result_obj);
        ReportSuccess(result, out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

//Message related methods
void NFCInstance::NDEFMessageContructor(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    try {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();
        NFCMessageUtils::ReportNDEFMessage(args, result_obj);
        ReportSuccess(result, out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::NDEFRecordContructor(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    try {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();
        NFCMessageUtils::ReportNDEFRecord(args, result_obj);
        ReportSuccess(result, out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::NDEFRecordTextContructor(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    try {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();
        NFCMessageUtils::ReportNDEFRecordText(args, result_obj);
        ReportSuccess(result, out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::NDEFRecordURIContructor(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    try {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();
        NFCMessageUtils::ReportNDEFRecordURI(args, result_obj);
        ReportSuccess(result, out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

void NFCInstance::NDEFRecordMediaContructor(const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    try {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();
        NFCMessageUtils::ReportNDEFRecordMedia(args, result_obj);
        ReportSuccess(result, out);
    }
    catch(const common::PlatformException& ex) {
        ReportError(ex, out);
    }
}

// NFCTag attributes getters
void NFCInstance::TagTypeGetter(
        const picojson::value& args, picojson::object& out) {

    LoggerD("Entered");

    int tag_id = (int)args.get("id").get<double>();
    LoggerD("Tag id: %d", tag_id);

    try {
        // Function below throws exception if core API call fails
        if (!NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id)) {
            LoggerE("Tag with id %d is not connected anymore", tag_id);
            // If tag is not connected then attribute's value
            // should be undefined
            ReportError(out);
            return;
        }

        std::string tag_type =
                NFCAdapter::GetInstance()->TagTypeGetter(tag_id);

        ReportSuccess(picojson::value(tag_type), out);
    }
    catch(const PlatformException& ex) {
        LoggerE("Failed to check tag connection");
        ReportError(ex, out);
    }
}

void NFCInstance::TagIsSupportedNDEFGetter(
        const picojson::value& args, picojson::object& out) {

    LoggerD("Entered");

    int tag_id = (int)args.get("id").get<double>();
    LoggerD("Tag id: %d", tag_id);

    try {
        // Function below throws exception if core API call fails
        if (!NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id)) {
            LoggerE("Tag with id %d is not connected anymore", tag_id);
            // If tag is not connected then attribute's value
            // should be undefined
            ReportError(out);
            return;
        }

        bool is_supported =
                NFCAdapter::GetInstance()->TagIsSupportedNDEFGetter(tag_id);

        ReportSuccess(picojson::value(is_supported), out);
    }
    catch(const PlatformException& ex) {
        LoggerE("Failed to check is NDEF supported");
        ReportError(ex, out);
    }

}

void NFCInstance::TagNDEFSizeGetter(
        const picojson::value& args, picojson::object& out) {

    LoggerD("Entered");

    int tag_id = (int)args.get("id").get<double>();
    LoggerD("Tag id: %d", tag_id);

    try {
        // Function below throws exception if core API call fails
        if (!NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id)) {
            LoggerE("Tag with id %d is not connected anymore", tag_id);
            // If tag is not connected then attribute's value
            // should be undefined
            ReportError(out);
            return;
        }

        unsigned int ndef_size =
                NFCAdapter::GetInstance()->TagNDEFSizeGetter(tag_id);

        ReportSuccess(picojson::value((double)ndef_size), out);
    }
    catch(const PlatformException& ex) {
        LoggerE("Failed to get tag NDEF size");
        ReportError(ex, out);
    }

}

void NFCInstance::TagPropertiesGetter(
        const picojson::value& args, picojson::object& out) {

    LoggerD("Entered");

    int tag_id = (int)args.get("id").get<double>();
    LoggerD("Tag id: %d", tag_id);
    try {
        // Function below throws exception if core API call fails
        if (!NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id)) {
            LoggerE("Tag with id %d is not connected anymore", tag_id);
            // If tag is not connected then attribute's value
            // should be undefined
            ReportError(out);
            return;
        }

        NFCTagPropertiesT result =
                NFCAdapter::GetInstance()->TagPropertiesGetter(tag_id);

        picojson::value properties = picojson::value(picojson::array());
        picojson::array& properties_array = properties.get<picojson::array>();
        for (auto it = result.begin() ; it != result.end(); it++) {
            picojson::value val = picojson::value(picojson::object());
            picojson::object& obj = val.get<picojson::object>();

            picojson::value value_vector = picojson::value(picojson::array());
            picojson::array& value_vector_obj = value_vector.get<picojson::array>();

            for (size_t i = 0 ; i < it->second.size(); i++) {
                value_vector_obj.push_back(picojson::value(
                        std::to_string(it->second[i])));
            }

            obj.insert(std::make_pair(it->first, value_vector));
            properties_array.push_back(val);
        }
        ReportSuccess(properties, out);
    }
    catch(const PlatformException& ex) {
        LoggerE("Failed to tag properties");
        ReportError(ex, out);
    }
}

void NFCInstance::TagIsConnectedGetter(
        const picojson::value& args, picojson::object& out) {

    LoggerD("Entered");

    int tag_id = (int)args.get("id").get<double>();
    LoggerD("Tag id: %d", tag_id);
    try {
        bool connected = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id);
        ReportSuccess(picojson::value(connected), out);
    }
    catch(const PlatformException& ex) {
        LoggerE("Failed to check tag connection");
        ReportError(ex, out);
    }
}


} // namespace nfc
} // namespace extension
