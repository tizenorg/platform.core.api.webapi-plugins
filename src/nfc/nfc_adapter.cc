// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_adapter.h"
#include "nfc_util.h"

#include <glib.h>

#include "common/logger.h"
#include "common/platform_exception.h"

using namespace common;
using namespace std;

namespace extension {
namespace nfc {

namespace {
const std::string CALLBACK_ID = "callbackId";
const std::string LISTENER_ID = "listenerId";
const std::string TYPE = "type";
const std::string MODE = "mode";

const std::string CARD_ELEMENT = "CardElement";
const std::string TRANSACTION = "Transaction";

const std::string ACTIVE_SECURE_ELEMENT_CHANGED = "ActiveSecureElementChanged";
const std::string CARD_EMULATION_MODE_CHANGED = "CardEmulationModeChanged";
const std::string TRANSACTION_EVENT_LISTENER_ESE = "TransactionEventListener_ESE";
const std::string TRANSACTION_EVENT_LISTENER_UICC = "TransactionEventListener_UICC";
}

NFCAdapter::NFCAdapter():
        m_is_listener_set(false),
        m_is_transaction_ese_listener_set(false),
        m_is_transaction_uicc_listener_set(false),
        m_is_peer_listener_set(false),
        m_latest_peer_id(0),
        m_peer_handle(NULL),
        m_is_ndef_listener_set(false)
{
}

NFCAdapter::~NFCAdapter() {
    if (m_is_listener_set) {
        nfc_manager_unset_se_event_cb();
    }

    if (m_is_peer_listener_set) {
        nfc_manager_unset_p2p_target_discovered_cb();
    }
    if (m_is_transaction_ese_listener_set) {
        nfc_manager_unset_se_transaction_event_cb(NFC_SE_TYPE_ESE);
    }
    if (m_is_transaction_uicc_listener_set) {
        nfc_manager_unset_se_transaction_event_cb(NFC_SE_TYPE_UICC);
    }
    if (m_is_ndef_listener_set) {
        nfc_p2p_unset_data_received_cb(m_peer_handle);
    }
}

static picojson::value createEventError(double callbackId, PlatformException ex) {

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    NFCInstance::getInstance().InstanceReportError(ex, obj);
    obj.insert(std::make_pair(CALLBACK_ID, callbackId));

    return event;
}

static picojson::value createEventSuccess(double callbackId) {
    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    NFCInstance::getInstance().InstanceReportSuccess(obj);
    obj.insert(std::make_pair(CALLBACK_ID, callbackId));

    return event;
}

static gboolean setPoweredCompleteCB(void * user_data) {

    double* callbackId = static_cast<double*>(user_data);
    picojson::value event = createEventSuccess(*callbackId);
    NFCInstance::getInstance().PostMessage(event.serialize().c_str());

    delete callbackId;
    callbackId = NULL;
    return false;
}

static void targetDetectedCallback(nfc_discovered_type_e type,
        nfc_p2p_target_h target, void *data) {

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    obj.insert(make_pair("listenerId", "PeerListener"));

    NFCAdapter* adapter = NFCAdapter::GetInstance();

    //unregister previous NDEF listener
    if (adapter->IsNDEFListenerSet()) {
        adapter->UnsetReceiveNDEFListener(adapter->GetPeerId());
    }

    if (NFC_DISCOVERED_TYPE_ATTACHED == type) {
        adapter->SetPeerHandle(target);
        obj.insert(make_pair("action", "onattach"));
        adapter->IncreasePeerId();
        obj.insert(make_pair("id", static_cast<double>(adapter->GetPeerId())));
        NFCInstance::getInstance().PostMessage(event.serialize().c_str());
    } else {
        adapter->SetPeerHandle(NULL);
        obj.insert(make_pair("action", "ondetach"));
        NFCInstance::getInstance().PostMessage(event.serialize().c_str());
    }
}

NFCAdapter* NFCAdapter::GetInstance() {
    static NFCAdapter instance;
    return &instance;
}

bool NFCAdapter::GetPowered() {
    return nfc_manager_is_activated();
}

#ifndef APP_CONTROL_SETTING_SUPPORT

static void NFCSetActivationCompletedCallback(nfc_error_e error, void *user_data)
{
    double* callbackId = static_cast<double*>(user_data);

    if (NFC_ERROR_NONE != error) {
        auto ex = PlatformException(NFCUtil::getNFCErrorString(error),
                NFCUtil::getNFCErrorMessage(error));

        picojson::value event = createEventError(*callbackId, ex);

        NFCInstance::getInstance().PostMessage(event.serialize().c_str());
    } else {
        picojson::value event = createEventSuccess(*callbackId);

        NFCInstance::getInstance().PostMessage(event.serialize().c_str());
    }
    delete callbackId;
    callbackId = NULL;
}

#endif

static void se_event_callback(nfc_se_event_e se_event, void *user_data) {

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    NFCInstance::getInstance().InstanceReportSuccess(obj);

    string result;
    switch (se_event) {
        case NFC_SE_EVENT_SE_TYPE_CHANGED:
            result = NFCAdapter::GetInstance()->GetActiveSecureElement();
            obj.insert(make_pair(LISTENER_ID, ACTIVE_SECURE_ELEMENT_CHANGED));
            break;
        case NFC_SE_EVENT_CARD_EMULATION_CHANGED:
            result = NFCAdapter::GetInstance()->GetCardEmulationMode();
            obj.insert(make_pair(LISTENER_ID, CARD_EMULATION_MODE_CHANGED));
            break;
        default:
            LOGD("se_event_occured: %d", se_event);
            return;
    }

    obj.insert(make_pair(TYPE, CARD_ELEMENT));
    obj.insert(make_pair(MODE, result));
    NFCInstance::getInstance().PostMessage(event.serialize().c_str());
}

static void transaction_event_callback(nfc_se_type_e type,
                                       unsigned char *_aid,
                                       int aid_size,
                                       unsigned char *param,
                                       int param_size,
                                       void *user_data)
{
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    NFCInstance::getInstance().InstanceReportSuccess(response_obj);
    picojson::array& aid_array = response_obj.insert(std::make_pair("aid",
            picojson::value(picojson::array()))).first->second.get<picojson::array>();
    picojson::array& data_array = response_obj.insert(std::make_pair("data",
            picojson::value(picojson::array()))).first->second.get<picojson::array>();

    for (unsigned int i = 0; i < aid_size; i++) {
        aid_array.push_back(picojson::value(static_cast<double>(_aid[i])));
    }

    for (unsigned int i = 0; i < param_size; i++) {
        aid_array.push_back(picojson::value(static_cast<double>(param[i])));
    }

    if (NFC_SE_TYPE_ESE == type) {
        response_obj.insert(make_pair(LISTENER_ID, TRANSACTION_EVENT_LISTENER_ESE));
    } else {
        response_obj.insert(make_pair(LISTENER_ID, TRANSACTION_EVENT_LISTENER_UICC));
    }

    response_obj.insert(make_pair(TYPE, TRANSACTION));
    NFCInstance::getInstance().PostMessage(response.serialize().c_str());
}

void NFCAdapter::SetPowered(const picojson::value& args) {

    double* callbackId = new double(args.get(CALLBACK_ID).get<double>());
    bool powered = args.get("powered").get<bool>();

    if (nfc_manager_is_activated() == powered) {
        if (!g_idle_add(setPoweredCompleteCB, static_cast<void *>(callbackId))) {
            delete callbackId;
            callbackId = NULL;
            LOGE("g_idle addition failed");
            throw UnknownException("SetPowered failed.");
        }
        return;
    }

#ifdef APP_CONTROL_SETTING_SUPPORT
    app_control_h service = NULL;
    int ret = app_control_create(&service);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOGE("app_control_create failed: %d", ret);
        delete callbackId;
        callbackId = NULL;
        throw UnknownException("app_control_create failed");
    }

    ret = app_control_set_operation(service,
        "http://tizen.org/appcontrol/operation/setting/nfc");
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOGE("app_control_set_operation failed: %d", ret);
        delete callbackId;
        callbackId = NULL;
        throw UnknownException("app_control_set_operation failed");
    }

    ret = app_control_add_extra_data(service, "type", "nfc");
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOGE("app_control_add_extra_data failed: %d", ret);
        delete callbackId;
        callbackId = NULL;
        throw UnknownException("app_control_add_extra_data failed");
    }

    ret = app_control_send_launch_request(service, [](app_control_h request,
        app_control_h reply, app_control_result_e result, void *user_data){
        double* callbackId = static_cast<double*>(user_data);
        try {
            if (result == APP_CONTROL_RESULT_SUCCEEDED) {
                char *type = NULL;
                int ret = app_control_get_extra_data(reply, "nfc_status",
                    &type);
                if (ret != APP_CONTROL_ERROR_NONE) {
                    LOGE("app_control_get_extra_data failed: %d", ret);
                    throw UnknownException("app_control_get_extra_data failed");
                }

                LOGD("app_control result: %s", type);
            } else {
                LOGE("NFC enable app control failed : %d", result);
                throw UnknownException("NFC enable app control failed");
            }
        } catch (PlatformException &ex) {
            picojson::value event = createEventError(*callbackId, ex);
            NFCInstance::getInstance().PostMessage(event.serialize().c_str());
            return;
        }

        if (!g_idle_add(setPoweredCompleteCB, static_cast<void *>(callbackId))) {
            LOGE("g_idle addition failed");
            PlatformException ex = PlatformException("UnknownError", "UnknownError");
            picojson::value event = createEventError(*callbackId, ex);
            NFCInstance::getInstance().PostMessage(event.serialize().c_str());
        }
    }, static_cast<void *>(callbackId));

    if (ret != APP_CONTROL_ERROR_NONE) {
        LOGE("app_control_send_launch_request failed: %d", ret);
        delete callbackId;
        callbackId = NULL;
        throw UnknownException("app_control_send_operation failed");
    }

    ret = app_control_destroy(service);
    if (ret != APP_CONTROL_ERROR_NONE) {
        LOGE("app_control_destroy failed: %d", ret);
        throw UnknownException("app_control_destroy failed");
    }
#else
    int ret = nfc_manager_set_activation(powered,
                NFCSetActivationCompletedCallback, static_cast<void *>(callbackId));

    if (NFC_ERROR_NONE != ret) {
        LOGE("setPowered failed %d",ret);
        delete callbackId;
        callbackId = NULL;
        NFCUtil::throwNFCException(ret, "setPowered failed.");
    }
#endif
}


std::string NFCAdapter::GetCardEmulationMode() {

    LoggerD("Entered");

    nfc_se_card_emulation_mode_type_e mode;
    int ret = nfc_se_get_card_emulation_mode(&mode);

    if (NFC_ERROR_NONE != ret) {
        LoggerE("Failed to get card emulation mode %d", ret);
        NFCUtil::throwNFCException(ret, "Failed to get card emulation mode");
    }

    return NFCUtil::toStringCardEmulationMode(mode);
}

void NFCAdapter::SetCardEmulationMode(std::string mode) {

    LoggerD("Entered");

    nfc_se_card_emulation_mode_type_e new_mode =
            NFCUtil::toCardEmulationMode(mode);
    LoggerD("Card emulation mode value: %x", (int)new_mode);

    std::string current_mode = GetCardEmulationMode();

    if (mode.compare(current_mode) == 0) {
        LoggerD("Card emulation mode already set to given value (%s)",
                mode.c_str());
        return;
    }

    int ret = NFC_ERROR_NONE;
    switch (new_mode) {
        case NFC_SE_CARD_EMULATION_MODE_OFF:
            ret = nfc_se_disable_card_emulation();
            break;
        case NFC_SE_CARD_EMULATION_MODE_ON:
            ret = nfc_se_enable_card_emulation();
            break;
        default:
            // Should never go here - in case of invalid mode
            // exception is thrown from convertert few lines above
            LoggerE("Invalid card emulation mode: %s", mode.c_str());
            throw InvalidValuesException("Invalid card emulation mode given");
    }

    if (NFC_ERROR_NONE != ret) {
        LoggerE("Failed to set card emulation mode %d", ret);
        NFCUtil::throwNFCException(ret, "Failed to set card emulation mode");
    }
}

std::string NFCAdapter::GetActiveSecureElement() {

    LoggerD("Entered");

    nfc_se_type_e type;
    int ret = nfc_manager_get_se_type(&type);
    if (NFC_ERROR_NONE != ret) {
        LoggerE("Failed to get active secure element type: %d", ret);
        NFCUtil::throwNFCException(ret, "Unable to get active secure element type");
    }

    return NFCUtil::toStringSecureElementType(type);
}

void NFCAdapter::SetActiveSecureElement(std::string element) {

    LoggerD("Entered");

    // if given value is not correct secure element type then
    // there's no sense to get current value for comparison
    nfc_se_type_e new_type = NFCUtil::toSecureElementType(element);
    LoggerD("Secure element type value: %x", (int)new_type);

    std::string current_type = GetActiveSecureElement();
    if (element == current_type) {
        LoggerD("Active secure element type already set to: %s", element.c_str());
        return;
    }

    int ret = nfc_manager_set_se_type(new_type);
    if (NFC_ERROR_NONE != ret) {
        LoggerE("Failed to set active secure element type: %d", ret);
        NFCUtil::throwNFCException(ret,
                "Unable to set active secure element type");
    }
}

void NFCAdapter::SetExclusiveModeForTransaction(bool exmode) {

    LoggerD("Entered");

    int ret = NFC_ERROR_NONE;
    if (exmode) {
        ret = nfc_manager_enable_transaction_fg_dispatch();
    } else {
        ret = nfc_manager_disable_transaction_fg_dispatch();
    }

    if (NFC_ERROR_NONE != ret) {
        LoggerE("Failed to set exclusive mode for transaction: %d", ret);
        NFCUtil::throwNFCException(ret,
                "Setting exclusive mode for transaction failed.");
    }
}

void NFCAdapter::AddCardEmulationModeChangeListener() {
    if (!m_is_listener_set) {
        int ret = nfc_manager_set_se_event_cb(se_event_callback, NULL);
        if (NFC_ERROR_NONE != ret) {
            LOGE("AddCardEmulationModeChangeListener failed: %d", ret);
            NFCUtil::throwNFCException(ret,
                NFCUtil::getNFCErrorMessage(ret).c_str());
        }
    }

    m_is_listener_set = true;
}

void NFCAdapter::RemoveCardEmulationModeChangeListener() {
    if (!nfc_manager_is_supported()) {
        throw NotSupportedException("NFC Not Supported");
    }

    if (m_is_listener_set) {
        nfc_manager_unset_se_event_cb();
    }
    m_is_listener_set = false;
}


void NFCAdapter::AddTransactionEventListener(const picojson::value& args) {

    nfc_se_type_e se_type = NFCUtil::toSecureElementType(
            args.get("type").get<string>());
    int ret = NFC_ERROR_NONE;

    if (NFC_SE_TYPE_ESE == se_type) {
        if (m_is_transaction_ese_listener_set) {
            ret = nfc_manager_set_se_transaction_event_cb(se_type,
                transaction_event_callback, NULL);
        }
        m_is_transaction_ese_listener_set = true;
    } else {
        if (m_is_transaction_uicc_listener_set) {
            ret = nfc_manager_set_se_transaction_event_cb(se_type,
                transaction_event_callback, NULL);
        }
        m_is_transaction_uicc_listener_set = true;
    }

    if (NFC_ERROR_NONE != ret) {
        LOGE("AddTransactionEventListener failed: %d", ret);
        NFCUtil::throwNFCException(ret,
            NFCUtil::getNFCErrorMessage(ret).c_str());
    }
}

void NFCAdapter::RemoveTransactionEventListener(const picojson::value& args) {

    nfc_se_type_e se_type = NFCUtil::toSecureElementType(
                args.get("type").get<string>());

    nfc_manager_unset_se_transaction_event_cb(se_type);

    if (se_type == NFC_SE_TYPE_ESE) {
        m_is_transaction_ese_listener_set = false;
    } else {
        m_is_transaction_uicc_listener_set = false;
    }
}

void NFCAdapter::AddActiveSecureElementChangeListener() {
    if (!m_is_listener_set) {
        int ret = nfc_manager_set_se_event_cb(se_event_callback, NULL);
        if (NFC_ERROR_NONE != ret) {
            LOGE("AddActiveSecureElementChangeListener failed: %d", ret);
            NFCUtil::throwNFCException(ret,
                NFCUtil::getNFCErrorMessage(ret).c_str());
        }
    }

    m_is_listener_set = true;
}

void NFCAdapter::RemoveActiveSecureElementChangeListener() {
    if (!nfc_manager_is_supported()) {
        throw NotSupportedException("NFC Not Supported");
    }

    if (m_is_listener_set) {
        nfc_manager_unset_se_event_cb();
    }
    m_is_listener_set = false;
}

void NFCAdapter::SetPeerHandle(nfc_p2p_target_h handle) {
    m_peer_handle = handle;
}

nfc_p2p_target_h NFCAdapter::GetPeerHandle() {
    return m_peer_handle;
}

int NFCAdapter::GetPeerId() {
    return m_latest_peer_id;
}

void NFCAdapter::IncreasePeerId() {
    m_latest_peer_id++;
}

bool NFCAdapter::IsPeerConnected(int peer_id) {
    if (m_latest_peer_id != peer_id || !m_peer_handle) {
        return false;
    }

    nfc_p2p_target_h handle = NULL;
    int ret = nfc_manager_get_connected_target(&handle);
    if (NFC_ERROR_NONE != ret) {
        LOGE("Failed to get connected target handle: %d", ret);
        NFCUtil::throwNFCException(ret, "Failed to get connected target handle.");
    }

    if (m_peer_handle == handle) {
        return true;
    }

    return false;
}

void NFCAdapter::SetPeerListener() {
    if (!nfc_manager_is_supported()) {
        throw NotSupportedException("NFC Not Supported");
    }

    if (!m_is_peer_listener_set) {
        int ret = nfc_manager_set_p2p_target_discovered_cb (targetDetectedCallback, NULL);
        if (NFC_ERROR_NONE != ret) {
            LOGE("Failed to set listener: %d", ret);
            NFCUtil::throwNFCException(ret, "setPeerListener failed");
        }
    }

    m_is_peer_listener_set = true;
}

void NFCAdapter::UnsetPeerListener() {
    if (!nfc_manager_is_supported()) {
        throw NotSupportedException("NFC Not Supported");
    }

    if (m_is_peer_listener_set) {
        nfc_manager_unset_p2p_target_discovered_cb();
    }

    m_is_peer_listener_set = false;
}

static void targetReceivedCallback(nfc_p2p_target_h target, nfc_ndef_message_h message, void *data)
{
    unsigned char *raw_data = NULL;
    unsigned int size;
    if (NFC_ERROR_NONE != nfc_ndef_message_get_rawdata(message, &raw_data, &size)) {
        LOGE("Unknown error while getting raw data of message.");
        free(raw_data);
        return;
    }

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    obj.insert(make_pair("listenerId", "ReceiveNDEFListener"));
    obj.insert(make_pair("id", static_cast<double>(NFCAdapter::GetInstance()->GetPeerId())));

    //TODO call function which create NDEFMessage object (from nfc_message_utils)

    NFCInstance::getInstance().PostMessage(event.serialize().c_str());
    free(raw_data);
}

void NFCAdapter::SetReceiveNDEFListener(int peer_id) {

    //unregister previous NDEF listener
    if (m_is_ndef_listener_set) {
        int ret = nfc_p2p_unset_data_received_cb(m_peer_handle);
        if (NFC_ERROR_NONE != ret) {
            LOGW("Unregister ReceiveNDEFListener error: %d", ret);
        }
        m_is_ndef_listener_set = false;
    }

    //check if peer object is still connected
    if (!IsPeerConnected(peer_id)) {
        LOGE("Target is not connected");
        throw UnknownException("Target is not connected");
    }

    int ret = nfc_p2p_set_data_received_cb(m_peer_handle, targetReceivedCallback, NULL);
    if (NFC_ERROR_NONE != ret) {
        LOGE("Failed to set NDEF listener: %d", ret);
        NFCUtil::throwNFCException(ret, "Failed to set NDEF listener");
    }

    m_is_ndef_listener_set = true;
}

void NFCAdapter::UnsetReceiveNDEFListener(int peer_id) {
    if (m_is_ndef_listener_set) {
        //check if peer object is still connected
        if (!IsPeerConnected(peer_id)) {
            LOGE("Target is not connected");
        }

        int ret = nfc_p2p_unset_data_received_cb(m_peer_handle);
        if (NFC_ERROR_NONE != ret) {
            LOGE("Unregister ReceiveNDEFListener error: %d", ret);
            NFCUtil::throwNFCException(ret, "Unregister ReceiveNDEFListener error");
        }

        m_is_ndef_listener_set = false;
    }
}

bool NFCAdapter::IsNDEFListenerSet() {
    return m_is_ndef_listener_set;
}

}// nfc
}// extension
