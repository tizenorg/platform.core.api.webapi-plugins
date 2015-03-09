// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

#include "nfc/nfc_instance.h"
#include "nfc/nfc_util.h"

#include "common/picojson.h"
#include "common/platform_result.h"
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
  common::PlatformResult SetPowered(const picojson::value& args);

  // cardEmulationMode getter and setter
  common::PlatformResult GetCardEmulationMode(std::string *mode);
  common::PlatformResult SetCardEmulationMode(std::string mode);
  // activeSecureElement getter and setter
  common::PlatformResult GetActiveSecureElement(std::string *type);
  common::PlatformResult SetActiveSecureElement(std::string element);

  // Adapter methods
  common::PlatformResult  SetExclusiveModeForTransaction(bool exmode);

  common::PlatformResult AddCardEmulationModeChangeListener();
  common::PlatformResult RemoveCardEmulationModeChangeListener();
  common::PlatformResult AddTransactionEventListener(const picojson::value& args);
  common::PlatformResult RemoveTransactionEventListener(const picojson::value& args);
  common::PlatformResult AddActiveSecureElementChangeListener();
  common::PlatformResult RemoveActiveSecureElementChangeListener();
  common::PlatformResult GetCachedMessage(picojson::object& out);

  void SetPeerHandle(nfc_p2p_target_h handle);
  nfc_p2p_target_h GetPeerHandle();
  int GetPeerId();
  void IncreasePeerId();
  common::PlatformResult PeerIsConnectedGetter(int peer_id, bool *state);
  common::PlatformResult SetPeerListener();
  common::PlatformResult UnsetPeerListener();
  common::PlatformResult SetReceiveNDEFListener(int peer_id);
  common::PlatformResult UnsetReceiveNDEFListener(int peer_id);
  common::PlatformResult sendNDEF(int peer_id, const picojson::value& args);
  bool IsNDEFListenerSet();

  // NFCTag related methods
  // attributes
  common::PlatformResult TagTypeGetter(int tag_id, std::string *type);
  common::PlatformResult TagIsSupportedNDEFGetter(int tag_id, bool *is_supported);
  common::PlatformResult TagNDEFSizeGetter(int tag_id, unsigned int *size);
  common::PlatformResult TagPropertiesGetter(int tag_id, NFCTagPropertiesT *properties);
  common::PlatformResult TagIsConnectedGetter(int tag_id, bool *state);
  // TODO remove after clean code from try/catch
  bool TagIsConnectedGetter(int tag_id);
  // methods
  common::PlatformResult TagReadNDEF(int tag_id, const picojson::value& args);
  void TagWriteNDEF(int tag_id, const picojson::value& args);
  void TagTransceive(int tag_id, const picojson::value& args);
  // listeners
  common::PlatformResult SetTagListener();
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
