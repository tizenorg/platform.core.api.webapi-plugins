// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_adapter.h"
#include "nfc_util.h"

#include <nfc.h>
#include <glib.h>

#include "common/logger.h"
#include "common/platform_exception.h"

using namespace common;
using namespace std;

namespace extension {
namespace nfc {

NFCAdapter::NFCAdapter() {

}

NFCAdapter::~NFCAdapter() {

}

static picojson::value createEventError(double callbackId, PlatformException ex) {

    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    NFCInstance::getInstance().InstanceReportError(ex, obj);
    obj.insert(std::make_pair("callbackId", callbackId));

    return event;
}

static picojson::value createEventSuccess(double callbackId) {
    picojson::value event = picojson::value(picojson::object());
    picojson::object& obj = event.get<picojson::object>();
    NFCInstance::getInstance().InstanceReportSuccess(obj);
    obj.insert(std::make_pair("callbackId", callbackId));

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

void NFCAdapter::SetPowered(const picojson::value& args) {

    double* callbackId = new double(args.get("callbackId").get<double>());
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
    int ret = nfc_manager_set_activation(args.get("powered").get<bool>(),
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

    nfc_se_card_emulation_mode_type_e newmode =
            NFCUtil::toCardEmulationMode(mode);
    std::string current_mode = GetCardEmulationMode();

    if (mode.compare(current_mode) == 0) {
        LoggerD("Card emulation mode already set to given value (%s)",
                mode.c_str());
        return;
    }

    int ret = NFC_ERROR_NONE;
    switch (newmode) {
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


}// nfc
}// extension
