// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

#include "nfc/nfc_instance.h"

#include "common/picojson.h"
#include <memory>

#ifdef APP_CONTROL_SETTING_SUPPORT
#include <app_control.h>
#endif

namespace extension {
namespace nfc {

class NFCInstance;

class NFCAdapter {
public:
    bool GetPowered();
    void SetPowered(const picojson::value& args);

// cardEmulationMode getter and setter
    std::string GetCardEmulationMode();
    void SetCardEmulationMode(std::string mode);
// activeSecureElement getter and setter
    std::string GetActiveSecureElement();
    void SetActiveSecureElement(std::string element);

// Adapter methods
    void SetExclusiveModeForTransaction(bool exmode);

    void AddCardEmulationModeChangeListener();
    void RemoveCardEmulationModeChangeListener();

    static NFCAdapter* GetInstance();
private:
    NFCAdapter();
    virtual ~NFCAdapter();

    bool m_is_listener_set;
};

} // nfc
} // extension

#endif // NFC_NFC_ADAPTER_H_
