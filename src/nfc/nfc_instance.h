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

#ifndef NFC_NFC_INSTANCE_H_
#define NFC_NFC_INSTANCE_H_

#include "common/extension.h"
#include "nfc/nfc_adapter.h"

namespace extension {
namespace nfc {

class NFCInstance: public common::ParsedInstance, NFCAdapter::IResponder
{
 public:
  NFCInstance();
  virtual ~NFCInstance();

  void RespondAsync(const char* msg);

 private:
  void GetDefaultAdapter(const picojson::value& args, picojson::object& out);
  void SetExclusiveMode(const picojson::value& args, picojson::object& out);
  void SetPowered(const picojson::value& args, picojson::object& out);
  void GetPowered(const picojson::value& args, picojson::object& out);
  void CardEmulationModeSetter(const picojson::value& args, picojson::object& out);
  void CardEmulationModeGetter(const picojson::value& args, picojson::object& out);
  void ActiveSecureElementSetter(const picojson::value& args, picojson::object& out);
  void ActiveSecureElementGetter(const picojson::value& args, picojson::object& out);
  void SetTagListener(const picojson::value& args, picojson::object& out);
  void PeerIsConnectedGetter(const picojson::value& args, picojson::object& out);
  void SetPeerListener(const picojson::value& args, picojson::object& out);
  void UnsetTagListener(const picojson::value& args, picojson::object& out);
  void UnsetPeerListener(const picojson::value& args, picojson::object& out);
  void AddCardEmulationModeChangeListener(const picojson::value& args, picojson::object& out);
  void RemoveCardEmulationModeChangeListener(const picojson::value& args, picojson::object& out);
  void AddTransactionEventListener(const picojson::value& args, picojson::object& out);
  void RemoveTransactionEventListener(const picojson::value& args, picojson::object& out);
  void AddActiveSecureElementChangeListener(const picojson::value& args, picojson::object& out);
  void RemoveActiveSecureElementChangeListener(const picojson::value& args, picojson::object& out);
  void GetCachedMessage(const picojson::value& args, picojson::object& out);
  void SetExclusiveModeForTransaction(const picojson::value& args, picojson::object& out);
  void ReadNDEF(const picojson::value& args, picojson::object& out);
  void WriteNDEF(const picojson::value& args, picojson::object& out);
  void Transceive(const picojson::value& args, picojson::object& out);
  void SetReceiveNDEFListener(const picojson::value& args, picojson::object& out);
  void UnsetReceiveNDEFListener(const picojson::value& args, picojson::object& out);
  void SendNDEF(const picojson::value& args, picojson::object& out);
  void ToByte(const picojson::value& args, picojson::object& out);

  // Message related methods
  void NDEFMessageContructor(const picojson::value& args, picojson::object& out);
  void NDEFMessageToByte(const picojson::value& args, picojson::object& out);
  void NDEFRecordContructor(const picojson::value& args, picojson::object& out);
  void NDEFRecordTextContructor(const picojson::value& args, picojson::object& out);
  void NDEFRecordURIContructor(const picojson::value& args, picojson::object& out);
  void NDEFRecordMediaContructor(const picojson::value& args, picojson::object& out);

  // NFCTag attributes getters
  void TagTypeGetter(const picojson::value& args, picojson::object& out);
  void TagIsSupportedNDEFGetter(const picojson::value& args, picojson::object& out);
  void TagNDEFSizeGetter(const picojson::value& args, picojson::object& out);
  void TagPropertiesGetter(const picojson::value& args, picojson::object& out);
  void TagIsConnectedGetter(const picojson::value& args, picojson::object& out);

  // HCE related methods
  void AddHCEEventListener(const picojson::value& args, picojson::object& out);
  void RemoveHCEEventListener(const picojson::value& args, picojson::object& out);
  void SendHostAPDUResponse(const picojson::value& args, picojson::object& out);
  void IsActivatedHandlerForAID(const picojson::value& args, picojson::object& out);
  void IsActivatedHandlerForCategory(const picojson::value& args, picojson::object& out);
  void RegisterAID(const picojson::value& args, picojson::object& out);
  void UnregisterAID(const picojson::value& args, picojson::object& out);
  void GetAIDsForCategory(const picojson::value& args, picojson::object& out);
};

} // namespace nfc
} // namespace extension

#endif // NFC_NFC_INSTANCE_H_
