// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

#include "nfc/nfc_instance.h"

#include "common/picojson.h"
#include <memory>
#include <nfc.h>

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
    void AddTransactionEventListener(const picojson::value& args);
    void RemoveTransactionEventListener(const picojson::value& args);
    void AddActiveSecureElementChangeListener();
    void RemoveActiveSecureElementChangeListener();
    void SetPeerHandle(nfc_p2p_target_h handle);
    nfc_p2p_target_h GetPeerHandle();
    int GetPeerId();
    void IncreasePeerId();
    bool IsPeerConnected(int peer_id);
    void SetPeerListener();
    void UnsetPeerListener();
    void SetReceiveNDEFListener(int peer_id);
    void UnsetReceiveNDEFListener(int peer_id);
    bool IsNDEFListenerSet();

    static NFCAdapter* GetInstance();
private:
    NFCAdapter();
    virtual ~NFCAdapter();

    bool m_is_listener_set;
    bool m_is_transaction_ese_listener_set;
    bool m_is_transaction_uicc_listener_set;
    bool m_is_peer_listener_set;
    int m_latest_peer_id;
    nfc_p2p_target_h m_peer_handle;
    bool m_is_ndef_listener_set;
};

} // nfc
} // extension

#endif // NFC_NFC_ADAPTER_H_
