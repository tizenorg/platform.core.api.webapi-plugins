/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef NFC_NFC_ADAPTER_H_
#define NFC_NFC_ADAPTER_H_

#ifdef APP_CONTROL_SETTINGS_SUPPORT
#include <app_control.h>
#endif
#include <network/nfc.h>

#include <list>
#include <memory>

#include "common/picojson.h"
#include "common/platform_result.h"
#include "nfc/aid_data.h"
#include "nfc/nfc_util.h"

namespace extension {
namespace nfc {

typedef std::list<std::pair<std::string, UCharVector>> NFCTagPropertiesT;

class NFCAdapter {
 public:
  static NFCAdapter* GetInstance();

  // Interface provides method PostMessage() that returns asynchronous response.
  class IResponder {
  public:
    virtual void RespondAsync(const char* msg) = 0;
  };

  void SetResponder(IResponder* responder);
  void RespondAsync(const char* msg);

  bool GetPowered();
  common::PlatformResult SetPowered(const picojson::value& args);

  // cardEmulationMode getter and setter
  common::PlatformResult GetCardEmulationMode(std::string* mode);
  common::PlatformResult SetCardEmulationMode(const std::string& mode);
  // activeSecureElement getter and setter
  common::PlatformResult GetActiveSecureElement(std::string* type);
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
  common::PlatformResult PeerIsConnectedGetter(int peer_id, bool* state);
  common::PlatformResult SetPeerListener();
  common::PlatformResult UnsetPeerListener();
  common::PlatformResult SetReceiveNDEFListener(int peer_id);
  common::PlatformResult UnsetReceiveNDEFListener(int peer_id);
  common::PlatformResult sendNDEF(int peer_id, const picojson::value& args);
  bool IsNDEFListenerSet();

  // NFCTag related methods
  // attributes
  common::PlatformResult TagTypeGetter(int tag_id, std::string* type);
  common::PlatformResult TagIsSupportedNDEFGetter(int tag_id, bool* is_supported);
  common::PlatformResult TagNDEFSizeGetter(int tag_id, unsigned int* size);
  common::PlatformResult TagPropertiesGetter(int tag_id, NFCTagPropertiesT* properties);
  common::PlatformResult TagIsConnectedGetter(int tag_id, bool* state);
  // methods
  common::PlatformResult TagReadNDEF(int tag_id, const picojson::value& args);
  common::PlatformResult TagWriteNDEF(int tag_id, const picojson::value& args);
  common::PlatformResult TagTransceive(int tag_id, const picojson::value& args);
  // listeners
  common::PlatformResult SetTagListener();
  void UnsetTagListener();
  int GetNextTagId();
  void SetTagHandle(nfc_tag_h tag);

  // HCE
  void SetSEHandle(nfc_se_h handle);
  nfc_se_h GetSEHandle();
  common::PlatformResult AddHCEEventListener();
  common::PlatformResult RemoveHCEEventListener();
  void SendHostAPDUResponse(
      const UCharVector& apdu,
      const std::function<void()>& success_cb,
      const std::function<void(const common::PlatformResult&)>& error_cb);
  common::PlatformResult IsActivatedHandlerForAID(const std::string& type,
                                                  const std::string& aid,
                                                  bool* is_activated_handler);
  common::PlatformResult IsActivatedHandlerForCategory(
      const std::string& type,
      nfc_card_emulation_category_type_e category,
      bool* is_activated_handler);
  common::PlatformResult RegisterAID(
      const std::string& type,
      const std::string& aid,
      nfc_card_emulation_category_type_e category);
  common::PlatformResult UnregisterAID(
      const std::string& type,
      const std::string& aid,
      nfc_card_emulation_category_type_e category);
  void GetAIDsForCategory(
      const std::string& type,
      nfc_card_emulation_category_type_e category,
      const std::function<void(const AIDDataVector&)>& success_cb,
      const std::function<void(const common::PlatformResult&)>& error_cb);


 private:
  NFCAdapter();
  virtual ~NFCAdapter();

  nfc_tag_h m_last_tag_handle;
  bool m_is_tag_listener_set;
  int m_latest_tag_id;
  bool m_is_listener_set;
  bool m_is_transaction_ese_listener_set;
  bool m_is_transaction_uicc_listener_set;
  bool m_is_transaction_hce_listener_set;
  bool m_is_peer_listener_set;
  int m_latest_peer_id;
  nfc_p2p_target_h m_peer_handle;
  bool m_is_ndef_listener_set;
  nfc_se_h m_se_handle;
  bool m_is_hce_listener_set;

  IResponder* responder_;
};

} // nfc
} // extension

#endif // NFC_NFC_ADAPTER_H_
