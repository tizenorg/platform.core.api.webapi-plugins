// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

#include "nfc/nfc_instance.h"
#include "nfc/nfc_util.h"

#include "common/picojson.h"
#include <memory>
#include <nfc.h>
#include <list>

#ifdef APP_CONTROL_SETTING_SUPPORT
#include <app_control.h>
#endif

namespace extension {
namespace nfc {

class NFCInstance;

typedef std::list<std::pair<std::string, UCharVector>> NFCTagPropertiesT;

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
  void GetCachedMessage(picojson::object& out);

  void SetPeerHandle(nfc_p2p_target_h handle);
  nfc_p2p_target_h GetPeerHandle();
  int GetPeerId();
  void IncreasePeerId();
  bool PeerIsConnectedGetter(int peer_id);
  void SetPeerListener();
  void UnsetPeerListener();
  void SetReceiveNDEFListener(int peer_id);
  void UnsetReceiveNDEFListener(int peer_id);
  void sendNDEF(int peer_id, const picojson::value& args);
  bool IsNDEFListenerSet();

  // NFCTag related methods
  // attributes
  std::string TagTypeGetter(int tag_id);
  bool TagIsSupportedNDEFGetter(int tag_id);
  unsigned int TagNDEFSizeGetter(int tag_id);
  NFCTagPropertiesT TagPropertiesGetter(int tag_id);
  bool TagIsConnectedGetter(int tag_id);
  // methods
  void TagReadNDEF(int tag_id, const picojson::value& args);
  void TagWriteNDEF(int tag_id, const picojson::value& args);
  void TagTransceive(int tag_id, const picojson::value& args);
  // listeners
  void SetTagListener();
  void UnsetTagListener();
  int GetNextTagId();
  void SetTagHandle(nfc_tag_h tag);

  static NFCAdapter* GetInstance();
 private:
  NFCAdapter();
  virtual ~NFCAdapter();

  nfc_tag_h m_last_tag_handle;
  bool m_is_tag_listener_set;
  int m_latest_tag_id;
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
