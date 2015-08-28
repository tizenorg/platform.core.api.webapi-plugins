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

#include "nfc/nfc_instance.h"

#include <network/nfc.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"
#include "common/task-queue.h"
#include "nfc/defs.h"
#include "nfc/nfc_message_utils.h"
#include "nfc/nfc_util.h"
#include <system_info.h>

namespace extension {
namespace nfc {

using namespace common;
using namespace extension::nfc;

void NFCInstance::RespondAsync(const char* msg) {
  LoggerD("Entered");
  Instance::PostMessage(this, msg);
}

static bool isTagSupported(){
    LoggerD("Entered");
    bool supported = true;
    if (system_info_get_platform_bool(
                    "http://tizen.org/feature/network.nfc.tag", &supported)
                    != SYSTEM_INFO_ERROR_NONE) {
        LoggerD("Can't check Tag is supported or not");
    }
    return supported;
}

static bool isP2PSupported(){
    LoggerD("Entered");
    bool supported = true;
    if (system_info_get_platform_bool(
                    "http://tizen.org/feature/network.nfc.p2p", &supported)
                    != SYSTEM_INFO_ERROR_NONE) {
        LoggerD("Can't check Tag is supported or not");
    }
    return supported;
}

NFCInstance::NFCInstance() {
  LoggerD("Entered");
  using std::placeholders::_1;
  using std::placeholders::_2;
#define REGISTER_ASYNC(c,x) \
  RegisterSyncHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
#define REGISTER_SYNC(c,x) \
  RegisterSyncHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
  REGISTER_SYNC("NFCManager_getDefaultAdapter", GetDefaultAdapter);
  REGISTER_SYNC("NFCManager_setExclusiveMode", SetExclusiveMode);
  REGISTER_ASYNC("NFCAdapter_setPowered", SetPowered);
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

  // HCE related methods
  REGISTER_SYNC("NFCAdapter_addHCEEventListener", AddHCEEventListener);
  REGISTER_SYNC("NFCAdapter_removeHCEEventListener", RemoveHCEEventListener);
  REGISTER_ASYNC("NFCAdapter_sendHostAPDUResponse", SendHostAPDUResponse);
  REGISTER_SYNC("NFCAdapter_isActivatedHandlerForAID",
                IsActivatedHandlerForAID);
  REGISTER_SYNC("NFCAdapter_isActivatedHandlerForCategory",
                IsActivatedHandlerForCategory);
  REGISTER_SYNC("NFCAdapter_registerAID", RegisterAID);
  REGISTER_SYNC("NFCAdapter_unregisterAID", UnregisterAID);
  REGISTER_ASYNC("NFCAdapter_getAIDsForCategory", GetAIDsForCategory);

  REGISTER_SYNC("NFCPeer_setReceiveNDEFListener", SetReceiveNDEFListener);
  REGISTER_SYNC("NFCPeer_unsetReceiveNDEFListener", UnsetReceiveNDEFListener);
  REGISTER_SYNC("NDEFMessage_toByte", ToByte);
  //Message related methods
  REGISTER_SYNC("NDEFMessage_constructor", NDEFMessageContructor);
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

  REGISTER_ASYNC("NFCTag_readNDEF", ReadNDEF);
  REGISTER_ASYNC("NFCTag_writeNDEF", WriteNDEF);
  REGISTER_ASYNC("NFCTag_transceive", Transceive );
  REGISTER_ASYNC("NFCPeer_sendNDEF", SendNDEF);
#undef REGISTER_SYNC
#undef REGISTER_ASYNC

  // Set a PostMessageHandler at NFCAdapter to provide mechanizm of returning
  // asynchronous response
  NFCAdapter::GetInstance()->SetResponder(this);
}

#define CHECK_EXIST(args, name, out)                                           \
  if (!args.contains(name)) {                                                  \
    LoggerE("args doesn't contain attribute '%s'", name);                      \
    ReportError(TypeMismatchException(name" is required argument"), out);      \
    return;                                                                    \
  }

NFCInstance::~NFCInstance() {
  LoggerD("Entered");
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
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                               "NFC manager not supported"), &out);
  }
  else {
    ReportSuccess(out);
  }
}

void NFCInstance::SetExclusiveMode(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "exclusiveMode", out);
  bool exmode = args.get("exclusiveMode").get<bool>();

  int ret = nfc_manager_set_system_handler_enable(!exmode);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("Error: %d", ret);
    PlatformResult result = NFCUtil::CodeToResult(ret,
                                                  "Failed to set exclusive mode.");
    ReportError(result, &out);
  } else {
    ReportSuccess(out);
  }
}

//TODO(g.rynkowski): Rewrite to asynchronous approach
void NFCInstance::SetPowered(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->SetPowered(args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::GetPowered(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  bool ret = NFCAdapter::GetInstance()->GetPowered();
  ReportSuccess(picojson::value(ret), out);
}

void NFCInstance::CardEmulationModeSetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "emulationMode", out);
  std::string mode = args.get("emulationMode").get<std::string>();
  PlatformResult result = NFCAdapter::GetInstance()->SetCardEmulationMode(mode);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::CardEmulationModeGetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  std::string mode = "";
  PlatformResult result = NFCAdapter::GetInstance()->GetCardEmulationMode(&mode);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(mode), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::ActiveSecureElementSetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "secureElement", out);
  std::string ase = args.get("secureElement").get<std::string>();
  PlatformResult result = NFCAdapter::GetInstance()->SetActiveSecureElement(ase);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::ActiveSecureElementGetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  std::string ase = "";
  PlatformResult result = NFCAdapter::GetInstance()->GetActiveSecureElement(&ase);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(ase), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::SetTagListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
    bool supported = isTagSupported();
      if (supported) {
          SLoggerE("Tag is supported");
      } else {
          SLoggerE("Tag is not supported");
          ReportError(
              common::PlatformResult(common::ErrorCode::NOT_SUPPORTED_ERR,
                                     "Tag is not supported on this device."),
              &out);
          return;
      }
  PlatformResult result = NFCAdapter::GetInstance()->SetTagListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::PeerIsConnectedGetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);

  int peer_id = (int)args.get("id").get<double>();
  bool ret = false;
  PlatformResult result = NFCAdapter::GetInstance()->PeerIsConnectedGetter(peer_id, &ret);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(ret), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }

}

void NFCInstance::SetPeerListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  bool supported = isP2PSupported();
  if (supported) {
    SLoggerE("P2P is supported");
  } else {
    SLoggerE("P2P is not supported");
    ReportError(
        common::PlatformResult(common::ErrorCode::NOT_SUPPORTED_ERR,
                               "P2P is not supported on this device."),
                               &out);
    return;
  }
  PlatformResult result = NFCAdapter::GetInstance()->SetPeerListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::UnsetTagListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  bool supported = isTagSupported();
  if (supported) {
    SLoggerE("Tag is supported");
  } else {
    SLoggerE("Tag is not supported");
    ReportError(
        common::PlatformResult(common::ErrorCode::NOT_SUPPORTED_ERR,
                               "Tag is not supported on this device."),
                               &out);
    return;
  }
  NFCAdapter::GetInstance()->UnsetTagListener();
  ReportSuccess(out);
}

void NFCInstance::UnsetPeerListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
    bool supported = isP2PSupported();
      if (supported) {
          SLoggerE("P2P is supported");
      } else {
          SLoggerE("P2P is not supported");
          ReportError(
              common::PlatformResult(common::ErrorCode::NOT_SUPPORTED_ERR,
                                     "P2P is not supported on this device."),
              &out);
          return;
      }
  PlatformResult result = NFCAdapter::GetInstance()->UnsetPeerListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::AddCardEmulationModeChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->AddCardEmulationModeChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveCardEmulationModeChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->RemoveCardEmulationModeChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::AddTransactionEventListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->AddTransactionEventListener(args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveTransactionEventListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->RemoveTransactionEventListener(args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::AddActiveSecureElementChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->AddActiveSecureElementChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveActiveSecureElementChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  PlatformResult result = NFCAdapter::GetInstance()->RemoveActiveSecureElementChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::GetCachedMessage(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  PlatformResult ret = NFCAdapter::GetInstance()->GetCachedMessage(result_obj);
  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

void NFCInstance::SetExclusiveModeForTransaction(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "transactionMode", out);

  bool transaction_mode = args.get("transactionMode").get<bool>();
  PlatformResult result = NFCAdapter::GetInstance()->SetExclusiveModeForTransaction(
        transaction_mode);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

//TODO(g.rynkowski): Rewrite to asynchronous approach
void NFCInstance::ReadNDEF(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);

  int tag_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Tag id: %d", tag_id);

  PlatformResult result = NFCAdapter::GetInstance()->TagReadNDEF(tag_id, args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

//TODO(g.rynkowski): Rewrite to asynchronous approach
void NFCInstance::WriteNDEF(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);

  int tag_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Tag id: %d", tag_id);

  PlatformResult result = NFCAdapter::GetInstance()->TagWriteNDEF(tag_id, args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

//TODO(g.rynkowski): Rewrite to asynchronous approach
void NFCInstance::Transceive(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  CHECK_EXIST(args, "id", out);
  int tag_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Tag id: %d", tag_id);

  PlatformResult result = NFCAdapter::GetInstance()->TagTransceive(tag_id, args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::SetReceiveNDEFListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);

  int peer_id = (int)args.get("id").get<double>();
  PlatformResult result = NFCAdapter::GetInstance()->SetReceiveNDEFListener(peer_id);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::UnsetReceiveNDEFListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);

  int peer_id = (int)args.get("id").get<double>();
  PlatformResult result = NFCAdapter::GetInstance()->UnsetReceiveNDEFListener(peer_id);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

//TODO(g.rynkowski): Rewrite to asynchronous approach
void NFCInstance::SendNDEF(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);

  int peer_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Peer id: %d", peer_id);

  PlatformResult result = NFCAdapter::GetInstance()->sendNDEF(peer_id, args);

  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::ToByte(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::NDEFMessageToByte(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

//Message related methods
void NFCInstance::NDEFMessageContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFMessage(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecord(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordTextContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecordText(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordURIContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecordURI(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordMediaContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecordMedia(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    LoggerE("Error: %s", ret.message().c_str());
    ReportError(ret, &out);
  }
}

// NFCTag attributes getters
void NFCInstance::TagTypeGetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  CHECK_EXIST(args, "id", out);
  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  std::string tag_type = "";
  result = NFCAdapter::GetInstance()->TagTypeGetter(tag_id, &tag_type);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(tag_type), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::TagIsSupportedNDEFGetter(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  bool is_supported = false;
  result = NFCAdapter::GetInstance()->TagIsSupportedNDEFGetter(tag_id, &is_supported);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(is_supported), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::TagNDEFSizeGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  unsigned int ndef_size;
  result = NFCAdapter::GetInstance()->TagNDEFSizeGetter(tag_id, &ndef_size);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value((double)ndef_size), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::TagPropertiesGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  NFCTagPropertiesT prop;

  result = NFCAdapter::GetInstance()->TagPropertiesGetter(tag_id, &prop);
  if (result.IsSuccess()) {
    picojson::value properties = picojson::value(picojson::array());
    picojson::array& properties_array = properties.get<picojson::array>();
    for (auto it = prop.begin() ; it != prop.end(); it++) {
      picojson::value val = picojson::value(picojson::object());
      picojson::object& obj = val.get<picojson::object>();

      picojson::value value_vector = picojson::value(picojson::array());
      picojson::array& value_vector_obj = value_vector.get<picojson::array>();

      for (size_t i = 0 ; i < it->second.size(); i++) {
        value_vector_obj.push_back(picojson::value(
            std::to_string(it->second[i])));
      }

      obj[it->first] = value_vector;
      properties_array.push_back(val);
    }
    ReportSuccess(properties, out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }

}

void NFCInstance::TagIsConnectedGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  CHECK_EXIST(args, "id", out);
  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  bool connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &connected);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(connected), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::AddHCEEventListener(const picojson::value& args,
                                      picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->AddHCEEventListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveHCEEventListener(const picojson::value& args,
                                         picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->RemoveHCEEventListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::SendHostAPDUResponse(const picojson::value& args,
                                       picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, JSON_APDU, out);
  const picojson::array& apdu_array = FromJson<picojson::array>(
      args.get<picojson::object>(), JSON_APDU);
  const UCharVector& apdu = NFCUtil::DoubleArrayToUCharVector(apdu_array);
  const double& callback_id = args.get(JSON_CALLBACK_ID).get<double>();

  auto success_cb = [this, callback_id]() -> void {
    LoggerD("Entered");
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj[JSON_CALLBACK_ID] = picojson::value(callback_id);
    ReportSuccess(response_obj);
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto error_cb = [this, callback_id](const PlatformResult& error) -> void {
    LoggerD("Entered error_cb: %s", error.message().c_str());
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj[JSON_CALLBACK_ID] = picojson::value(callback_id);
    ReportError(error, &response_obj);
    Instance::PostMessage(this, response.serialize().c_str());
  };

  common::TaskQueue::GetInstance().Async(
      std::bind(&NFCAdapter::SendHostAPDUResponse, NFCAdapter::GetInstance(),
          apdu, success_cb, error_cb));
}

void NFCInstance::IsActivatedHandlerForAID(const picojson::value& args,
                                           picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, JSON_TYPE, out);
  CHECK_EXIST(args, JSON_AID, out);

  const std::string& aid = args.get(JSON_AID).get<std::string>();
  bool is_activated_handler = false;

  PlatformResult result = NFCAdapter::GetInstance()->IsActivatedHandlerForAID(
      args.get(JSON_TYPE).get<std::string>(),
      aid,
      &is_activated_handler);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(is_activated_handler), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::IsActivatedHandlerForCategory(const picojson::value& args,
                                                picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, JSON_TYPE, out);
  CHECK_EXIST(args, JSON_CATEGORY, out);

  nfc_card_emulation_category_type_e category =
      NFCUtil::StringToCategory(args.get(JSON_CATEGORY).get<std::string>());
  bool is_activated_handler = false;

  PlatformResult result =
      NFCAdapter::GetInstance()->IsActivatedHandlerForCategory(
          args.get(JSON_TYPE).get<std::string>(),
          category,
          &is_activated_handler);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(is_activated_handler), out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::RegisterAID(const picojson::value& args,
                              picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, JSON_TYPE, out);
  CHECK_EXIST(args, JSON_AID, out);
  CHECK_EXIST(args, JSON_CATEGORY, out);

  nfc_card_emulation_category_type_e category =
      NFCUtil::StringToCategory(args.get(JSON_CATEGORY).get<std::string>());

  PlatformResult result = NFCAdapter::GetInstance()->RegisterAID(
      args.get(JSON_TYPE).get<std::string>(),
      args.get(JSON_AID).get<std::string>(),
      category);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::UnregisterAID(const picojson::value& args,
                                picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, JSON_TYPE, out);
  CHECK_EXIST(args, JSON_AID, out);
  CHECK_EXIST(args, JSON_CATEGORY, out);

  nfc_card_emulation_category_type_e category =
      NFCUtil::StringToCategory(args.get(JSON_CATEGORY).get<std::string>());

  PlatformResult result =
      NFCAdapter::GetInstance()->UnregisterAID(
          args.get(JSON_TYPE).get<std::string>(),
          args.get(JSON_AID).get<std::string>(),
          category);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    LoggerE("Error: %s", result.message().c_str());
    ReportError(result, &out);
  }
}

void NFCInstance::GetAIDsForCategory(const picojson::value& args,
                                     picojson::object& out) {
  LoggerD("Entered");
  CHECK_EXIST(args, JSON_TYPE, out);
  CHECK_EXIST(args, JSON_CATEGORY, out);
  const std::string& type = args.get(JSON_TYPE).get<std::string>();
  nfc_card_emulation_category_type_e required_category =
      NFCUtil::StringToCategory(args.get(JSON_CATEGORY).get<std::string>());
  const double& callback_id = args.get(JSON_CALLBACK_ID).get<double>();

  auto success_cb = [this, callback_id](const AIDDataVector& data) -> void {
    LoggerD("enter");
    picojson::array aids;
    aids.reserve(data.size());
    for (const auto& aid : data) {
      aids.push_back(aid.toJSON());
    }
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj[JSON_CALLBACK_ID] = picojson::value(callback_id);
    ReportSuccess(picojson::value(aids), response_obj);
    Instance::PostMessage(this, response.serialize().c_str());
  };

  auto error_cb = [this, callback_id](const PlatformResult& error) -> void {
    LoggerD("entered error_cb: %s", error.message().c_str());
    picojson::value response = picojson::value(picojson::object());
    picojson::object& response_obj = response.get<picojson::object>();
    response_obj[JSON_CALLBACK_ID] = picojson::value(callback_id);
    ReportError(error, &response_obj);
    Instance::PostMessage(this, response.serialize().c_str());
  };

  common::TaskQueue::GetInstance().Async(
      std::bind(&NFCAdapter::GetAIDsForCategory, NFCAdapter::GetInstance(),
                type, required_category, success_cb, error_cb));
}

} // namespace nfc
} // namespace extension
